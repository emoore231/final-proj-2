#include "DatabaseBeginTransactionMenu.h"
#include <thread>
#include <string>
#include <iostream>
#include "SQLiteDB.h"
#include "global.h"

bool DatabaseBeginTransactionMenu ()
{
	//check if in transaction
	if (inTransaction)
	{
		std::cout << "Database already in transaction.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}

	std::cout << "Do you want to enter into a transaction? (Y/N)" << lf;
	
	std::string opt;
	for (size_t i = 0; i < opt.size (); i++) if (opt[i] >= 'A' && opt[i] <= 'Z') opt[i] += 'a' - 'A';

	std::cin >> std::ws >> opt;

	if (opt == "y" || opt == "yes")	try
	{
		//start new transaction
		currentTransactionDatabase = new SQL::Transaction (SQL::Database ("master.db").BeginTransaction ());
		currentTransactionOperationCount = 0;
		inTransaction = true;
		std::cout << "Transaction started successfully" << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
	catch (...)
	{
		delete currentTransactionDatabase;
		throw;
	}
	else
	{
		std::cout << "Transaction canceled.\nNo operation was performed." << lf;
		std::this_thread::sleep_for (std::chrono::seconds (2));
		return true;
	}
}