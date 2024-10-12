#include "DatabaseStandardEntryMenu.h"
#include <string>
#include <iostream>
#include <vector>
#include "global.h"

bool DatabaseStandardEntryMenu ()
{
DISPLAY:

	const static std::vector<std::pair<int, std::pair<std::string, fptr_t>>> menuOptions = {
		{1, {"Return to previous menu", [] () -> bool { return false; }}},
		{1, {"Add new sales contract", nullptr }},
		{1, {"Add new acquisition contract", nullptr }},
		{1, {"Add new transfer", nullptr }},
	};

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