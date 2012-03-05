/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: database.h
 * Description: deals with the database connection (MySQLe or SQLite).
 *
 * @author Mathieu Grzybek on 2010-05-16
 * @copyright 2010 Mathieu Grzybek. All rights reserved.
 * @version $Id: code-gpl-license.txt,v 1.2 2004/05/04 13:19:30 garry Exp $
 *
 * @see The GNU Public License (GPL) version 3 or higher
 *
 *
 * OWS is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 * for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write to the Free Software Foundation, Inc.,
 * 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <fstream>
#include <iostream>

#include <boost/regex.hpp>
#include <boost/foreach.hpp>
#include <boost/thread/mutex.hpp>

#include "common.h"

#ifdef USE_MYSQL
#include <mysql.h>
#endif
#ifdef USE_SQLITE
//#include <boost/filesystem.hpp>
#include <sqlite3.h>
#endif

// namespace ows {

typedef	std::vector<std::string>			v_row;
typedef	std::vector<v_row>					v_v_row;
typedef	std::vector<std::string>			v_queries;

///////////////////////////////////////////////////////////////////////////////
/*
class Database {
public:
	virtual	bool	init_node(const std::string*)	= 0;
	virtual	bool	execute(const std::string*)		= 0;
	virtual	bool	shutdown()						= 0;
};
*/
///////////////////////////////////////////////////////////////////////////////

#ifdef USE_MYSQL

//class Mysql : public Database {
class Mysql {
public:
	Mysql();
	~Mysql();

	/*
	 * prepare
	 *
	 * Prepares the domain to be used :
	 * - creates the embedded database engine
	 * - creates the schema
	 * - loads the skeleton from file
	 *
	 * @arg node_name	: node's name
	 * @arg db_skeleton	: the SQL file to use
	 *
	 * @return true		: success
	 */
	bool	prepare(const std::string* node_name, const std::string* db_skeleton);

	/*
	 * execute
	 *
	 * Executes an SQL query without result
	 *
	 * @arg query	: SQL query
	 * @return true	: success
	 */
	bool	standalone_execute(const v_queries* queries);

	/*
	 * query_one_row
	 *
	 * Executes a SQL query returing a single-row result
	 *
	 * @arg query	: SQL query
	 * @return true	: success
	 */
	v_row	query_one_row(const char* query);

	/*
	 * query_full_result
	 *
	 * Executes a SQL query returning several rows
	 *
	 * @arg query		: SQL query
	 * @arg	result		: the rows
	 * @return true		: the success
	 */
	v_v_row*	query_full_result(const char* query);

	/*
	 * get_inserted_id
	 *
	 * Gives mysql_insert_id(). It is used to update the object's id
	 *
	 * @arg		: none
	 * @return	: the id
	 */
	int		get_inserted_id();

	/*
	 * shutdown
	 *
	 * Makes a clean database shutdown
	 *
	 * @return true
	 */
	bool	shutdown();

private:
	/*
	 * mysql
	 *
	 * This is the MySQL server
	 *
	 */
	MYSQL*			mysql;

	/*
	 * results
	 *
	 * Used to store the query's result
	 */
//	MYSQL_RES*		results;

	/*
	 * record
	 *
	 * Used to store each row of the result
	 */
//	MYSQL_ROW		record;

	/*
	 * updates_mutex
	 *
	 * This mutex is used to secure the database's access
	 * MySQLe does not support multi access
	 *
	 */
//	boost::mutex	updates_mutex;

	/*
	 * execute
	 *
	 * Executes an SQL query without result
	 *
	 * @arg query	: SQL query
	 * @arg	mysql	: the MySQL connector
	 * @return true	: success
	 */
	bool	atomic_execute(const std::string& query, MYSQL* m);

	/*
	 * load_file
	 *
	 * Reads an SQL file and execute the queries
	 *
	 * @arg file_path	: the SQL file
	 * @return true		: success
	 */
	bool		load_file(const char* node_name, const char* file_path);

	/*
	 * init
	 *
	 * Initializes the MySQL connection to the embedded server
	 * - must be called on each thread
	 * - basically it is called at the beginning of each method
	 */
	MYSQL*		init();

