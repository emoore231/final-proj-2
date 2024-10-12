#include "DatabaseInsertRecordIntoTableMenu.h"
#include "SQLiteDB.h"
#include "global.h"
#include <map>
#include <string>
#include <vector>
#include <iostream>




std::string LookupForeignKey (Table table);

bool DatabaseInsertRecordIntoTableMenu ()
{
	Table table;
	std::string opt;
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

	std::vector<std::string> params;

	std::cout << "Enter the fields for the insertion or \'NULL\' for null values, or type \'LOOKUP\' to lookup a value for a foreign key." << lf;

	if (inTransaction)
		std::cout << "TRANSACT MODE" << lf;
	else
		std::cout << "NON-TRANSACT MODE, BE CAREFUL!" << lf;

	//enter fields
	for (auto& i : TableToInsertionFields.at (table))
	{
		try
		{
			std::cout << "Enter the value for field " << i.first << " with datatype " << TypeToString.at (i.second) << " : ";
			std::cin >> std::ws;
			std::getline (std::cin, opt);
			if (opt == "NULL" || opt == "null")
				opt.clear ();
			else if (opt == "LOOKUP" || opt == "lookup")
				opt = LookupForeignKey ((Table)i.second);
			params.push_back (opt);
		}
		catch (...)
		{
			std::cout << "An error occurred while retrieving parameters for the insertion. \nNo operation was performed." << lf;
			return true;
		}
	}

	std::cout << lf << "Confirm these options" << lf;
	if (inTransaction)
		std::cout << "TRANSACTION MODE" << lf;
	else
		std::cout << "NON-TRANSACTION MODE BE CAREFUL!" << lf;

	for (int i = 0; i < params.size (); i++)
	{
		std::cout << "FIELD " << TypeToString.at (TableToInsertionFields.at (table).at (i).second) << " " << TableToInsertionFields.at (table).at (i).first << " " << params[i] << lf;
	}

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


	if (inTransaction)
	{
		try
		{//add to transaction connection
			currentTransactionDatabase->ExecutePrepared (SQL::ParameterizedQuery (TableToInsertionQuery.at (table), params));
			currentTransactionOperationCount++;
			std::cout << "OPERATION EXECUTED SUCCESSFULLY" << lf << "PENDING TRANSACT OPERATIONS: " << currentTransactionOperationCount << lf;
			return true;
		}
		catch (const SQL::Exception& ex)
		{//fail and rollback
			currentTransactionDatabase->Rollback ();
			delete currentTransactionDatabase;
			std::cout << "Insertion failed, code: " << ex.Code << "; message: " << ex.what () << lf
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
			database.ExecutePrepared (SQL::ParameterizedQuery (TableToInsertionQuery.at (table), params));
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
			std::cout << "Insertion failed, code: " << ex.Code << "; message: " << ex.what () << lf
				<< "OPERATION CANCELED" << lf;
			return true;
		}
	}

	return true;
}

std::string LookupForeignKey (Table table)
{
	for (;;) try
	{
		std::string query = TableToSelectionQuery.at (table);

		SQL::Database database ("master.db");

		SQL::Result res = database.ExecutePrepared (SQL::Query (query));

		int k = 0;
		for (auto& i : res)
		{
			std::cout << i.at ("ID") << ": " << i << lf;
			k++;
		}

		database.Close ();

		std::cout << "SELECT RECORD ID: ";
		size_t i;
		std::cin >> std::ws >> i;
		if (i < 1 || i > k)
		{
			std::cout << "OPTION NOT RECOGNIZED" << std::endl;
			continue;
		}
		return std::to_string (i);
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
}
