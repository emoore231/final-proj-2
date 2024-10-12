#include "SQLiteDB.h"

//use latest v 3.46.1
#include "sqlite3.h"

#include <stdexcept>


SQL::Database::Database (Database&& rhs)
{
	//moves the database ptr
	database = rhs.database;
	rhs.database = 0;
	connectionString = rhs.connectionString;
	rhs.connectionString.clear ();
}

SQL::Database::Database (const Database& rhs)
{
	connectionString = rhs.connectionString;
	if (!connectionString.empty ())
		Open (connectionString);
}

SQL::Database::Database ()
	: database (nullptr)
{}

SQL::Database::Database (const std::string& Path)
	: Database ()
{
	Database::Open (Path);
}

SQL::Database& SQL::Database::Open (const std::string& Path)
{
	connectionString = Path;

	int ExitCode;

	//will fail if file not exist
	ExitCode = sqlite3_open_v2 (connectionString.c_str(), (sqlite3**)&database, SQLITE_OPEN_READWRITE, NULL);

	if (ExitCode)
	{
		throw SQL::Exception (ExitCode, "Error opening database");
	}
	else
	{
		return *this;
	}
}

SQL::Database& SQL::Database::Close ()
{
	sqlite3_close ((sqlite3*)database);
	database = nullptr;

	return *this;
}

void SQL::Database::Execute (const SQL::Query q)
{
	//execute query
	char* ErrMsg;
	int Code = sqlite3_exec ((sqlite3*)database, q.RawQuery ().c_str (), nullptr, nullptr, &ErrMsg);
	if (Code == SQLITE_OK)
		return;
	else
	{//handle 'n throw
		std::string Err (ErrMsg);
		sqlite3_free (ErrMsg);
		throw Exception (Code, Err);
	}
}

SQL::Result SQL::Database::ExecutePrepared (const ParameterizedQuery& q)
{
	SQL::Result res {};

	sqlite3_stmt* statement;

	//prepare the statement
	int Code = sqlite3_prepare_v2 ((sqlite3*)(this->database), q.RawQuery ().c_str (), (int)q.RawQuery ().size (), &statement, nullptr);
	if (Code)
	{
		const char* errMsg = sqlite3_errmsg ((sqlite3*)database);
		std::string errMsgStr (errMsg);

		throw SQL::Exception (Code, "Error preparing statement, MSG: " + errMsgStr);
	}

	//bind params
	for (size_t i = 0; i < q.Parameters.size (); i++)
	{
		sqlite3_bind_text (statement, (int)i + 1, q.Parameters[i].c_str (), (int)q.Parameters[i].size (), SQLITE_STATIC);
	}

	//init the result set if has values
	do 
	switch (int Code = sqlite3_step (statement))
	{
		case SQLITE_ROW:
			{//get cols
				std::map<std::string, std::string> row;
				int cols = sqlite3_column_count (statement);
				//add cols to result
				for (int i = 0; i < cols; ++i)
				{
					//sdd elements
					std::string column_name = sqlite3_column_name (statement, i);
					const unsigned char* column_value = sqlite3_column_text (statement, i);
					row[column_name] = column_value ? rc<const char*>(column_value) : "";
				}
				res.Data.push_back (row);
			}
			res.HasData = true;
			break;
		case SQLITE_DONE://no data, return control
			res.HasData = true;
			return res;
		default:
			sqlite3_finalize (statement);
			throw Exception (SQLITE_ERROR, "An error occured");
	}
	while (true);
}

SQL::Result SQL::Database::ExecutePrepared (const Query& q)
{
	return ExecutePrepared (ParameterizedQuery (q));
}

SQL::Transaction SQL::Database::BeginTransaction ()
{
	Transaction transaction;
	//assign database
	transaction.database = new SQL::Database (connectionString);
	//begin transaction
	transaction.database->Execute (Query ("BEGIN;"));
	return transaction;
}

