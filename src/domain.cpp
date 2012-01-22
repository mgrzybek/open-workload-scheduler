/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: domain.cpp
 * Description: this is where the jobs are stored. The data are stored using
 * a database object.
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

#include "domain.h"

///////////////////////////////////////////////////////////////////////////////

Domain::Domain(Config* c) {
	if ( c == NULL )
		throw "The config is null";

	this->config = c;

	this->name = this->config->get_param("domain_name");

#ifdef USE_MYSQL
	this->database.prepare(this->config->get_param("node_name"), this->config->get_param("db_skeleton"));
#endif
#ifdef USE_SQLITE
	Sqlite	sqlite_buf;
	sqlite_buf.prepare(this->config->get_param("node_name")->c_str(), this->config->get_param("db_data"), this->config->get_param("db_skeleton"));
	this->databases.insert(p_nodes(*this->config->get_param("node_name"), sqlite_buf));
#endif
}

Domain::~Domain() {
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* running_node, const char* n) {
	std::string query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "INSERT IGNORE INTO ";
#ifdef USE_MYSQL
	query += running_node;
	query += ".";
#endif
	query += "node (node_name) VALUES ('";
	query += n;
	query += "');";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* running_node, const std::string& n, const int& w) {
	std::string query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "REPLACE INTO node (node_name,node_weight) VALUES ('";
	query += n.c_str();
	query += "','";
	query += boost::lexical_cast<std::string>(w);
	query += "');";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_job(Job* j) {
	std::string		query;
	const char*		running_node = j->get_node_name();
	v_queries		queries;

	this->add_node(running_node, running_node);

	this->updates_mutex.lock();

	query = "INSERT INTO ";
#ifdef USE_MYSQL
	query += running_node;
	query += ".";
#endif
	query += "job (job_name,job_cmd_line,job_node_name,job_weight) VALUES ('";
	query += j->get_name();
	query += "','";
	query += j->get_cmd_line();
	query += "','";
	query += running_node;
	query += "','";
	query += boost::lexical_cast<std::string>(j->get_weight());
	query += "');";

	queries.insert(queries.end(), query);

	BOOST_FOREACH(int i, j->get_prev()) {
		query = "INSERT INTO ";
#ifdef USE_MYSQL
		query += running_node;
		query += ".";
#endif
		query += "jobs_link (job_prev,job_next) VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += boost::lexical_cast<std::string>(j->get_id());
		query += "');";

		queries.insert(queries.end(), query);
	}

	BOOST_FOREACH(int i, j->get_next()) {
		query = "INSERT INTO ";
#ifdef USE_MYSQL
		query += j->get_node_name();
		query += ".";
#endif
		query += "jobs_link (job_next,job_prev) VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += boost::lexical_cast<std::string>(j->get_id());
		query += "');";

		queries.insert(queries.end(), query);
	}

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		j->set_id(this->database.get_inserted_id());
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		j->set_id(this->get_database(running_node)->get_inserted_id());
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.lock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job(const Job* j) {
	std::string		query;
	const char*		running_node = j->get_node_name();
	v_queries		queries;

	this->updates_mutex.lock();

	query = "UPDATE ";

#ifdef USE_MYSQL
	query += running_node;
	query += ".";
#endif
	query += "job SET (job_name,job_cmd_line,job_node_name,job_weight) VALUES ('";
	query += j->get_name();
	query += "','";
	query += j->get_cmd_line();
	query += "','";
	query += running_node;
	query += "','";
	query += j->get_weight();
	query += "') WHERE job_id = ";
	query += j->get_id();
	query += ";";

	queries.insert(queries.end(), query);

	BOOST_FOREACH(int i, j->get_prev()) {
		query = "REPLACE INTO ";
#ifdef USE_MYSQL
		query += running_node;
		query += ".";
#endif
		query += "jobs_link ('job_prev', 'job_next') VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += boost::lexical_cast<std::string>(j->get_id());
		query += "');";

		queries.insert(queries.end(), query);
	}

	BOOST_FOREACH(int i, j->get_next()) {
		query = "REPLACE INTO ";
#ifdef USE_MYSQL
		query += running_node;
		query += ".";
#endif
		query += "jobs_link ('job_next', 'job_prev') VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += boost::lexical_cast<std::string>(j->get_id());
		query += "');";

		queries.insert(queries.end(), query);
	}

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const Job* j) {
	std::string		query;
	const char*		running_node = j->get_node_name();
	v_queries		queries;

	this->updates_mutex.lock();

	query = "DELETE FROM ";
#ifdef USE_MYSQL
	query += running_node;
	query += ".";
#endif
	query += "job WHERE job_id = ";
	query += j->get_id();
	query += ";";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const std::string& running_node, const int j_id) {
	std::string	query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "DELETE FROM ";
#ifdef USE_MYSQL
	query += running_node.c_str();
	query += ".";
#endif
	query += "job ";
	query += "WHERE job_id = ";
	query += boost::lexical_cast<std::string>(j_id);
	query += ";";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const Job* j, const rpc::e_job_state::type js) {
	std::string		query;
//	char*			running_node = j->get_node_name();
	v_queries		queries;

	if ( j == NULL )
		throw "Job is null";

	this->updates_mutex.lock();

	query = "UPDATE ";
#ifdef USE_MYSQL
	query += j->get_node_name();
	query += ".";
#endif
	query += "job SET job_state = '";

	switch (js) {
		case rpc::e_job_state::WAITING: {
			query += "waiting";
			break;
		}
		case rpc::e_job_state::RUNNING: {
			query += "running";
			break;
		}
		case rpc::e_job_state::SUCCEDED: {
			query += "succeded";
			break;
		}
		case rpc::e_job_state::FAILED: {
			query += "failed";
			break;
		}
	}
	query += "' WHERE job_id = '";
	query += boost::lexical_cast<std::string>(j->get_id());
	query += "';";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const std::string& running_node, const int& j_id, const rpc::e_job_state::type& js) {
	std::string	query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "UPDATE ";
#ifdef USE_MYSQL
	query += running_node.c_str();
	query += ".";
#endif
	query += "job SET job_state = '";

	switch (js) {
		case rpc::e_job_state::WAITING: {
			query += "waiting";
			break;
		}
		case rpc::e_job_state::RUNNING: {
			query += "running";
			break;
		}
		case rpc::e_job_state::SUCCEDED: {
			query += "succeded";
			break;
		}
		case rpc::e_job_state::FAILED: {
			query += "failed";
			break;
		}
	}
	query += "' WHERE job_id = '";
	query += j_id;
	query += "';";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
#ifdef USE_SQLITE
	if ( this->get_database(running_node).standalone_execute(&queries) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

v_jobs	Domain::get_ready_jobs(const char* running_node) {
	std::string			query("SELECT job_id,job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM ");
	v_v_row*			jobs_matrix;
	v_jobs				jobs_list;
	Job*				job	= NULL;

	std::string			buf_name;
	std::string			buf_node_name;
	std::string			buf_cmd_line;
#ifdef USE_SQLITE
	Sqlite*				database;
#endif

#ifdef USE_MYSQL
	query += running_node;
	query += ".get_ready_job;";

	jobs_matrix = this->database.query_full_result(query.c_str());
#endif
#ifdef USE_SQLITE
	database = this->get_database(running_node);

	if ( database == NULL )
		return jobs_list;

	query += "get_ready_job;";
	jobs_matrix = this->get_database(running_node)->query_full_result(&query);
#endif

	if ( jobs_matrix == NULL )
		return jobs_list;

	BOOST_FOREACH(v_row job_row, *jobs_matrix) {
		delete job;

		job = new Job((Domain*)this, boost::lexical_cast<int>(job_row[0]), *this->name, job_row[3], job_row[2], (const int)boost::lexical_cast<int>(job_row[4]));
		jobs_list.push_back(*job);
	}

	delete job;
	jobs_matrix->clear();
	delete jobs_matrix;

	return jobs_list;
}

///////////////////////////////////////////////////////////////////////////////

rpc::v_jobs	Domain::get_ready_rpc_jobs(const char* running_node) {
	std::string			query("SELECT job_id,job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM ");
	v_v_row*			jobs_matrix;
	rpc::v_jobs			jobs_list;
	rpc::t_job*			job	= NULL;

	std::string			buf_name;
	std::string			buf_node_name;
	std::string			buf_cmd_line;
#ifdef USE_SQLITE
	Sqlite*				database;
#endif

#ifdef USE_MYSQL
	query += running_node;
	query += ".get_ready_job;";

	jobs_matrix = this->database.query_full_result(query.c_str());
#endif
#ifdef USE_SQLITE
	database = this->get_database(running_node);

	if ( database == NULL )
		return jobs_list;

	query += "get_ready_job;";
	jobs_matrix = this->get_database(running_node)->query_full_result(&query);
#endif

	if ( jobs_matrix == NULL )
		return jobs_list;

	BOOST_FOREACH(v_row job_row, *jobs_matrix) {
		delete job;

		job = new rpc::t_job();

		job->id			= boost::lexical_cast<int>(job_row[0]);
		job->name		= job_row[1];
		job->node_name	= job_row[3];
		job->cmd_line	= job_row[2];
		job->weight		= boost::lexical_cast<int>(job_row[4]);

		jobs_list.push_back(*job);
	}

	delete job;
	jobs_matrix->clear();
	delete jobs_matrix;

	return jobs_list;
}

///////////////////////////////////////////////////////////////////////////////
/*
v_jobs	Domain::get_jobs(const char* running_node) {
		std::string			query("SELECT job_id,job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM ");
		v_v_row*			jobs_matrix;
		v_jobs				jobs_list;
		Job*				job	= NULL;

		std::string			buf_name;
		std::string			buf_node_name;
		std::string			buf_cmd_line;
#ifdef USE_SQLITE
		Sqlite*				database;
#endif

#ifdef USE_MYSQL
		query += running_node;
		query += ".job;";

		jobs_matrix = this->database.query_full_result(&query);
#endif
#ifdef USE_SQLITE
		database = this->get_database(running_node);

		if ( database == NULL )
			return jobs_list;

		query += "get_ready_job;";
		jobs_matrix = this->get_database(running_node)->query_full_result(&query);
#endif

		if ( jobs_matrix == NULL )
			return jobs_list;

		BOOST_FOREACH(v_row job_row, *jobs_matrix) {
			delete job;

			buf_name		= job_row[1];
			buf_node_name	= job_row[3];
			buf_cmd_line	= job_row[2];

			job = new Job((Domain*)this, boost::lexical_cast<int>(job_row[0]), *this->name, buf_node_name, buf_cmd_line, (const int)boost::lexical_cast<int>(job_row[4]));
			jobs_list.push_back(*job);
		}

		delete job;
		delete [] jobs_matrix;

		return jobs_list;
	}
}
*/
///////////////////////////////////////////////////////////////////////////////

rpc::v_jobs	Domain::get_jobs(const char* running_node) {
	std::string			query("SELECT job_id,job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM ");
	v_v_row*			jobs_matrix;
	rpc::v_jobs			jobs_list;
	rpc::t_job*			job	= NULL;

	std::string			buf_name;
	std::string			buf_node_name;
	std::string			buf_cmd_line;
#ifdef USE_SQLITE
	Sqlite*				database;
#endif

#ifdef USE_MYSQL
	query += running_node;
	query += ".job;";

	jobs_matrix = this->database.query_full_result(query.c_str());
#endif
#ifdef USE_SQLITE
	database = this->get_database(running_node);

	if ( database == NULL )
		return jobs_list;

	query += "get_ready_job;";
	jobs_matrix = this->get_database(running_node)->query_full_result(&query);
#endif

	if ( jobs_matrix == NULL )
		return jobs_list;

	BOOST_FOREACH(v_row job_row, *jobs_matrix) {
		delete job;

		job = new rpc::t_job();

		job->id			= boost::lexical_cast<int>(job_row[0]);
		job->name		= job_row[1];
		job->node_name	= job_row[3];
		job->cmd_line	= job_row[2];
		job->weight		= boost::lexical_cast<int>(job_row[4]);

		jobs_list.push_back(*job);
	}

	delete job;
	jobs_matrix->clear();
	delete jobs_matrix;

	return jobs_list;
}

///////////////////////////////////////////////////////////////////////////////

void			Domain::sql_exec(const std::string& running_node, const std::string& s) {
	v_v_row*	result = NULL;
#ifdef USE_MYSQL
	result = this->database.query_full_result(s.c_str());
#endif
#ifdef USE_SQLITE
	result = this->get_database(running_node)->execute(s);
#endif

	delete result;
}

///////////////////////////////////////////////////////////////////////////////

void			Domain::sql_exec(const std::string& s) {
#ifdef USE_MYSQL
	this->database.query_full_result(s.c_str());
#endif
}

///////////////////////////////////////////////////////////////////////////////

std::string*	Domain::get_name() const {
	return this->name;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_SQLITE
Sqlite*			Domain::get_database(const char* node_name) {
	m_nodes::iterator	it = this->databases.find(node_name);

	if ( it == this->databases.end() )
		return NULL;

	return &it->second;
}
#endif

///////////////////////////////////////////////////////////////////////////////

