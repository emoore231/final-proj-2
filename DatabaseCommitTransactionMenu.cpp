#include "DatabaseCommitTransactionMenu.h"
#include <thread>
#include <string>
#include <iostream>
#include "SQLiteDB.h"
#include "global.h"


bool DatabaseCommitTransactionMenu ()
{
	//check if no transaction
	if (!inTransaction)
	{
		std::cout << "No transaction is in progress.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
	else if (currentTransactionOperationCount == 0)
	{
		std::cout << "There are no operations pending.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}

	std::cout << "Do you want to commit this transaction? (Y/N)" << lf
		<< "There are " << currentTransactionOperationCount << " operations pending." << lf;
	//get opt
	std::string opt;
	std::cin >> std::ws >> opt;
	for (auto& i : opt)	if (i >= 'A' && i <= 'Z') i += 'a' - 'A';

	if (opt == "y" || opt == "yes")	try
	{//commit
		currentTransactionDatabase->Commit ();
		currentTransactionOperationCount = 0;
		inTransaction = false;
		delete currentTransactionDatabase;
		std::cout << "Transaction committed successfully." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
	catch (...)
	{//handle
		try
		{
			currentTransactionDatabase->Rollback ();
		}
		catch (const std::exception&)
		{
			std::cout << "Transaction failed to commit. Changes have failed to roll back, the database is in an inconsistent  and/or inoperable state. The program will now terminate.";
			std::this_thread::sleep_for (std::chrono::seconds (2));
			throw "FAIL_FAST";
		}
		std::cout << "Transaction failed to commit, all changes have been rolled back prior to the beginning of the transaction." << lf;
		delete currentTransactionDatabase;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		throw;
	}
	else
	{
		std::cout << "Transaction canceled.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
}