SQL::Database::~Database ()
{
	if (database)
		Database::Close ();
	return;
}

SQL::Result SQL::Database::ExecutePreparedInPlace (const ParameterizedQuery& q)
{
	SQL::Result res {};

	sqlite3_stmt* statement;

	//prepare the statement
	int Code = sqlite3_prepare_v2 ((sqlite3*)(this->database), q.RawQuery ().c_str (), (int)q.RawQuery ().size (), &statement, nullptr);
	if (Code)
	{
		const char* errMsg = sqlite3_errmsg ((sqlite3*)database);
		std::string errMsgStr (errMsg);

		throw SQL::Exception (Code, "Error preparing statement, MSG: " + errMsgStr);
	}

	//bind params
	for (size_t i = 0; i < q.Parameters.size (); i++)
	{
		sqlite3_bind_text (statement, (int)i + 1, q.Parameters[i].c_str (), (int)q.Parameters[i].size (), SQLITE_STATIC);
	}

	//init the result set if has values
	switch (int Code = sqlite3_step (statement))
	{
		case SQLITE_ROW:
			{//get cols
				std::map<std::string, std::string> row;
				int cols = sqlite3_column_count (statement);
				//add cols to result
				for (int i = 0; i < cols; ++i)
				{
					//sdd elements
					std::string column_name = sqlite3_column_name (statement, i);
					const unsigned char* column_value = sqlite3_column_text (statement, i);
					row[column_name] = column_value ? rc<const char*> (column_value) : "";
				}
				res.Data.push_back (row);
			}
			res.HasData = true;

			return res;

		case SQLITE_DONE://no data, return control
			res.HasData = false;
			return res;
		case SQLITE_INTERRUPT:
			res.HasData = false;
			return res;
		default:
			sqlite3_finalize (statement);
			throw Exception (SQLITE_ERROR, "An error occured");
	}
}

SQL::Result SQL::Database::ExecutePreparedInPlace (const Query& q)
{
	return ExecutePreparedInPlace (SQL::ParameterizedQuery (q));
}

SQL::Query::Query (std::string q)
	: _Query (q)
{}

SQL::Query::Query (const char* q)
	: _Query (q)
{}

const std::string& SQL::Query::RawQuery () const
{
	return _Query;
}

SQL::ParameterizedQuery::ParameterizedQuery (std::string q, std::vector<std::string> params) :
	Query (q), Parameters (params)
{}

SQL::ParameterizedQuery::ParameterizedQuery (const char* q, std::vector<std::string> params) :
	Query (q), Parameters (params)
{}

//SQL::Iterator SQL::Result::begin ()
//{
//	return Iterator (this);
//}

//SQL::Iterator SQL::Result::end ()
//{
//	return Iterator (nullptr);
//}

//clears the iterator if it is not empty
//this is needed because only one query can be open per connection

//void SQL::Result::clear ()
//{
//	//clears all rows
//	for (auto& i : *this)
//	{
//		NULL;
//	}
//}

SQL::Result::~Result ()
{
//	if (statement)
//		sqlite3_finalize ((sqlite3_stmt*)statement);
}

//std::map<std::string, std::string>& SQL::Iterator::operator*() const
//{
//	return *res->CurrentVal;
//}
//
//std::map<std::string, std::string>* SQL::Iterator::operator->()
//{
//	return (res->CurrentVal);
//}

