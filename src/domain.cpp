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

// TODO: use day_{start_date,start_time,duration} to fill in planning_{start_time,duration}
Domain::Domain(Config* c) {
	boost::regex	duration_matching("^(([[:digit:]]+)h)?(([[:digit:]]+)min)?$", boost::regex::perl);
	boost::regex	time_matching("^([[:digit:]]{2}):([[:digit:]]{2})$", boost::regex::perl);

	boost::smatch			what;
	//boost::match_flag_type		flags = boost::match_default;
	std::string::const_iterator	start = c->get_param("day_duration")->begin();
	std::string::const_iterator	end = c->get_param("day_duration")->end();

	if ( c == NULL ) {
		rpc::ex_processing e;
		e.msg = "The config is null";
		throw e;
	}

	this->config	= c;
	this->name	= this->config->get_param("domain_name")->c_str();

	/*
	 * Make sure the duration and the start time are zero before calculating them
	 */
	this->planning_duration = 0;
	this->initial_planning_start_time = 0;
	
	try {
		if ( boost::regex_search(start, end, what, duration_matching) ) {
			/*
			 * what[0] empty -> problem
			 * what[2] hours -> convert to seconds (*3600)
			 * what[4] minutes -> convert to seconds (*60)
			 */
			if ( what[0].length() == 0 ) {
				rpc::ex_processing e;
				e.msg = "Error: duration is empty";
				throw e;
			}

			if ( what[2].length() != 0 )
				this->planning_duration += boost::lexical_cast<int>(what[1]) * 3600;

			if ( what[4].length() != 0 )
				this->planning_duration += boost::lexical_cast<int>(what[4]) * 60;
		} else {
			rpc::ex_processing	e;
			e.msg = "Error: cannot get the planning's duration";
			throw e;
		}

		std::cout << "Planning duration is: " << this->planning_duration << std::endl;

		start = c->get_param("day_start_time")->begin();
		end = c->get_param("day_start_time")->end();

		if ( boost::regex_search(start, end, what, time_matching) ) {
			this->initial_planning_start_time = boost::lexical_cast<int>(what[1]) * 3600 + boost::lexical_cast<int>(what[2]) * 60;
			std::cout << "Initial planning start time is: " << boost::lexical_cast<int>(what[1]) << ":" << boost::lexical_cast<int>(what[2]) << " -> " << this->initial_planning_start_time << std::endl;
		}
	} catch (const std::exception& e) {
		std::cerr << e.what();
	}

	this->planning_start_time = this->get_next_planning_start_time();
	std::cout << "Next start time is: " << this->planning_start_time << std::endl;

	if ( this->database.prepare(this->config->get_param("domain_name"), this->config->get_param("db_skeleton")) == false ) {
		rpc::ex_processing e;
		e.msg = "Error: cannot prepare the database";
		throw e;
	}
}

