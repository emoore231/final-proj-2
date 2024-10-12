#include "DatabaseAdvancedManipulationMenu.h"
#include "DatabaseBeginTransactionMenu.h"
#include "DatabaseCommitTransactionMenu.h"
#include "DatabaseRollbackTransactionMenu.h"
#include "DatabaseInsertRecordIntoTableMenu.h"
#include "DatabaseUpdateRecordMenu.h"
#include "DatabaseDeleteRecordMenu.h"
#include <thread>
#include <string>
#include <iostream>
#include "SQLiteDB.h"
#include "global.h"

bool DatabaseAdvancedManipulationMenu ()
{
DISPLAY:

	const static std::vector<std::pair<int, std::pair<std::string, fptr_t>>> menuOptions = {
		//return to main menu
		{1, {"Return to previous menu", [] () -> bool { return false; }}},
		//starts a transaction, this is not the transaction inserts, just an option to use a transaction
		{3, {"Begin database transaction", DatabaseBeginTransactionMenu }},
		//commits and rolls back a transaction
		{3, {"Commit database transaction", DatabaseCommitTransactionMenu }},
		{3, {"Rollback database transaction", DatabaseRollbackTransactionMenu }},
		//inserts one record into db, provides LOOKUP functionality for the user to lookup fks
		{3, {"Insert record into database", DatabaseInsertRecordIntoTableMenu }},
		//updates a single record in the database
		{3, {"Update record in database", DatabaseUpdateRecordMenu }},
		//deleted a record from db
		{3, {"Delete record from database", DatabaseDeleteRecordMenu }},
		
		//{3, {"Search records in database", nullptr }},
	};

	std::cout << "Moore's C&R Central database" << lf
		<< "ACCESS LEVEL " << accessLevel << "." << lf << lf;
	std::cout << "Moore's C&R Central database" << lf
		<< "ACCESS LEVEL " << accessLevel << "." << lf;
	if (inTransaction)
		std::cout << "TRANSACTION IN PROGRESS" << lf << lf;
	std::cout << lf;

	std::cout << "Please select a menu option:" << lf;

	int i = 1;
	for (const auto menuOption : menuOptions)
		if (accessLevel >= menuOption.first)
			std::cout << '\t' << i++ << ": " << menuOption.second.first << std::endl;

	try
	{
		std::cin >> std::ws >> i;
	}
	catch (...)
	{
		HandleIstreamFailure ();
		goto DISPLAY;
	}

	if (i > menuOptions.size () || i < 1)
		goto DISPLAY;
	else
		if (menuOptions[i - 1].second.second ())
			goto DISPLAY;
		else
			return true;
}