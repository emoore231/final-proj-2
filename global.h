#pragma once

#include <string>
#include <map>
#include <vector>
#include <iostream>

namespace SQL
{
	class Transaction;
}

#include <setjmp.h>

//my old friend
extern jmp_buf jpr;

//the access level of the user
extern int accessLevel;
//if the database is in a transaction
extern bool inTransaction;
//this is only allocated if there is a transaction
extern SQL::Transaction* currentTransactionDatabase;
extern int currentTransactionOperationCount;

inline void HandleIstreamFailure ()
{
    if (std::cin.eof ())
        std::cout << "EOF DETECTED ON STDIN, TERMINATING\nTHANKS BASH!", exit (1);

    std::cout << "OPTION NOT RECOGNIZED" << std::endl;
    std::cin.clear ();
    std::string s;
    std::cin >> s;
}

typedef bool (*fptr_t)();

#define lf std::endl


//list of tables
enum class Table
{
	JURISDICTION,
	LOCATION,
	MANUFACTURER,
	SCHEDULE,
	JOB_POSITION,
	PRODUCT_CATEGORY,
	PRODUCT_TYPE,
	REGULATION,
	PURCHASER,
	SELLER,
	OFFICE,
	EMPLOYEE,
	PRODUCT_REGULATION,
	WAREHOUSE,
	MANAGER,
	MANAGEMENT,
	NEGOTIATER,
	SALES_CONTRACT,
	ACQUISITION_CONTRACT,
	PRODUCT,
	TRANSFER,
	PRODUCT_CAPACITY,
	PRODUCT_CAPACITY_RESTRICTION,
	EMPLOYEE_CHANGE,
};

//list of types / fk constraints
enum class Type
{
	JURISDICTION = (short)Table::JURISDICTION,
	LOCATION = (short)Table::LOCATION,
	MANUFACTURER = (short)Table::MANUFACTURER,
	SCHEDULE = (short)Table::SCHEDULE,
	JOB_POSITION = (short)Table::JOB_POSITION,
	PRODUCT_CATEGORY = (short)Table::PRODUCT_CATEGORY,
	PRODUCT_TYPE = (short)Table::PRODUCT_TYPE,
	REGULATION = (short)Table::REGULATION,
	PURCHASER = (short)Table::PURCHASER,
	SELLER = (short)Table::SELLER,
	OFFICE = (short)Table::OFFICE,
	EMPLOYEE = (short)Table::EMPLOYEE,
	PRODUCT_REGULATION = (short)Table::PRODUCT_REGULATION,
	WAREHOUSE = (short)Table::WAREHOUSE,
	MANAGER = (short)Table::MANAGER,
	MANAGEMENT = (short)Table::MANAGEMENT,
	NEGOTIATER = (short)Table::NEGOTIATER,
	SALES_CONTRACT = (short)Table::SALES_CONTRACT,
	ACQUISITION_CONTRACT = (short)Table::ACQUISITION_CONTRACT,
	PRODUCT = (short)Table::PRODUCT,
	TRANSFER = (short)Table::TRANSFER,
	PRODUCT_CAPACITY = (short)Table::PRODUCT_CAPACITY,
	PRODUCT_CAPACITY_RESTRICTION = (short)Table::PRODUCT_CAPACITY_RESTRICTION,
	EMPLOYEE_CHANGE = (short)Table::EMPLOYEE_CHANGE,
	INTEGER,
	TEXT,
};

//table to str
extern const std::map<Table, std::string> TableToString;

extern const std::map<Type, std::string> TypeToString;

extern const std::map<std::string, Table> StringToTable;

//table to insertion query
extern const std::map<Table, std::string> TableToInsertionQuery;

extern const std::map<Table, std::string> TableToSelectionQuery;

extern const std::map<Table, std::string> TableToFilterQuery;

//table to the list of fields
//used for type checking / lookup
extern const std::map<Table, std::vector<std::pair<std::string, Type>>> TableToInsertionFields; 
extern const std::map<Table, std::string> TableToUpdateQuery;
extern const std::map<Table, std::string> TableToDeleteQuery;