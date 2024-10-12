#include "DatabaseGenerateReports.h"
#include "global.h"
#include <vector>
#include <string>
#include <iostream>
#include "SQLiteDB.h"

static inline bool SearchNegotiatorsByValue ();
static inline bool SearchManagersByRegulations ();
static inline bool SearchLocationsByHandguns();



bool DatabaseGenerateReports ()
{
DISPLAY:

	const static std::vector<std::pair<int, std::pair<std::string, fptr_t>>> menuOptions = {
		{1, {"Return to previous menu", [] () -> bool { return false; }}},
		{1, {"Search negotiators by total value of contracts", SearchNegotiatorsByValue }},
		{1, {"Search managers by number of regulations on products that they signed off on", SearchManagersByRegulations }},
		{1, {"Search top locations by number of handgun sales", SearchLocationsByHandguns }},
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


static inline bool SearchNegotiatorsByValue () try 
{
	static constexpr const char* Query = R"(
-- Negotiators by value of contracts

SELECT N, TVA, TVS
FROM 
(
    SELECT
        EMPLOYEE.NAME AS N, 
        SUM(ACQUISITION_CONTRACT.GRAND_TOTAL) AS TVA,
        SUM(SALES_CONTRACT.GRAND_TOTAL) AS TVS 
    FROM NEGOTIATER
    JOIN ACQUISITION_CONTRACT 
        ON ACQUISITION_CONTRACT.NEGOTIATER_ID = NEGOTIATER.ID
    JOIN SALES_CONTRACT 
        ON SALES_CONTRACT.NEGOTIATER_ID = NEGOTIATER.ID
    JOIN EMPLOYEE 
        ON NEGOTIATER.EMPLOYEE_ID = EMPLOYEE.ID
    GROUP BY EMPLOYEE.ID
)
ORDER BY TVA DESC, TVS DESC;
)";

    SQL::Database db;
    db.Open ("master.db");
    SQL::Result res = db.ExecutePrepared (SQL::Query (Query));

    for (const auto& i : res)
    {
        std::cout << "Negotiator: " << i.at ("N") << lf
            << "Total value of Acquisition Contracts: " << i.at ("TVA") << lf
            << "Total value of Sales Contracts: " << i.at ("TVS") << lf << lf;
    }

    db.Close ();

	return true;
}
catch (const std::ios_base::failure&)
{
    std::cout << "A generic stream error occured" << lf;
    return true;
}
catch (const SQL::Exception& ex)
{
    std::cout << "An error occured while managing the database" << lf
        << "Code: " << ex.Code << lf
        << "Message: " << ex.what () << lf;
    return true;
}
catch (...)
{
    std::cout << "An error occured while performing the operation, it is recomended that the application is terminated." << lf;
    return true;
}

static inline bool SearchManagersByRegulations () try
{
    static constexpr const char* Query = R"(
-- Managers who signed off on acquisition products ordered by the number of regulations that the product had


WITH _MOST_REGULATED_PRODUCTS AS 
(
    SELECT * FROM 
    (
        SELECT COUNT(REGULATION.ID) AS _REGULATION_COUNT, PRODUCT.ID AS _PRODUCT_ID, PRODUCT_CATEGORY.NAME AS _PRODUCT_TYPE
        FROM PRODUCT
        JOIN PRODUCT_TYPE
            ON PRODUCT.PRODUCT_TYPE_ID = PRODUCT_TYPE.ID
        JOIN PRODUCT_CATEGORY
            ON PRODUCT_TYPE.PRODUCT_CATEGORY_ID = PRODUCT_CATEGORY.ID
        JOIN PRODUCT_REGULATION
            ON PRODUCT_REGULATION.PRODUCT_CATEGORY_ID = PRODUCT_CATEGORY.ID
        JOIN REGULATION
            ON PRODUCT_REGULATION.REGULATION_ID = REGULATION.ID
        GROUP BY REGULATION.ID
    )
    ORDER BY _REGULATION_COUNT DESC
)

SELECT _MANAGER AS 'M', _PRODUCT_TYPE AS 'PT', _REGULATION_COUNT AS 'NR'
FROM
(
    SELECT 
        EMPLOYEE.NAME AS _MANAGER,
        _PRODUCT_TYPE,
        _REGULATION_COUNT
    FROM MANAGER
    JOIN EMPLOYEE
        ON MANAGER.EMPLOYEE_ID = EMPLOYEE.ID
    JOIN ACQUISITION_CONTRACT
        ON ACQUISITION_CONTRACT.MANAGER_ID = MANAGER.ID
    JOIN PRODUCT
        ON PRODUCT.ACQUISITION_CONTRACT_ID = ACQUISITION_CONTRACT.ID
    JOIN _MOST_REGULATED_PRODUCTS
        ON _MOST_REGULATED_PRODUCTS._PRODUCT_ID = PRODUCT.ID
    ORDER BY  _REGULATION_COUNT DESC
);
)";

    SQL::Database db;
    db.Open ("master.db");
    SQL::Result res = db.ExecutePrepared (SQL::Query (Query));

    for (const auto& i : res)
    {
        std::cout << "Manager: " << i.at ("M") << lf
            << "Most regulated product type purchased: " << i.at ("PT") << lf
            << "Number of regulations: " << i.at ("NR") << lf << lf;
    }

    db.Close ();

	return true;
}
catch (const std::ios_base::failure&)
{
    std::cout << "A generic stream error occured" << lf;
    return true;
}
catch (const SQL::Exception& ex)
{
    std::cout << "An error occured while managing the database" << lf
        << "Code: " << ex.Code << lf
        << "Message: " << ex.what () << lf;
    return true;
}
catch (...)
{
    std::cout << "An error occured while performing the operation, it is recomended that the application is terminated." << lf;
    return true;
}