	/*
	 * end
	 *
	 * Ends the MySQL connection to the embedded server
	 * - must be called on each thread
	 * - basically it is called at the end of each method
	 */
	void		end(MYSQL* db);

	/*
	 * translate_into_db
	 *
	 * Replaces the dots by underlines to be compatible with
	 * MySQL's schemas' syntax
	 *
	 * @arg str	: the string to convert
	 * @return	: the translated string

	 */
	std::string	translate_into_db(const std::string*);

	/*
	 * translate_into_prog
	 *
	 * Replaces the underlines by bots to be compatible with
	 * OWS's syntax
	 *
	 * @arg str	: the string to convert
	 * @return	: the translated string
	 */
	std::string translate_into_prog(const std::string*);
};

#endif

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_SQLITE

//class Sqlite : public Database {
class Sqlite {
public:
	Sqlite();
	~Sqlite();

	/*
	 * prepare
	 *
	 * Prepares the domain to be used :
	 * - creates the database file from the template
	 *
	 * @arg node_name	: node's name
	 * @arg db_skeleton	: the SQL skeleton to use
	 *
	 * @return true		: success
	 */
	bool	prepare(const char* node_name, const std::string* db_data, const std::string* db_skeleton );

	/*
	 * standaline_execute
	 *
	 * Executes an SQL query without result
	 *
	 * @arg query	: SQL queries
	 * @return true	: success
	 */
	bool	standalone_execute(const v_queries* queries);

	/*
	 * query_one_row
	 *
	 * Executes a SQL query returing a single-row result
	 *
	 * @arg query	: SQL query
	 * @return true	: success
	 */
	v_row	query_one_row(const char* query);

	/*
	 * query_full_result
	 *
	 * Executes a SQL query returning several rows
	 *
	 * @arg query		: SQL query
	 * @arg	result		: the rows
	 * @return true		: the success
	 */
	v_v_row*	query_full_result(const char* query);

	/*
	 * get_inserted_id
	 *
	 * Gives mysql_insert_id(). It is used to update the object's id
	 *
	 * @arg		: none
	 * @return	: the id
	 */
	int		get_inserted_id();

	/*
	 * shutdown
	 *
	 * Makes a clean database shutdown
	 *
	 * @return true
	 */
	bool	shutdown();

private:
	/*
	 * updates_mutex
	 *
	 * This mutex is used to secure the database's access
	 * MySQLe does not support multi access
	 *
	 */
//	boost::mutex	updates_mutex;

	/*
	 * pp_db
	 *
	 * This SQLite db handler
	 */
//	sqlite3*		p_db;
	std::string		db;

	/*
	 * execute
	 *
	 * Executes an SQL query without result
	 *
	 * @arg query	: SQL query
	 * @arg	mysql	: the MySQL connector
	 * @return true	: success
	 */
	bool	atomic_execute(const std::string& query, sqlite3* p_db);

	/*
	 * load_file
	 *
	 * Reads an SQL file and execute the queries
	 *
	 * @arg file_path	: the SQL file
	 * @return true		: success
	 */
	bool	load_file(const char* node_name, const char* file_path);

	/*
	 * init
	 *
	 * Initializes the SQLite connection to the embedded server
	 * - must be called on each thread
	 * - basically it is called at the beginning of each method
	 */
	sqlite3*	init();

	/*
	 * end
	 *
	 * Ends the SQLite connection to the embedded server
	 * - must be called on each thread
	 * - basically it is called at the end of each method
	 */
	void		end(sqlite3* p_db);

	/*
	 * callback
	 *
	 * The SQLite's callback function from the quickstart guide
	 * Must be declared and defined here, otherwise it won't be usable into
	 * sqlite3_exec calls
	 */
	static int	callback(void *NotUsed, int argc, char **argv, char **azColName) {
		int i;

		for ( i=0; i < argc ; i++ ) {
			std::cout << azColName[i] << " = ";

			if ( argv[i] )
				std::cout << argv[i];
			else
				std::cout << "NULL";

			std::cout << std::endl;
		}

		return 0;
	}
};

#endif // USE_SQLITE

//} // namespace ows

#endif // DATABASE_H