Domain::~Domain() {
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::get_planning(rpc::t_planning& _return, const char* domain_name, const char* node_name) {
	rpc::t_node	node;

	this->get_node(domain_name, node, node_name);

	_return.day.begin_time = this->get_next_planning_start_time();
	_return.day.duration = this->planning_duration;
	_return.nodes.push_back(node);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::set_next_planning() {
	rpc::v_nodes	nodes;
	std::string		planning_name;

	this->get_nodes(this->name.c_str(), nodes);

	planning_name = this->name.c_str();
	planning_name += "_";
	planning_name += boost::lexical_cast<std::string>(this->get_next_planning_start_time());

	this->database.init_domain_structure(planning_name, this->config->get_param("db_skeleton")->c_str());

	BOOST_FOREACH(rpc::t_node node, nodes) {
		if ( this->add_node(planning_name.c_str(), node) == false )
			return false;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

time_t	Domain::get_next_planning_start_time() {
	time_t now = time(NULL);

	return now + this->planning_duration - ( now - this->initial_planning_start_time) % this->planning_duration;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* domain_name, const rpc::t_node& n) {
	std::string	query;
	v_queries	queries;

	if ( this->add_node(domain_name, n.name, n.weight) == false )
		return false;

	// TODO: find a way to lock the whole transaction
	// TODO: use a single transaction

	if ( n.resources.empty() == false ) {
		BOOST_FOREACH(rpc::t_resource r, n.resources) {
			if ( this->add_resource(domain_name, r, n.name.c_str()) == false )
				return false;
		}
	}

	if ( n.jobs.empty() == false ) {
		BOOST_FOREACH(rpc::t_job j, n.jobs) {
			if ( this->add_job(domain_name, j) == false )
				return false;
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* domain_name, const char* n) {
	std::string query;
	v_queries	queries;

	this->updates_mutex.lock();

#ifdef USE_MYSQL
	query = "INSERT IGNORE INTO ";
#endif
	query += "node (node_name) VALUES ('";
	query += n;
	query += "');";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* domain_name, const std::string& n, const rpc::integer& w) {
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
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_job(const char* domain_name, const rpc::t_job& j) {
	std::string		query;
	const char*		running_node = j.node_name.c_str();
	v_queries		queries;

//	this->add_node(this->name.c_str(), running_node); // TODO: think of removing it

	this->updates_mutex.lock();

	query = "INSERT INTO job (job_name,job_cmd_line,job_node_name,job_weight) VALUES ('";
	query += j.name;
	query += "','";
	query += j.cmd_line;
	query += "','";
	query += j.node_name;
	query += "','";
	query += boost::lexical_cast<std::string>(j.weight);
	query += "');";

	queries.push_back(query);

	BOOST_FOREACH(std::string i, j.prv) {
		query = "INSERT INTO ";
		query += "jobs_link (job_name_prv,job_name_nxt) VALUES ('";
		query += i;
		query += "','";
		query += j.name;
		query += "');";

		queries.push_back(query);
	}

	BOOST_FOREACH(std::string i, j.nxt) {
		query = "INSERT INTO jobs_link (job_name_nxt,job_name_prv) VALUES ('";
		query += i;
		query += "','";
		query += boost::lexical_cast<std::string>(j.name);
		query += "');";

		queries.push_back(query);
	}

	BOOST_FOREACH(rpc::t_time_constraint tc, j.time_constraints) {
		query = "INSERT INTO time_constraint (time_c_job_name, time_c_type, time_c_value) VALUES ('";
		query += j.name;
		query += "','";
		query += build_string_from_time_constraint_type(tc.type);
		query += "',SEC_TO_TIME(";
		query += boost::lexical_cast<std::string>(tc.value);
		query += "));";

		queries.push_back(query);
	}

	// TODO: add recovery types
//	this->get_add_recovery_type_query(query, j.recovery_type);
//	queries.push_back(query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job(const char* domain_name, const rpc::t_job& j) {
	std::string		query;
	v_queries		queries;

	this->updates_mutex.lock();

	query = "REPLACE INTO job (job_name,job_cmd_line,job_node_name,job_weight) VALUES ('";
	query += j.name;
	query += "','";
	query += j.cmd_line;
	query += "','";
	query += j.node_name;
	query += "','";
	query += boost::lexical_cast<std::string>(j.weight);
	query += "');";

	queries.insert(queries.end(), query);

	query = "DELETE FROM jobs_link WHERE job_name_nxt ='";
	query += j.name;
	query += "');";

	queries.insert(queries.end(), query);

	BOOST_FOREACH(std::string i, j.prv) {
		query = "REPLACE INTO";
		query += "jobs_link (job_name_prv,job_name_nxt) VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += j.name;
		query += "');";

		queries.insert(queries.end(), query);
	}

	query = "DELETE FROM jobs_link WHERE job_name_prv ='";
	query += j.name;
	query += "');";

	queries.insert(queries.end(), query);

	BOOST_FOREACH(std::string i, j.nxt) {
		query = "REPLACE INTO jobs_link (job_name_nxt,job_name_prv) VALUES ('";
		query += boost::lexical_cast<std::string>(i);
		query += "','";
		query += j.name;
		query += "');";

		queries.insert(queries.end(), query);
	}

	query = "DELETE FROM time_constraint WHERE time_c_job_name = '";
	query += j.name;
	query += "');";

	queries.insert(queries.end(), query);

	BOOST_FOREACH(rpc::t_time_constraint tc, j.time_constraints) {
		query = "REPLACE INTO time_constraint (time_c_job_name, time_c_type, time_c_value) VALUES ('";
		query += j.name;
		query += "','";
		query += build_string_from_time_constraint_type(tc.type);
		query += "','";
		query += boost::lexical_cast<std::string>(tc.value);
		query += "');";

		queries.insert(queries.end(), query);
	}

//	this->get_add_recovery_type_query(query, j.recovery_type);
	queries.push_back(query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, j.node_name.c_str()) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const char* domain_name, const rpc::t_job& j) {
	return this->remove_job(domain_name, j.node_name, j.name);
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const char* domain_name, const Job* j) {
	return this->remove_job(domain_name, j->get_node_name(), j->get_name());
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const char*domain_name, const std::string& running_node, const std::string& j_name) {
	std::string	query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "DELETE FROM job WHERE job_name = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

	query = "DELETE FROM jobs_link WHERE job_next = '";
	query += j_name;
	query += "' OR job_prev = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

	query = "DELETE FROM time_constraint WHERE time_c_job_name = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const rpc::t_job& j) {
	return this->update_job_state(domain_name, j.node_name, j.name, j.state);
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const Job* j, const rpc::e_job_state::type js) {
	if ( j == NULL ) {
		std::cerr << "Error: j is NULL" << std::endl;
		return false;
	}

	return this->update_job_state(domain_name, j->get_node_name2(), j->get_name(), js);
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js) {
	std::string		query;
	v_queries		queries;
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	if ( running_node.empty() == true or boost::regex_match(query, empty_string) == true ) {
		std::cerr << "Error: running_node is empty" << std::endl;
		return false;
	}

	this->updates_mutex.lock();

	query = "UPDATE job SET job_state = '";
	query += build_string_from_job_state(js);
	query += "' WHERE job_name = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time) {
	std::string		query;
	v_queries		queries;
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	if ( running_node.empty() == true or boost::regex_match(query, empty_string) == true ) {
		std::cerr << "Error: running_node is empty" << std::endl;
		return false;
	}

	this->updates_mutex.lock();

	query = "UPDATE job SET job_state = '";
	query += build_string_from_job_state(js);
	query += "', job_start_time = '";
	query += boost::lexical_cast<std::string>(start_time);
	query += "', job_stop_time = '";
	query += boost::lexical_cast<std::string>(stop_time);
	query += "' WHERE job_name = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const Job* j, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time) {
	return this->update_job_state(domain_name, j->get_node_name(), j->get_name(), js, start_time, stop_time);
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_ready_jobs(v_jobs& _return, const char* running_node) {
	std::string	query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM get_ready_job");
	v_row		job_rectype;
	v_v_row		jobs_matrix;
	Job*		job	= NULL;
	rpc::t_job*	rpc_j	= NULL;

	if ( running_node == NULL )
		query += ";";
	else {
		query += " WHERE job_node_name = '";
		query += running_node;
		query += "';";
	}

#ifdef USE_MYSQL
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), this->get_current_name().c_str()) == false ) {
		rpc::ex_job e;
		e.msg = "The database query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row job_row, jobs_matrix) {
		delete job;
		delete rpc_j;
		rpc_j = new rpc::t_job();

		rpc_j->name		= job_row[0];
		rpc_j->cmd_line	= job_row[1];
		rpc_j->node_name	= job_row[2];
		rpc_j->weight	= boost::lexical_cast<int>(job_row[3]);
		rpc_j->state		= build_job_state_from_string(job_row[4].c_str());
/*
		query = "SELECT job_rectype_id FROM job WHERE job_name ='";
		query += rpc_j->name;
		query += "';";

#ifdef USE_MYSQL
		if ( this->database.query_one_row(job_rectype, query.c_str(), this->name.c_str()) == false ) {
			rpc::ex_job e;
			e.msg = "The database query failed";
			throw e;
		}
#endif
		this->get_recovery_type(rpc_j->recovery_type, running_node, boost::lexical_cast<int>(job_row[5]));
*/
		job = new Job((Domain*)this, *rpc_j);
		_return.push_back(*job);
	}

	delete job;
	delete rpc_j;
	jobs_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_ready_jobs(rpc::v_jobs& _return, const char* running_node) {
	std::string		query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM get_ready_job");
	v_v_row			jobs_matrix;
	rpc::t_job*		job	= NULL;

	std::string		buf_name;
	std::string		buf_node_name;
	std::string		buf_cmd_line;

	if ( running_node == NULL )
		query += ";";
	else {
		query += " WHERE job_node_name = '";
		query += running_node;
		query += "';";
	}

#ifdef USE_MYSQL
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), this->get_current_name().c_str()) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif

	if ( jobs_matrix.empty() == true ) {
		rpc::ex_job	e;
		e.msg = "The database returned an empty result";
		throw e;
	}

	BOOST_FOREACH(v_row job_row, jobs_matrix) {
		delete job;

		job = new rpc::t_job();

		job->domain		= this->name.c_str();
		job->name		= job_row[0];
		job->node_name	= job_row[2];
		job->cmd_line	= job_row[1];
		job->weight		= boost::lexical_cast<int>(job_row[3]);

		_return.push_back(*job);
	}

	delete job;
	jobs_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_jobs(const char* domain_name, rpc::v_jobs& _return, const char* running_node) {
	std::string			query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM job");
	v_v_row				jobs_matrix;
	rpc::t_job*			job	= NULL;

	if ( running_node == NULL )
		query += ";";
	else {
		query += " WHERE job_node_name = '";
		query += running_node;
		query += "';";
	}

#ifdef USE_MYSQL
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), domain_name) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row job_row, jobs_matrix) {
		delete job;
		job = new rpc::t_job();

		job->domain		= this->name.c_str();
		job->name		= job_row[0];
		job->node_name	= job_row[2];
		job->cmd_line	= job_row[1];
		job->weight		= boost::lexical_cast<int>(job_row[3]);
		job->state		= build_job_state_from_string(job_row[4].c_str());

		this->get_jobs_next(domain_name, job->nxt, running_node, job->name);
		this->get_time_constraints(domain_name, job->time_constraints, running_node, job->name);

		_return.push_back(*job);
	}

	delete job;
	jobs_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_job(const char* domain_name, rpc::t_job& _return, const char* running_node, const char* job_name) {
	std::string	query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM job");
	v_row		job_row;

	query += " WHERE job_name = '";
	query += job_name;
	query += "'";

	if ( running_node == NULL ) {
		query += ";";
	} else {
		query += " AND job_node_name = '";
		query += running_node;
		query += "';";
	}

	if ( this->database.query_one_row(job_row, query.c_str(), domain_name) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}

	_return.domain		= this->name.c_str();
	_return.name		= job_row[0];
	_return.node_name	= job_row[2];
	_return.cmd_line	= job_row[1];
	_return.weight		= boost::lexical_cast<int>(job_row[3]);
	_return.state		= build_job_state_from_string(job_row[4].c_str());

	this->get_jobs_next(domain_name, _return.nxt, running_node, _return.name);
	this->get_time_constraints(domain_name, _return.time_constraints, running_node, _return.name);
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_jobs_next(const char* domain_name, rpc::v_job_names& _return, const char* running_node, const std::string& j_name) {
	std::string	query("SELECT job_name_nxt FROM jobs_link WHERE job_name_prv = '");
	v_v_row	job_links_matrix;

	query += j_name;
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_full_result(job_links_matrix, query.c_str(), domain_name) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row job_next_row, job_links_matrix) {
		_return.push_back(job_next_row[0]);
	}

	job_links_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_macro_jobs(const char* domain_name, rpc::v_macro_jobs& _return, const char* running_node) {
	std::string			query("SELECT macro_id,macro_name FROM macro_job;");
	v_v_row				macro_jobs_matrix;
	rpc::t_macro_job*	macro_job = NULL;

#ifdef USE_MYSQL
	if ( this->database.query_full_result(macro_jobs_matrix, query.c_str(), domain_name) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row macro_job_row, macro_jobs_matrix) {
		delete macro_job;

		macro_job->id = boost::lexical_cast<int>(macro_job_row[0]);
		macro_job->name = macro_job_row[1];

		_return.push_back(*macro_job);
	}

	delete macro_job;
	macro_jobs_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_resource(const char* domain_name, const rpc::t_resource& r, const char* node_name) {
	std::string	query("INSERT INTO resource (resource_name,resource_node_name,resource_current_value,resource_initial_value) VALUES ('");
	v_queries	queries;

	query += r.name;
	query += "','";
	query += node_name;
	query += "','";
	query += boost::lexical_cast<std::string>(r.current_value);
	query += "','";
	query += boost::lexical_cast<std::string>(r.initial_value);
	query += "');";

	queries.push_back(query);

	this->updates_mutex.lock();

#ifdef USE_MYSQL
	if ( this->database.standalone_execute(queries, domain_name) == true ) {
		this->updates_mutex.unlock();
		return true;
	}
#endif
	this->updates_mutex.unlock();
	return false;
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_resources(const char* domain_name, rpc::v_resources& _return, const char* running_node) {
	std::string			query("SELECT resource_name,resource_node_name,resource_current_value,resource_initial_value FROM resource;");
	v_v_row			resources_matrix;
	rpc::t_resource*	resource = NULL;

#ifdef USE_MYSQL
	 if ( this->database.query_full_result(resources_matrix, query.c_str(), domain_name) == false )   {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row resource_row, resources_matrix) {
		delete resource;
		resource = new rpc::t_resource();

		resource->name		= resource_row[1];
//		resource->node_name	= resource_row[2];
		resource->current_value		= boost::lexical_cast<int>(resource_row[2]);
		resource->initial_value		= boost::lexical_cast<int>(resource_row[3]);

		_return.push_back(*resource);
	}

	delete resource;
	resources_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_time_constraints(const char* domain_name, rpc::v_time_constraints& _return, const char* running_node, const std::string& job_name) {
	std::string			query("SELECT time_c_job_name,time_c_type,TIME_TO_SEC(time_c_value) FROM time_constraint WHERE time_c_job_name = '");
	v_v_row				time_constraints_matrix;
	rpc::t_time_constraint*	time_constraint = NULL;

	query += job_name;
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_full_result(time_constraints_matrix, query.c_str(), domain_name) == false )  {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row time_constraint_row, time_constraints_matrix) {
		delete time_constraint;
		time_constraint = new rpc::t_time_constraint();

		std::cout << "job_name -> " << time_constraint_row[0].c_str() << std::endl;
		time_constraint->job_name	= time_constraint_row[0].c_str();
		std::cout << "type -> " << build_time_constraint_type_from_string(time_constraint_row[1].c_str()) << std::endl;
		time_constraint->type		= build_time_constraint_type_from_string(time_constraint_row[1].c_str());
		std::cout << "value -> " << time_constraint_row[2] << std::endl;
		time_constraint->value		= boost::lexical_cast<rpc::integer>(time_constraint_row[2]);

		_return.push_back(*time_constraint);
	}

	delete time_constraint;
	time_constraints_matrix.clear(); // TODO: is it really useful ?
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_recovery_types(const char* domain_name, rpc::v_recovery_types& _return, const char* running_node) {
	std::string	query("SELECT rectype_id,rectype_short_label,rectype_label,rectype_action FROM recovery_type;");
	v_v_row				recoveries_matrix;
	rpc::t_recovery_type*	recovery = NULL;

#ifdef USE_MYSQL
	if ( this->database.query_full_result(recoveries_matrix, query.c_str(), domain_name) == false ) {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row recovery_row, recoveries_matrix) {
		delete recovery;

		recovery->id			= boost::lexical_cast<int>(recovery_row[0]);
		recovery->short_label	= recovery_row[1];
		recovery->label			= recovery_row[2];
		recovery->action		= build_rectype_action_from_string(recovery_row[3].c_str());

		_return.push_back(*recovery);
	}

	delete recovery;
	recoveries_matrix.clear(); // TODO: is it really useful ?
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_recovery_type(const char* domain_name, rpc::t_recovery_type& _return, const char* running_node, const int rec_id) {
	std::string	query("SELECT rectype_id,rectype_short_label,rectype_label,rectype_action FROM recovery_type WHERE rectype_id = '");
	v_row				recovery_row;

	query += rec_id;
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_one_row(recovery_row, query.c_str(), domain_name) == false ) {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	_return.id			= boost::lexical_cast<int>(recovery_row[0]);
	_return.short_label	= recovery_row[1];
	_return.label		= recovery_row[2];
	_return.action		= build_rectype_action_from_string(recovery_row[3].c_str());
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_node(const char* domain_name, rpc::t_node& _return, const char* node_name) {
	std::string	query("SELECT node_name,node_weight FROM node WHERE node_name = '");
	v_row				node_row;

	query += node_name;
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_one_row(node_row, query.c_str(), domain_name) == false ) {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	if ( node_row.size() > 0 ) {
		_return.name	= node_row[0].c_str();
		_return.weight	= boost::lexical_cast<rpc::integer>(node_row[1]);
		_return.domain_name	= this->name.c_str();
		this->get_resources(domain_name, _return.resources, _return.name.c_str());
		this->get_jobs(domain_name, _return.jobs, _return.name.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_nodes(const char* domain_name, rpc::v_nodes& _return) {
	std::string	query("SELECT node_name,node_weight FROM node;");
	v_v_row			nodes_matrix;
	rpc::t_node*	node = NULL;

#ifdef USE_MYSQL
	if ( this->database.query_full_result(nodes_matrix, query.c_str(), domain_name) == false ) {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	BOOST_FOREACH(v_row node_row, nodes_matrix) {
		delete node;
		node = new rpc::t_node();

		node->name		= node_row[0];
		node->weight	= boost::lexical_cast<rpc::integer>(node_row[1]);

		this->get_node(domain_name, *node, node_row[0].c_str());

		_return.push_back(*node);
	}

	delete node;
	nodes_matrix.clear(); // TODO: is it really useful ?
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::sql_exec(const std::string& running_node, const std::string& s) {
	v_v_row	result;
#ifdef USE_MYSQL
	this->database.query_full_result(result, s.c_str(), running_node.c_str());
#endif
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::sql_exec(const std::string& s) {
	v_v_row	result;
#ifdef USE_MYSQL
	this->database.query_full_result(result, s.c_str(), NULL);
#endif
}

///////////////////////////////////////////////////////////////////////////////

const char*	Domain::get_name() const {
	return this->name.c_str();
}

///////////////////////////////////////////////////////////////////////////////

std::string	Domain::get_current_name() {
	std::string result;

	result = get_name();
	result += "_";
	result += boost::lexical_cast<std::string>(this->planning_start_time);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_add_node_query(std::string& _return, const rpc::t_node& node) {
	_return = "REPLACE INTO node (node_name,node_weight) VALUES ('";
	_return += node.name.c_str();
	_return += "','";
	_return += boost::lexical_cast<std::string>(node.weight);
	_return += "');";
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_add_recovery_type_query(std::string& _return, const rpc::t_recovery_type& rc_type) {
	_return = "REPLACE INTO recovery_type (rectype_id, rectype_short_label, rectype_label, rectype_action) VALUES ('";
	_return += boost::lexical_cast<std::string>(rc_type.id);
	_return += "','";
	_return += rc_type.short_label;
	_return += "','";
	_return += rc_type.label;
	_return += "','";
	_return += build_string_from_rectype_action(rc_type.action);
	_return += "');";
}

///////////////////////////////////////////////////////////////////////////////