static inline bool SearchLocationsByHandguns () try
{
    static constexpr const char* Query = R"(
-- Top locations by handgun sales

SELECT LOCATION.STATE AS 'S', COUNT(PRODUCT.ID) AS 'NHS'
FROM LOCATION
JOIN OFFICE 
    ON OFFICE.LOCATION_ID = LOCATION.ID
JOIN WAREHOUSE
    ON WAREHOUSE.OFFICE_ID = OFFICE.ID
JOIN PRODUCT
    ON PRODUCT.WAREHOUSE_ID = WAREHOUSE.ID
JOIN SALES_CONTRACT
    ON PRODUCT.SALES_CONTRACT_ID = SALES_CONTRACT.ID
JOIN PRODUCT_TYPE 
    ON PRODUCT.PRODUCT_TYPE_ID = PRODUCT_TYPE.ID
JOIN PRODUCT_CATEGORY
    ON PRODUCT_TYPE.PRODUCT_CATEGORY_ID = PRODUCT_CATEGORY.ID
WHERE PRODUCT_CATEGORY.NAME = 'Handguns'
GROUP BY LOCATION.STATE;
)";

    SQL::Database db;
    db.Open ("master.db");
    SQL::Result res = db.ExecutePrepared (SQL::Query (Query));

    for (const auto& i : res)
    {
        std::cout << "State: " << i.at ("S") << lf
            << "Number of handguns sold: " << i.at ("NHS") << lf << lf;
    }

    db.Close ();
    
	return true;
}
catch (const std::ios_base::failure&)
{
    std::cout << "A generic stream error occured" << lf;
    return true;
}
catch (const SQL::Exception& ex)
{
    std::cout << "An error occured while managing the database" << lf
        << "Code: " << ex.Code << lf
        << "Message: " << ex.what () << lf;
    return true;
}
catch (...)
{
    std::cout << "An error occured while performing the operation, it is recomended that the application is terminated." << lf;
    return true;
}