//SQL::Iterator& SQL::Iterator::operator++()
//{
//	//step a row, copy results or set null if done / error
//	switch (int Code = sqlite3_step ((sqlite3_stmt*)res->statement))
//	{
//	case SQLITE_ROW:
//		//copy vals
//		for (int i = 0; i < res->CurrentVal->size(); ++i)
//		{
//			const char* Field = (const char*)sqlite3_column_text ((sqlite3_stmt*)res->statement, i);
//			if (Field)
//				*res->CurrentValVector[i] = std::string (Field);
//		}
//		return *this;
//	case SQLITE_DONE:
//		sqlite3_finalize ((sqlite3_stmt*)res->statement);
//		res->statement = 0;
//		if (res->inExecution)
//			*res->inExecution = false;
//		res = nullptr;
//		return *this;
//	case SQLITE_INTERRUPT:
//		sqlite3_finalize ((sqlite3_stmt*)res->statement);
//		res->statement = 0;
//		if (res->inExecution)
//			*res->inExecution = false;
//		res = nullptr;
//		return *this;
//	default:
//		sqlite3_finalize ((sqlite3_stmt*)res->statement);
//		res->statement = 0;
//		res = nullptr;
//		throw Exception (SQLITE_ERROR, "An error occured");
//	}
//}


#ifdef NO_DEF

sqlite3_stmt* statement;

int Code = sqlite3_prepare_v2 ((sqlite3*)database, q.RawQuery ().c_str (), (int)q.RawQuery ().size (), &statement, nullptr);
if (Code)
{
	throw SQL::Exception (Code, "Error preparing statement");
}

for (size_t i = 0; i < q.Parameters.size (); i++)
{
	sqlite3_bind_text (statement, (int)i + 1, q.Parameters[i].c_str (), (int)q.Parameters[i].size (), SQLITE_STATIC);
}

Result r;
while (Code = sqlite3_step (statement), true) switch (Code)
{
	case SQLITE_DONE:
		sqlite3_finalize (statement);
		return r;

	case SQLITE_ROW:
		{
			Result r;

			//get cols
			int ColCnt = sqlite3_column_count (statement);
			std::vector<std::string> ColNames;
			for (int i = 0; i < ColCnt; ++i)
			{
				ColNames.push_back (sqlite3_column_name (statement, i));
			}

			do
			{
				for (int i = 0; i < ColCnt; ++i)
				{
					const char* Field = (const char*)sqlite3_column_text (statement, i);
					r.Data[ColNames[i]].emplace_back (Field ? Field : "");
				}

				int Code = sqlite3_step (statement);
				if (Code == SQLITE_ROW)
					continue;
				else if (Code == SQLITE_DONE)
					return r;
				else if (Code == SQLITE_BUSY)
					throw Exception (SQLITE_BUSY, "The database is busy");
				else
					throw Exception (SQLITE_ERROR, "An error occured");
			}
			while (true);
		}
		break;

	case SQLITE_BUSY:
		sqlite3_finalize (statement);
		throw Exception (SQLITE_BUSY, "Database busy");

	case SQLITE_ERROR:
	default:
		sqlite3_finalize (statement);
		throw Exception (SQLITE_ERROR, "An error occured");
}


#endif // !NO_DEF

SQL::Transaction::Transaction (Transaction&& rhs)
{
	database = rhs.database;
	rhs.database = nullptr;
}

SQL::Transaction::Transaction ()
{
}

void SQL::Transaction::Execute (const Query q)
{
	if (inExecution)
		sqlite3_interrupt (rc<sqlite3*> (database));
	database->Execute (q);
	inExecution = false;
}

SQL::Result SQL::Transaction::ExecutePrepared (const ParameterizedQuery& q)
{
	Result res = database->ExecutePrepared (q);
	return res;
}

SQL::Result SQL::Transaction::ExecutePrepared (const Query& q)
{
	Result res = database->ExecutePrepared (q);
	return res;
}

void SQL::Transaction::Commit ()
{
	database->Execute ("COMMIT;");
}

void SQL::Transaction::Rollback ()
{
	try
	{
		database->Execute ("ROLLBACK;");
	}
	catch (const std::exception& ex)
	{
		std::cerr << "A critical error occured while attempting to roll back this transaction." << std::endl << "The exception information is " << ex.what () << std::endl << "The program will now terminate.";
		throw "FAIL_FAST";
	}
}

SQL::Transaction::~Transaction ()
{
	delete rc<Database*> (database);
}
