#include "DatabaseStandardEntryMenu.h"
#include <string>
#include <iostream>
#include <vector>
#include "global.h"
#include "SQLiteDB.h"

bool AddNewAcquisitionContract ();
//static inline std::string LookupForeignKey (Table table);

bool DatabaseStandardEntryMenu ()
{
DISPLAY:
	//menu otps
	const static std::vector<std::pair<int, std::pair<std::string, fptr_t>>> menuOptions = {
		{1, {"Return to previous menu", [] () -> bool { return false; }}},
		{1, {"Add new acquisition contract", AddNewAcquisitionContract }},
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


bool AddNewAcquisitionContract ()
{
	//get products
	std::vector<std::vector<std::string>> productsToInsert;

	std::cout << "Enter the products or QUIT to stop adding products" << lf;

	do try
	{
		std::cout << "Enter product type ID or LOOKUP to lookup a value or QUIT to stop adding products" << lf;
		std::string productTypeId;
		std::cin >> std::ws >> productTypeId;
		if (productTypeId == "LOOKUP" || productTypeId == "lookup")
			productTypeId = LookupForeignKey (Table::PRODUCT_TYPE);
		else if (productTypeId == "QUIT" || productTypeId == "quit")
			break;
		std::cout << "Enter warehouse ID or LOOKUP to lookup a value or QUIT to stop adding products" << lf;
		std::string warehouseId;
		std::cin >> std::ws >> warehouseId;
		if (warehouseId == "LOOKUP" || warehouseId == "lookup")
			warehouseId = LookupForeignKey (Table::WAREHOUSE);
		std::cout << "Enter product serial number" << lf;
		std::string productSerialNumber;
		std::cin >> std::ws >> productSerialNumber;

		productsToInsert.push_back ({"0", "1", warehouseId, productTypeId, productSerialNumber});
	}
	catch (...)
	{
		std::cout << "INPUT NOT RECOGNIZED" << lf;
	}
	while (true);

	std::vector<std::string> acquisitionContract;
	//get contract info
	try
	{
		{
			std::cout << "Enter seller ID or LOOKUP to lookup a value" << lf;
			std::string sellerId;
			std::cin >> std::ws >> sellerId;
			if (sellerId == "LOOKUP" || sellerId == "lookup")
				sellerId = LookupForeignKey (Table::SELLER);
			acquisitionContract.push_back (sellerId);
		}
		{
			std::cout << "Enter juridiction ID or LOOKUP to lookup a value" << lf;
			std::string jurisdictionId;
			std::cin >> std::ws >> jurisdictionId;
			if (jurisdictionId == "LOOKUP" || jurisdictionId == "lookup")
				jurisdictionId = LookupForeignKey (Table::JURISDICTION);
			acquisitionContract.push_back (jurisdictionId);
		}
		{
			std::cout << "Enter negotiator ID or LOOKUP to lookup a value" << lf;
			std::string negotiatorId;
			std::cin >> std::ws >> negotiatorId;
			if (negotiatorId == "LOOKUP" || negotiatorId == "lookup")
				negotiatorId = LookupForeignKey (Table::NEGOTIATER);
			acquisitionContract.push_back (negotiatorId);
		}
		{
			std::cout << "Enter manager ID or LOOKUP to lookup a value" << lf;
			std::string managerId;
			std::cin >> std::ws >> managerId;
			if (managerId == "LOOKUP" || managerId == "lookup")
				managerId = LookupForeignKey (Table::MANAGER);
			acquisitionContract.push_back (managerId);
		}
		{
			std::cout << "Enter grand total" << lf;
			std::string managerId;
			std::cin >> std::ws >> managerId;
			acquisitionContract.push_back (managerId);
		}
		{
			std::cout << "Enter date (yyyy-mm-dd)" << lf;
			std::string managerId;
			std::cin >> std::ws >> managerId;
			acquisitionContract.push_back ("DATETIME(" + managerId + ")");
		}
	}
	catch (...)
	{
		std::cout << "An unknown error occured" << lf;
		return true;
	}

	try
	{//commit to database
		SQL::Database database ("master.db");
		SQL::Transaction transaction = database.BeginTransaction ();
		database.Close ();

		SQL::Result res = transaction.ExecutePrepared (SQL::ParameterizedQuery (R"(INSERT INTO ACQUISITION_CONTRACT (PURCHASER_ID, JURISDICTION_ID, NEGOTIATER_ID, MANAGER_ID, GRAND_TOTAL, DATE) VALUES (?,?,?,?,?,?) RETURNING ID;)", acquisitionContract));
		//!!!
		//this needs sqlite 3.35.0, if it fails, build with compile.sh
		//!!!

		std::string acquisitionContractId;
		for (auto& i : res.Data)
		{
			acquisitionContractId = i.at ("ID");
		}

		for (auto& i : productsToInsert)
		{
			i[0] = acquisitionContractId;
			transaction.ExecutePrepared (SQL::ParameterizedQuery (TableToInsertionQuery.at (Table::PRODUCT), i));


		}

		//if failure, destructor will roll back
		transaction.Commit ();
	}
	catch (const SQL::Exception& e)
	{
		std::cout << "DATABASE ERROR: " << e.Code << "; MESSAGE: " << e.what () << lf;
		return true;
	}

	return true;
}



