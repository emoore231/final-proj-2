#include "DatabaseMainMenu.h"

#include "SQLiteDB.h"
#include "global.h"

#include "DatabaseStandardEntryMenu.h"
#include "DatabaseAdvancedManipulationMenu.h"
#include "DatabaseGenerateReports.h"

bool DatabaseMainMenu ()
{
DISPLAY:
	//I call this the Moore Menu System
	const static std::vector<std::pair<int, std::pair<std::string, fptr_t>>> menuOptions = {
		//exits the application
		{1, {"Exit application", [] () -> bool { return false; }}},
		//this is the transaction stuff
		{1, {"Standard data entry", DatabaseStandardEntryMenu }},
		//generates user reports
		{1, {"Generate database reports", DatabaseGenerateReports }},
		//this does single table inserts/updates/delets and allows for transaction opetations
		{3, {"Advanced data manipulation", DatabaseAdvancedManipulationMenu }},
	};

	std::cout << "Moore's C&R Central database" << lf
		<< "ACCESS LEVEL " << accessLevel << "." << lf;
	if (inTransaction)
		std::cout << "TRANSACTION IN PROGRESS" << lf << lf;
	std::cout << lf;

	std::cout << "Please select a menu option:" << lf;

	//print the menu options
	int i = 1;
	for (const auto& menuOption : menuOptions)
		if (accessLevel >= menuOption.first)
			std::cout << '\t' << i++ << ": " << menuOption.second.first << std::endl;

	//get 'n call option
	try
	{
		std::cin >> std::ws >> i;
	}
	catch (...)
	{
		HandleIstreamFailure ();
		goto DISPLAY;
	}

	//get option and execute
	if (i > menuOptions.size () || i < 1)
		goto DISPLAY;
	else
		if (menuOptions[i - 1].second.second ())
			goto DISPLAY;
		else
			return true;
}