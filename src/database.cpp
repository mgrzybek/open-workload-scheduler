/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: database.cpp
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

#include "database.h"

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_MYSQL

Mysql::Mysql() {
}

Mysql::~Mysql() {
}

///////////////////////////////////////////////////////////////////////////////

bool	Mysql::prepare(const std::string* node_name, const std::string* db_skeleton) {
	static char* server_args[] = {
		"this_program",	// this string is not used
		"--datadir=."
	};
	static char* server_groups[] = {
		"embedded",
		"server",
		"this_program_SERVER",
		(char *)NULL
	};
	std::string	query;
	v_queries	queries;


	// DB init
	if ( mysql_library_init(sizeof(server_args) / sizeof(char *), server_args, server_groups) )
		std::cerr << "could not initialize MySQL library" << std::endl;

	if ( ! mysql_thread_safe() ) {
		std::cerr << "MySQL is NOT theadsafe !" << std::endl;
		return false;
	}

	// Creates the schema
	query = "CREATE SCHEMA IF NOT EXISTS ";
	query += this->translate_into_db(node_name);
	query += " DEFAULT CHARACTER SET latin1;";

	queries.insert(queries.end(), query);
	query.clear();

	if ( this->standalone_execute(&queries) == false ) {
		return false;
	}

	// Loads the skeleton from file
	return this->load_file(node_name->c_str(), db_skeleton->c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool	Mysql::atomic_execute(const std::string& query, MYSQL* m) {
	MYSQL_RES*	res;
	MYSQL_ROW	row;
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	std::cout << query.c_str() << std::endl;

	if ( query.empty() == true or boost::regex_match(query, empty_string) == true ) {
		std::cerr << "Error : query is empty !" << std::endl;
		return false;
	}


	if ( mysql_query(m, query.c_str()) != 0 ) {
		std::cerr << query.c_str() << std::endl << mysql_error(m);
		return false;
	}

	res = mysql_store_result(m);
	if (res)
		while ( ( row = mysql_fetch_row(res) ) )
			for ( uint i=0 ; i < mysql_num_fields(res) ; i++ )
				std::cerr << row[i] << std::endl;
	else
		if ( mysql_field_count(m) != 0 ) {
			std::cerr << "Erreur : " << mysql_error(m) << std::endl;
			mysql_free_result(res);
			return false;
		}

	mysql_free_result(res);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Mysql::standalone_execute(const v_queries* queries) {
	MYSQL*		local_mysql	= this->init();
	std::string	query		= "START TRANSACTION;";

	if ( local_mysql == NULL ) {
		std::cerr << "Error: local_mysql is null" << std::endl;
		return false;
	}

	if ( this->atomic_execute(query, local_mysql) == false ) {
		this->end(local_mysql);
		return false;
	}

	BOOST_FOREACH(std::string q, *queries) {
		if ( this->atomic_execute(q, local_mysql) == false ) {
			query = "ROLLBACK;";
			this->atomic_execute(query, local_mysql);
			this->end(local_mysql);
			return false;
		}
	}

	query = "COMMIT;";

	if ( this->atomic_execute(query, local_mysql) == false ) {
		this->end(local_mysql);
		return false;
	}

	this->end(local_mysql);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

v_row	Mysql::query_one_row(const char* query) {
	MYSQL_RES*	res;
	MYSQL_ROW	row;
	MYSQL*		local_mysql = this->init();
	v_row		result;

	if ( query == NULL )
		return result;


	if ( mysql_query(local_mysql, query) != 0 ) {
		std::cerr << query << std::endl << mysql_error(local_mysql);
		return result;
	}

	res = mysql_store_result(local_mysql);
	if (res)
		while ( ( row = mysql_fetch_row(res) ) )
			for ( uint i=0 ; i < mysql_num_fields(res) ; i++ )
				result.push_back(std::string(row[i]));
	else
		if ( mysql_field_count(local_mysql) != 0 )
			std::cerr << "Erreur : " << mysql_error(local_mysql) << std::endl;

	mysql_free_result(res);
	this->end(local_mysql);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

v_v_row*	Mysql::query_full_result(const char* query) {
	MYSQL_RES*	res;
	MYSQL_ROW	row;
	MYSQL*		local_mysql = this->init();
	v_row		line;
	v_v_row*	result = NULL;

	if ( query == NULL )
		return NULL;


	if ( mysql_query(local_mysql, query) != 0 ) {
		std::cerr << query << std::endl << mysql_error(local_mysql);
		return NULL;
	}

	res = mysql_store_result(local_mysql);

	if (res) {
		result = new v_v_row();
		while ( ( row = mysql_fetch_row(res) ) ) {
			line.clear();

			for ( uint i=0 ; i < mysql_num_fields(res) ; i++ ) {
				if ( row[i] != NULL )
					line.push_back(std::string(row[i]));
				else
					line.push_back(std::string("NULL"));
			}

			result->push_back(line);
		}
	} else
		if ( mysql_field_count(local_mysql) != 0 )
			std::cerr << "Erreur : " << mysql_error(local_mysql) << std::endl;

	mysql_free_result(res);
	this->end(local_mysql);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

// TODO: make sure it works thanks to the domain's mutex
int	Mysql::get_inserted_id() {
	MYSQL*	local_mysql	= this->init();
	int		result;

	result = mysql_insert_id(local_mysql);

	this->end(local_mysql);
	return result;
}

///////////////////////////////////////////////////////////////////////////////

bool	Mysql::shutdown() {
	mysql_library_end();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Mysql::load_file(const char* node_name, const char* file_path) {
	std::ifstream	f (file_path, std::ifstream::in);
	std::string		line;
	std::string		query;
	v_queries		queries;
	boost::regex	end_of_query("^.*?;$", boost::regex::perl);
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	query = "USE ";
	query += node_name;
	query += ";";
	queries.insert(queries.end(), query);
	query.clear();

	if ( f.is_open() ) {
		while ( ! f.eof() ) {
			getline(f, line);
			query += " ";
			if ( boost::regex_match(line, end_of_query) == true ) {
				query += line;
				queries.insert(queries.end(), query);
				query.clear();
			} else
				query += line;
		}
	} else
		return false;

	if ( query.empty() == false and boost::regex_match(query, empty_string) == false  )
		queries.insert(queries.end(), query);

	return this->standalone_execute(&queries);
}

///////////////////////////////////////////////////////////////////////////////

MYSQL*		Mysql::init() {
	MYSQL*	local_mysql = NULL;

	mysql_thread_init();

	local_mysql = mysql_init(NULL);

	if ( local_mysql == NULL ) {
		std::cerr << "Error: cannot init a MySQL connector" << std::endl;
		return NULL;
	}

	mysql_options(local_mysql, MYSQL_READ_DEFAULT_GROUP, "libmysqld_threaded_client");
	if ( ! mysql_real_connect(local_mysql, NULL, NULL, NULL, NULL, 0, NULL, 0) ) {
		std::cerr << "Error: cannot connect: " << mysql_error(local_mysql) << std::endl;
		return NULL;
	}

	return local_mysql;
}

///////////////////////////////////////////////////////////////////////////////

void	Mysql::end(MYSQL* db) {
	mysql_close(db);
	mysql_thread_end();
}

///////////////////////////////////////////////////////////////////////////////

std::string	Mysql::translate_into_db(const std::string* str) {
	// TODO : find the right code for "."
	boost::regex	dot("[^[:word:]]", boost::regex::perl);
	return boost::regex_replace(*str, dot, "_");
}

///////////////////////////////////////////////////////////////////////////////

std::string Mysql::translate_into_prog(const std::string* str) {
	boost::regex	underline("_", boost::regex::perl);
	return boost::regex_replace(*str, underline, ".");
}

#endif // USE_MYSQL

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_SQLITE

Sqlite::Sqlite() {
//	if ( sqlite3_threadsafe() != SQLITE_OK )
//		throw "SQLite3 is not threadsafe !";

//	if ( sqlite3_config(SQLITE_CONFIG_SERIALIZED) != SQLITE_OK )
//		throw "SQLite3 cannot use SQLITE_CONFIG_SERIALIZED";

	if ( sqlite3_config(SQLITE_CONFIG_MULTITHREAD) != SQLITE_OK )
		throw "SQLite3 cannot use SQLITE_CONFIG_MULTITHREAD";

	if ( sqlite3_initialize() != SQLITE_OK )
		throw "SQLite3 cannot initialize";
}

Sqlite::~Sqlite() {
	sqlite3_shutdown();
}

///////////////////////////////////////////////////////////////////////////////

bool	Sqlite::prepare(const char* node_name, const std::string* db_data, const std::string* db_skeleton ) {
	// TODO : deal with plannings' progression
	std::string	db_name("schd_1.db");
	std::string	mkdir_call("mkdir -p ");
	int			result;

	this->db += db_data->c_str();
	this->db += "/";
	this->db += node_name;
	this->db += "/";
	this->db += db_name.c_str();

	mkdir_call += db_data->c_str();
	mkdir_call += "/";
	mkdir_call += node_name;

	result = system(mkdir_call.c_str());
	if ( result != 0 ) {
		std::cerr << "Error : system error : mkdir -p returned " << result << std::endl;
		return false;
	}

	return this->load_file(node_name, db_skeleton->c_str());
}

///////////////////////////////////////////////////////////////////////////////

bool	Sqlite::atomic_execute(const std::string& query, sqlite3* p_db) {
	char*	err_msg	= 0;
	int		ret_code;

	std::cout << query.c_str() << std::endl;
	ret_code = sqlite3_exec(p_db, query.c_str(), callback, 0, &err_msg);

	if ( ret_code != SQLITE_OK ) {
		std::cerr << "Error : SQLITE error code " << ret_code << " : " << err_msg << std::endl;
		sqlite3_free(err_msg);

		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Sqlite::standalone_execute(const v_queries* queries) {
	sqlite3*	p_db	= this->init();
	std::string	query	= "BEGIN TRANSACTION;";

	if ( p_db == NULL )
		return false;

	if ( this->atomic_execute(query, p_db) == false ) {
		this->end(p_db);
		return false;
	}

	BOOST_FOREACH(std::string q, *queries) {
		if ( this->atomic_execute(q, p_db) == false ) {
			query = "ROLLBACK;";
			this->atomic_execute(query, p_db);
			this->end(p_db);
			return false;
		}
	}

	query = "COMMIT;";

	if ( this->atomic_execute(query, p_db) == false ) {
		this->end(p_db);
		return false;
	}

	this->end(p_db);
	return true;
}

///////////////////////////////////////////////////////////////////////////////

v_row	Sqlite::query_one_row(const char* query) {
	sqlite3*		p_db = this->init();
	sqlite3_stmt*	stmt;
	v_row			result;

	if ( query == NULL or p_db == NULL)
		return result;

	if ( sqlite3_prepare_v2(p_db, query, -1, &stmt, NULL) != SQLITE_OK ) {
		return result;
	}

	if ( sqlite3_step(stmt) == SQLITE_ROW ) {
		result.clear();

		for ( int i = 0 ; i < sqlite3_data_count(stmt) ; i++ )
			result.push_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i))));
	}

	sqlite3_free(stmt);
	this->end(p_db);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

v_v_row*	Sqlite::query_full_result(const char* query) {
	sqlite3*		p_db = this->init();
	sqlite3_stmt*	stmt;
	v_row			line;
	v_v_row*		result = NULL;

	if ( query == NULL or p_db == NULL)
		return result;

	if ( sqlite3_prepare_v2(p_db, query, -1, &stmt, NULL) != SQLITE_OK ) {
		return result;
	}

	while ( sqlite3_step(stmt) == SQLITE_ROW ) {
		line.clear();

		for ( int i = 0 ; i < sqlite3_data_count(stmt) ; i++ )
			line.push_back(std::string(reinterpret_cast<const char*>(sqlite3_column_text(stmt, i))));

		result->push_back(line);
	}

	sqlite3_free(stmt);
	this->end(p_db);

	return result;
}

///////////////////////////////////////////////////////////////////////////////
// TODO: make it more robust
int		Sqlite::get_inserted_id() {
	sqlite3*	p_db = this->init();

	return sqlite3_last_insert_rowid(p_db);
}

///////////////////////////////////////////////////////////////////////////////

bool	Sqlite::shutdown() {/*
	if ( sqlite3_close(this->p_db) == SQLITE_OK )
		return true;
*/
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Sqlite::load_file(const char* node_name, const char* file_path) {
	std::ifstream	f (file_path, std::ifstream::in);
	std::string		line;
	std::string		query;
	v_queries		queries;
	boost::regex	end_of_query("^.*?;$", boost::regex::perl);

	// TODO: throw an exception if the file does not exist
	if ( f.is_open() ) {
		while ( ! f.eof() ) {
			getline(f, line);
			query += " ";
			if ( boost::regex_match(line, end_of_query) == true ) {
				query += line;
				queries.insert(queries.end(), query);
				query.clear();
			} else
				query += line;
		}
	} else
		return false;

	queries.insert(queries.end(), query);

	return this->standalone_execute(&queries);
}

///////////////////////////////////////////////////////////////////////////////

sqlite3*	Sqlite::init() {
	sqlite3*	p_db;

	if ( sqlite3_open_v2(this->db.c_str(), &p_db, SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE, NULL) != SQLITE_OK ) {
		std::cerr << "Error : SQLITE error : " << sqlite3_errmsg(p_db) << std::endl;
		return NULL;
	}

	if ( p_db == NULL ) {
		std::cerr << "Error : SQLITE cannot create pointer : not enough memory" << std::endl;
		sqlite3_close(p_db);
		return NULL;
	}

	return p_db;
}

///////////////////////////////////////////////////////////////////////////////

void	Sqlite::end(sqlite3* p_db) {
	sqlite3_close(p_db);
}


#endif // USE_SQLITE
