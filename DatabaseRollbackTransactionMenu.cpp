#include "DatabaseRollbackTransactionMenu.h"
#include <thread>
#include <string>
#include <iostream>
#include "SQLiteDB.h"
#include "global.h"

bool DatabaseRollbackTransactionMenu ()
{
	//check transaction
	if (!inTransaction)
	{
		std::cout << "No transaction is in progress.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}

	std::cout << "Do you want to rollback this transaction? (Y/N)" << lf
		<< "There are " << currentTransactionOperationCount << " operations pending." << lf
		<< "ALL CHANGES WILL BE LOST" << lf;

	std::string opt;
	std::cin >> std::ws >> opt;
	for (auto& i : opt)	if (i >= 'A' && i <= 'Z') i += 'a' - 'A';
	//get confirm
	if (opt == "y" || opt == "yes")	try
	{//rollback
		currentTransactionDatabase->Rollback ();
		currentTransactionOperationCount = 0;
		inTransaction = false;
		delete currentTransactionDatabase;
		std::cout << "Transaction rolled back successfully." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
	catch (...)
	{
		std::cout << "Transaction failed to roll back, the database is in an inconsistent and/or inoperable state. The program will now terminate.";
		std::this_thread::sleep_for (std::chrono::seconds (2));
		throw "FAIL_FAST";//bypasses all exception handlers, just like exit() but I can break
	}
	else
	{
		std::cout << "Transaction canceled.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
}