#pragma once

#include <stdexcept>
#include <vector>
#include <string>
#include <map>
#include <iostream>
#include <iomanip>

//the only good thing about these c++ casts is that they only have 1 parenthesis for pointer conversions of member types
//they are useless clutter in all other cases
#ifndef sc
#define sc static_cast
#endif
#ifndef rc
#define rc reinterpret_cast
#endif
#ifndef dc
#define dc dynamic_cast
#endif
#ifndef cc
#define cc const_cast
#endif

//If you are interested in using this class for your purposes,
//Our sales department is open and ready!

//happy little wrappers for the SQLite database
//says the guy that hates oop
namespace SQL
{
	struct Exception : public std::runtime_error
	{
		const size_t Code;
		Exception (int ExitCode, const std::string& ErrorMessage) :
			std::runtime_error (ErrorMessage),
			Code (ExitCode)
		{}
	};

	//simple query
	class Query
	{
	private:
		std::string _Query;

	public:
		Query (std::string q);

		Query (const char* q);

		virtual const std::string& RawQuery () const;
	};

	//using prepared statements
	class ParameterizedQuery : public Query
	{
	public:
		const std::vector<std::string> Parameters;

	public:
		ParameterizedQuery (std::string q, std::vector<std::string> params);

		ParameterizedQuery (const char* q, std::vector<std::string> params);

		ParameterizedQuery (std::string q) : Query (q)
		{}

		ParameterizedQuery (const char* q) : Query (q)
		{}

		ParameterizedQuery (const Query& q) : Query (q)
		{}

		ParameterizedQuery (const ParameterizedQuery& q) = default;
	};

	class Iterator;

	//result of query, uses input iterators to get values
	//only supposed to be interacted by with iterators
	//update: now the database connection is managed entirely in the result struct allowing for multiple queries sumoultainously (if I can spell)
	struct Result
	{
		friend class Iterator;
		friend class Database;
		friend class Transaction;
	private:

		void* statement;
		std::map<std::string, std::string> CurrentVal;//holds the current value of the iterator
		std::vector<std::string*> CurrentValVector;//to interact with fields by index, so the iterators can directly replace the vals of CurrentVal

		bool HasData;

		//only used for the transaction class
		bool* inExecution = nullptr;

	public:
		Iterator begin ();
		Iterator end ();

		//clears the iterator if it is not empty
		//this is needed because only one query can be open per connection
		void clear ();

		~Result ();
	};

	//iterator from a result
	class Iterator
	{
	public:
		//iterator defs
		using iterator_category = std::input_iterator_tag;
		using difference_type = size_t;
		using value_type = std::map<std::string, std::string>;
		using pointer = value_type*;
		using reference = value_type&;

		//goes back to the result
		Result* res;//nullptr means end()


	public:
		Iterator (const Iterator&) = delete;
		Iterator (Iterator&& rhs) noexcept(true)
		{
			res = rhs.res;
			rhs.res = NULL;
		}


		Iterator (Result* r) : res (r)
		{
			if (!r)
				res = nullptr;
			else if (!(r->HasData))//no data, set to null iterator
				res = nullptr;
		}

		//deref
		reference operator*() const;
		pointer operator->();

		// Prefix increment
		Iterator& operator++();

		// Postfix increment
		void operator++(int)
		{
			(void)this->operator++();
		}

		//since these are input iterators, very simple checking is sufficent
		friend bool operator== (const Iterator& a, const Iterator& b)
		{
			return a.res == b.res;
		}
		friend bool operator!= (const Iterator& a, const Iterator& b)
		{
			return a.res != b.res;
		}
	};

	class Transaction;

	//simple RAII class for sqlite
	class Database
	{
	private:
		//Ive finally surrendered to camelCase for private members
		//But Im still using MooreCase for all else
		void* database;
		std::string connectionString;

	public:
		//move constructor
		Database (Database&& rhs);

		//creates a new connection
		//should this have a copy constructor? The answer is absolutely!
		Database (const Database& rhs);

		//open later
		Database ();

		//open to file
		Database (const std::string& Path);

		//opens to path
		Database& Open (const std::string& Path);

		//closes
		Database& Close ();

		//executes a parameterless query and discards the result
		void Execute (const Query q);

		//executes a paremeterized (or empty) query and returns the results if they exist as an input iterator
		Result ExecutePrepared (const ParameterizedQuery& q);
		Result ExecutePrepared (const Query& q);

		Transaction BeginTransaction ();

		friend class Transaction;
		//transactional stuffs
		void Begin ()
		{
			Execute ("BEGIN;");
		}
		void Commit ()
		{
			Execute ("COMMIT;");
		}
		void Rollback ()
		{
			Execute ("ROLLBACK;");
		}

		//closes the connection if open
		~Database ();

	private:
		Result ExecutePreparedInPlace (const ParameterizedQuery& q);
		Result ExecutePreparedInPlace (const Query& q);
	};

	//a complete raii transactional class
	class Transaction
	{
		Transaction ();
		Transaction (const Transaction&) = delete;

		friend class Database;

		Database* database;

		bool inExecution;

	public:
		Transaction (Transaction&&);
		void Execute (const Query q);
		Result ExecutePrepared (const ParameterizedQuery& q);
		Result ExecutePrepared (const Query& q);

		void Commit ();
		void Rollback ();

		//closes the connection and rolls back transaction if it is still in progress
		~Transaction ();
	};
}

//quick 'n dirty ostream function
inline std::ostream& operator<<(std::ostream& _Ostr, const std::map<std::string, std::string>& rhs)
{
	for (const auto& pair : rhs)
	{
		_Ostr << std::setw (12) << std::setfill (' ') << std::left << (pair.second.size () == 0 ? "NULL" : pair.second);
	}
	return _Ostr;
}

typedef std::string secure_string;