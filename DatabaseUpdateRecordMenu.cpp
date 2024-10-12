#include "DatabaseUpdateRecordMenu.h"
#include "global.h"
#include <iostream>
#include <string>
#include "SQLiteDB.h"

bool DatabaseUpdateRecordMenu ()
{
	Table table;
	std::string opt;

	//get table
	do try
	{
		std::cout << "Please select the table to insert data to." << lf;

		for (auto& i : TableToString)
		{
			std::cout << '\t' << i.second << lf;
		}

		std::cout << "Enter table name: ";

		std::cin >> std::ws;
		std::getline (std::cin, opt);
		for (size_t i = 0; i < opt.size (); i++) if (opt[i] >= 'a' && opt[i] <= 'z') opt[i] += 'A' - 'a'; else if (opt[i] == ' ') opt[i] = '_';//convert to uppercase and ' ' to _


		table = StringToTable.at (opt);
		break;
	}
	catch (const std::ios::failure&)
	{
		HandleIstreamFailure ();
		continue;
	}
	catch (const std::out_of_range&)
	{
		std::cout << "OPTION NOT RECOGNIZED" << lf;
		continue;
	}
	while (true);

	std::string rowIdx;

	std::map<std::string, std::string> row;

	//select row to alter
	do try
	{
		std::cout << "SELECT DATABASE RECORD" << lf;
		std::string query = TableToSelectionQuery.at (table);

		std::map<size_t, std::map<std::string, std::string>> rows;

		int k = 0;
		{
			SQL::Database database ("master.db");

			SQL::Result res = database.ExecutePrepared (SQL::Query (query));

			for (const auto& i : res)
			{
				std::cout << i.at ("ID") << ": " << i << lf;
				k++;
				rows[(size_t)atoll (i.at ("ID").c_str())] = i;
			}
		}

		std::cout << "SELECT RECORD ID: ";
		size_t i;
		std::cin >> std::ws >> i;
		if (i < 1 || i > k)
		{
			std::cout << "OPTION NOT RECOGNIZED" << std::endl;
			continue;
		}
		rowIdx = std::to_string (i);
		row = rows.at (i);
		break;
	}
	catch (const std::ios_base::failure&)
	{
		HandleIstreamFailure ();
		continue;
	}
	catch (const std::out_of_range&)
	{
		std::cout << "TABLE NOT FOUND";
		throw;
	}
	catch (const SQL::Exception& ex)
	{
		std::cout << "TABLE NOT FOUND OR RECORDS INACCESSIBLE" << std::endl;
		throw;
	}
	while (true);

	auto tableFields = TableToInsertionFields.at (table);

	std::vector<std::string> newInsertionFields;

	//get new field values
	for (auto& i : tableFields)
	{
		std::cout << "ENTER " << TypeToString.at (i.second) << " " << i.first << lf
			<< "ORIGNAL VALUE: " << row.at (i.first) << lf
			<< "ENTER NEW VALUE OR 'NULL' FOR NULL VALUE OR 'LOOKUP' TO LOOKUP KEY: ";
		std::string newValue;
		std::cin >> std::ws;
		std::getline (std::cin, newValue);
		if (newValue == "NULL" || newValue == "null")
			newValue = "";
		newInsertionFields.push_back (newValue);
	}

	newInsertionFields.push_back (rowIdx);//last element is id for the WHERE clause

	//get confirmation
	do try
	{
		std::cout << lf << "CONFIRM (Y/N)" << lf;
		char ch;
		std::cin >> std::ws >> ch;
		if (ch == 'y' || ch == 'Y')
			break;
		else if (ch == 'n' || ch == 'N')
		{
			std::cout << "OPERATION CANCELED." << lf;
			return true;
		}
		else
			continue;
	}
	catch (const std::ios_base::failure&)
	{
		std::cout << "OPTION NOT RECOGNIZED" << lf;
	}
	while (true);

	//commit to transaction / add to db
	if (inTransaction)
	{
		try
		{//add to transaction connection
			currentTransactionDatabase->ExecutePrepared (SQL::ParameterizedQuery (TableToUpdateQuery.at (table), newInsertionFields));
			currentTransactionOperationCount++;
			std::cout << "OPERATION EXECUTED SUCCESSFULLY" << lf << "PENDING TRANSACT OPERATIONS: " << currentTransactionOperationCount << lf;
			return true;
		}
		catch (const SQL::Exception& ex)
		{//fail and rollback
			currentTransactionDatabase->Rollback ();
			delete currentTransactionDatabase;
			std::cout << "Update failed, code: " << ex.Code << "; message: " << ex.what () << lf
				<< "Rolling back database, " << currentTransactionOperationCount << " changes reverted." << lf;
			currentTransactionOperationCount = 0;
			return true;
		}
	}
	else
	{
		SQL::Database database {};
		try
		{//open and add
			database.Open ("master.db");
			database.Begin ();
			database.ExecutePrepared (SQL::ParameterizedQuery (TableToUpdateQuery.at (table), newInsertionFields));
			database.Commit ();
			database.Close ();
			std::cout << "Statement executed successfully." << lf;
			return true;
		}
		catch (const SQL::Exception& ex)
		{
			//rollback if failure
			database.Rollback ();
			database.Close ();
			std::cout << "Update failed, code: " << ex.Code << "; message: " << ex.what () << lf
				<< "OPERATION CANCELED" << lf;
			return true;
		}
	}

}
