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
	if ( c == NULL ) {
		rpc::ex_processing e;
		e.msg = "The config is null";
		throw e;
	}

	this->config	= c;

	boost::regex	duration_hour_matching("^([[:digit:]]+)h", boost::regex::perl);
	boost::regex	duration_minute_matching("([[:digit:]]+)min$", boost::regex::perl);
	boost::regex	time_matching("^([[:digit:]]{2}):([[:digit:]]{2})$", boost::regex::perl);

	boost::smatch			what;
	boost::smatch			what_hour;
	boost::smatch			what_minute;
	//boost::match_flag_type		flags = boost::match_default;
	std::string::const_iterator	start = this->config->get_param("day_duration")->begin();
	std::string::const_iterator	end = this->config->get_param("day_duration")->end();


	this->name	= this->config->get_param("domain_name")->c_str();

	if ( this->name.size() == 0 ) {
		rpc::ex_processing e;
		e.msg = "The domain's name is empty";
		throw e;
	}

	/*
	 * Make sure the duration and the start time are zero before calculating them
	 */
	this->planning_duration = 0;
	this->initial_planning_start_time = 0;

	/*
	 * Let's calculate the duration and the start time
	 */
	if ( boost::regex_search(start, end, what_hour, duration_hour_matching) ) {
		try {
			this->planning_duration += boost::lexical_cast<int>(what_hour[1]) * 3600;
		} catch (const std::exception& l) {
			rpc::ex_processing e;
			e.msg = "Error: cannot set planning duration. Cannot cast ";
			e.msg += what_hour[1];
			throw e;
		}
	}

	if ( boost::regex_search(start, end, what_minute, duration_minute_matching) ) {
		try {
			this->planning_duration += boost::lexical_cast<int>(what_minute[1]) * 60;
		} catch (const std::exception& l) {
			rpc::ex_processing e;
			e.msg = "Error: cannot set planning duration. Cannot cast ";
			e.msg += what_minute[1];
			throw e;
		}
	}

	if ( this->planning_duration == 0 ) {
		rpc::ex_processing e;
		e.msg = "Error: planning duration is 0";
		throw e;
	}

	INFO << "Planning duration is " << this->planning_duration << " seconds";

	start = this->config->get_param("day_start_time")->begin();
	end = this->config->get_param("day_start_time")->end();

	if ( boost::regex_search(start, end, what, time_matching) ) {
		this->initial_planning_start_time = boost::lexical_cast<int>(what[1]) * 3600 + boost::lexical_cast<int>(what[2]) * 60;
		INFO << "Initial planning start time is " << boost::lexical_cast<int>(what[1]) << ":" << boost::lexical_cast<int>(what[2]) << " (" << this->initial_planning_start_time << ")";
	}

	/*
	 * Let's prepare the template database
	 */
	if ( this->database.prepare() == false ) {
		rpc::ex_processing e;
		e.msg = "Error: cannot prepare the database";
		throw e;
	}

	/*
	 * Let's prepare and populate the template planning
	 */
	if ( this->database.init_domain_structure(this->name, this->config->get_param("db_skeleton")->c_str()) == false ) {
		rpc::ex_processing e;
		e.msg = "Error: cannot prepare the template planning";
		throw e;
	}

	/*
	 * Let's prepare and populate the next planning to start
	 */
	try {
		if ( this->set_next_planning(this->planning_start_time) == false ) {
			rpc::ex_processing e;
			e.msg = "Error: cannot prepare the next planning";
			ALERT << e.msg;
			throw e;
		}
	} catch (rpc::ex_processing& e) {
		ALERT << e.msg;
	}

	INFO << "First start time is " << this->planning_start_time << " ("  << build_human_readable_time(this->planning_start_time) << ")";
}

Domain::~Domain() {
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::get_planning(rpc::t_planning& _return, const char* domain_name, const char* node_name) {
	rpc::t_node	node;

	this->get_node(domain_name, node, node_name);

	_return.day.begin_time = this->planning_start_time;
	_return.day.duration = this->planning_duration;
	_return.nodes.push_back(node);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::set_next_planning(time_t& _return) {
	rpc::v_nodes	nodes;
	std::string		next_planning_name;

	_return = this->get_next_planning_start_time();

	next_planning_name = this->name;
	next_planning_name += "_";
	next_planning_name += boost::lexical_cast<std::string>(_return);

	DEBUG << "next planning is " << next_planning_name << ", starting at " << _return << " (" << build_human_readable_time(_return) << ")";

	try {
		v_row result;
		std::string	query;

		query ="SELECT IF('" + next_planning_name + "' IN(SELECT SCHEMA_NAME FROM INFORMATION_SCHEMA.SCHEMATA), 1, 0) AS found;";

		if ( this->database.query_one_row(result, query.c_str(), NULL) == false ) {
			ERROR << "cannot query the DB to find if the next planning alread exists";
			return false;
		}

		if ( result.at(0).compare("1") == 0 ) {
			INFO << "The schema already exists, skipping domain init...";
			return true;
		}

		if ( this->database.init_domain_structure(next_planning_name.c_str(), *this->config->get_param("db_skeleton")) == false ) {
			return true;
		}
	} catch ( const rpc::ex_processing& e) {
		ERROR << e.msg;
		return false;
	}

	this->get_nodes(this->name.c_str(), nodes);

	// We could use a method called "add_nodes" as well
	BOOST_FOREACH(rpc::t_node node, nodes) {
		if ( this->add_node(next_planning_name.c_str(), node) == false ) {
			ERROR << "failed to add node " << node.name << " into planning " << next_planning_name;
			return false;
		}
	}

	INFO << "added " << nodes.size() << " nodes to " << next_planning_name;
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::switch_planning() {
	time_t	result;
	INFO << "old planning start time is " << this->planning_start_time << " (" << build_human_readable_time(this->planning_start_time) << ")";

	if ( this->set_next_planning(result) == false )
		return false;

	this->planning_start_time = result;

	INFO << "new planning start time is " << this->planning_start_time << " (" << build_human_readable_time(this->planning_start_time) << ")";

	return true;
}

///////////////////////////////////////////////////////////////////////////////

std::string	Domain::dump_planning(const char* planning_name) {
	std::string	query;
	v_queries	queries;
	std::string	result;

	if ( planning_name == NULL ) {
		rpc::ex_processing e;
		e.msg = "planning_name is NULL";
		throw e;
	}

	// TODO: update it
/*
	result = this->dump_planning(planning_name);

	if ( result.empty() == true ) {
		rpc::ex_processing e;
		e.msg = "Cannot dump the database";
		throw e;
	}

	query = "DROP DATABASE ";
	query += planning_name;

	queries.push_back(query);

	if ( this->database.standalone_execute(queries, NULL) == false ) {
		rpc::ex_processing e;
		e.msg = "Query failed";
		throw e;
	}
*/
	return result;
}

///////////////////////////////////////////////////////////////////////////////

time_t	Domain::get_next_planning_start_time() {
	time_t	now = time(NULL);
	time_t	result;

	result = now + this->planning_duration - ( now - this->initial_planning_start_time) % this->planning_duration;
	DEBUG << "next planning start time is " << result << " (" << build_human_readable_time(result) << ")";

	return result;
}

///////////////////////////////////////////////////////////////////////////////

time_t	Domain::get_planning_start_time() {
	return this->planning_start_time;
}

///////////////////////////////////////////////////////////////////////////////

time_t	Domain::get_current_planning_remaining_time() {
	time_t	now = time(NULL);

	return this->planning_start_time + this->planning_duration - now;
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::add_node(const char* domain_name, const rpc::t_node& n) {
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

	query = "INSERT IGNORE INTO node (node_name,node_weight) VALUES ('";
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

bool	Domain::remove_node(const char* domain_name, const std::string& n) {
	rpc::t_node	node_to_remove;
	std::string	query = "DELETE FROM node WHERE node_name='";
	v_queries	queries;

	this->get_node(domain_name, node_to_remove, n.c_str());

	// Remove the jobs
	BOOST_FOREACH(rpc::t_job j, node_to_remove.jobs) {
		if ( this->remove_job(j) == false )
			return false;
	}

	// TODO: Remove the resources

	// Remove the node
	query += n;
	query += "';";

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

bool	Domain::add_job(const char* domain_name, const rpc::t_job& j) {
	std::string		query;
	//const char*		running_node = j.node_name.c_str();
	v_queries		queries;
	v_row			row;

	this->updates_mutex.lock();

	query = "SELECT COUNT(*) FROM node WHERE node_name = '";
	query += j.node_name;
	query += "';";

	if ( this->database.query_one_row(row, query.c_str(), j.domain.c_str()) == false ) {
		rpc::ex_processing e;
		e.msg = "Cannot query the node table";

		throw e;
	}

	if ( boost::lexical_cast<int>(row.at(0)) == 0 ) {
		rpc::ex_node e;
		e.msg = "The node ";
		e.msg += j.node_name;
		e.msg += " does not exist";
		throw e;
	}

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
		if ( this->planning_duration < tc.value ) {
			rpc::ex_processing e;
			e.msg = "the given value of the time_constraint is higher than the planning's duration (";
			e.msg += tc.value;
			e.msg += " > ";
			e.msg += this->planning_duration;
			e.msg += ")";

			throw e;
		}

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

bool	Domain::update_job(const rpc::t_job& j) {
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

bool	Domain::remove_job(const rpc::t_job& j) {
	return this->remove_job(j.domain.c_str(), j.name);
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::remove_job(const char* domain_name, const std::string& j_name) {
	std::string	query;
	v_queries	queries;

	this->updates_mutex.lock();

	query = "DELETE FROM job WHERE job_name = '";
	query += j_name;
	query += "';";

	queries.insert(queries.end(), query);

	query = "DELETE FROM jobs_link WHERE job_name_nxt = '";
	query += j_name;
	query += "' OR job_name_prv = '";
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
		ERROR << "Error: j is NULL";
		return false;
	}

	return this->update_job_state(domain_name, j->get_node_name2(), j->get_name(), js);
}

///////////////////////////////////////////////////////////////////////////////

bool	Domain::update_job_state(const char* domain_name, const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js) {
	std::string	query;
	v_queries	queries;
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	if ( running_node.empty() == true or boost::regex_match(query, empty_string) == true ) {
		ERROR << "Error: running_node is empty";
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
	std::string	query;
	v_queries	queries;
	boost::regex	empty_string("^\\s+$", boost::regex::perl);

	if ( running_node.empty() == true or boost::regex_match(query, empty_string) == true ) {
		ERROR << "Error: running_node is empty";
		return false;
	}

	this->updates_mutex.lock();

	query = "UPDATE job SET job_state = '";
	query += build_string_from_job_state(js);
	query += "', job_start_time = FROM_UNIXTIME(";
	query += boost::lexical_cast<std::string>(start_time);
	query += "), job_stop_time = FROM_UNIXTIME(";
	query += boost::lexical_cast<std::string>(stop_time);
	query += ") WHERE job_name = '";
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
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), this->get_current_planning_name().c_str()) == false ) {
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
		try {
			rpc_j->weight	= boost::lexical_cast<int>(job_row[3]);
		} catch (const std::exception& lc_e) {
			rpc::ex_processing e;
			e.msg = "cannot cast value ";
			e.msg += job_row[3];
			e.msg += " to integer. Query is ";
			e.msg += query;
			e.msg += ". Exception is ";
			e.msg += lc_e.what();
			ERROR << e.msg;
			throw e;
		}

		rpc_j->state		= build_job_state_from_string(job_row[4].c_str());

		rpc_j->domain	= this->get_current_planning_name();
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
	std::string	query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id FROM get_ready_job");
	v_v_row		jobs_matrix;
	rpc::t_job*	job	= NULL;

	// We do not query the database if the planning is not started yet
	if ( this->planning_start_time < time(NULL) )
		return;

	if ( running_node != NULL ) {
		query += " WHERE job_node_name = '";
		query += running_node;
	}
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), this->get_current_planning_name().c_str()) == false ) {
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

		job->domain		= this->get_current_planning_name();
		job->name		= job_row[0];
		job->node_name	= job_row[2];
		job->cmd_line	= job_row[1];
		try {
			job->weight		= boost::lexical_cast<int>(job_row[3]);
		} catch (const std::exception& lc_e) {
			rpc::ex_processing e;
			e.msg = "cannot cast value ";
			e.msg += job_row[3];
			e.msg += " to integer. Query is ";
			e.msg += query;
			e.msg += ". Exception is ";
			e.msg += lc_e.what();
			ERROR << e.msg;
			throw e;
		}
		job->state		= build_job_state_from_string(job_row[4].c_str());

		try {
			this->get_recovery_type(this->get_current_planning_name().c_str(), job->recovery_type, boost::lexical_cast<int>(job_row[5].c_str()));
		} catch (const std::exception& lc_e) {
			rpc::ex_processing e;
			e.msg = "cannot cast value ";
			e.msg += job_row[5];
			e.msg += " to integer. Query is ";
			e.msg += query;
			e.msg += ". Exception is ";
			e.msg += lc_e.what();
			ERROR << e.msg;
			throw e;
		}

		_return.push_back(*job);
	}

	delete job;
	jobs_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_jobs(const char* domain_name, rpc::v_jobs& _return, const char* running_node) {
	std::string	query("SELECT job_name,job_cmd_line,job_node_name,job_weight,job_state,job_rectype_id, unix_timestamp(job_start_time), unix_timestamp(job_stop_time) FROM job");
	v_v_row		jobs_matrix;
	rpc::t_job*	job	= NULL;

	if ( running_node == NULL )
		query += ";";
	else {
		query += " WHERE job_node_name = '";
		query += running_node;
		query += "';";
	}

#ifdef USE_MYSQL
	if ( this->database.query_full_result(jobs_matrix, query.c_str(), domain_name) == false ) {
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
		try {
			job->weight		= boost::lexical_cast<int>(job_row[3]);
		} catch (const std::exception& lc_e) {
			rpc::ex_processing e;
			e.msg = "cannot cast value ";
			e.msg += job_row[3];
			e.msg += " to integer. Query is ";
			e.msg += query;
			e.msg += ". Exception is ";
			e.msg += lc_e.what();
			ERROR << e.msg;
			throw e;
		}
		job->state		= build_job_state_from_string(job_row[4].c_str());

		if ( job_row[6].size() > 0 && job_row[6].compare("NULL") != 0 )
			job->start_time		= boost::lexical_cast<int64_t>(job_row[6].c_str());
		if ( job_row[7].size() > 0 && job_row[7].compare("NULL") != 0 )
			job->stop_time		= boost::lexical_cast<int64_t>(job_row[7].c_str());

		this->get_jobs_next(domain_name, job->nxt, job->name);
		this->get_time_constraints(domain_name, job->time_constraints, job->name);

		if ( job_row[5].size() > 0 && job_row[5].compare("NULL") != 0 ) {
			DEBUG << job_row[5];
			this->get_recovery_type(domain_name, job->recovery_type, boost::lexical_cast<int>(job_row[5].c_str()));
		}

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

	if ( this->database.query_one_row(job_row, query.c_str(), domain_name) == false ) {
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

	this->get_jobs_next(domain_name, _return.nxt, _return.name);
	this->get_time_constraints(domain_name, _return.time_constraints, _return.name);
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_jobs_next(const char* domain_name, rpc::v_job_names& _return, const std::string& j_name) {
	std::string	query("SELECT job_name_nxt FROM jobs_link WHERE job_name_prv = '");
	v_v_row		job_links_matrix;

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
	std::string		query("SELECT macro_id,macro_name FROM macro_job;");
	v_v_row			macro_jobs_matrix;
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

void	Domain::get_resources(const char* domain_name, rpc::v_resources& _return) {
	std::string		query("SELECT resource_name,resource_node_name,resource_current_value,resource_initial_value FROM resource;");
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

		if ( resource_row[2].size() > 0 && resource_row[2].compare("NULL") > 0 )
			resource->current_value		= boost::lexical_cast<int>(resource_row[2]);
		else {
			rpc::ex_processing	e;
			e.msg = "cannot cast ";
			e.msg += resource_row[2];
			e.msg += " to integer";

			throw e;
		}

		if ( resource_row[3].size() > 0 && resource_row[3].compare("NULL") > 0 )
			resource->initial_value		= boost::lexical_cast<int>(resource_row[3]);
		else {
			rpc::ex_processing	e;
			e.msg = "cannot cast ";
			e.msg += resource_row[3];
			e.msg += " to integer";

			throw e;
		}

		_return.push_back(*resource);
	}

	delete resource;
	resources_matrix.clear();
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_time_constraints(const char* domain_name, rpc::v_time_constraints& _return, const std::string& job_name) {
	std::string			query("SELECT time_c_job_name,time_c_type,TIME_TO_SEC(time_c_value) FROM time_constraint WHERE time_c_job_name = '");
	v_v_row				time_constraints_matrix;
	rpc::t_time_constraint*		time_constraint = NULL;

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

		DEBUG << "job_name -> " << time_constraint_row[0].c_str();
		time_constraint->job_name	= time_constraint_row[0].c_str();

		DEBUG << "type -> " << build_time_constraint_type_from_string(time_constraint_row[1].c_str());
		time_constraint->type		= build_time_constraint_type_from_string(time_constraint_row[1].c_str());

		try {
			DEBUG << "value -> " << time_constraint_row[2];
			time_constraint->value		= boost::lexical_cast<rpc::integer>(time_constraint_row[2]);
		} catch (std::exception& lc_e) {
			rpc::ex_processing	e;
			e.msg = "cannot cast ";
			e.msg += time_constraint_row[2];
			e.msg += " to integer - ";
			e.msg += lc_e.what();
		}

		_return.push_back(*time_constraint);
	}

	delete time_constraint;
	time_constraints_matrix.clear(); // TODO: is it really useful ?
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_recovery_types(const char* domain_name, rpc::v_recovery_types& _return) {
	std::string		query("SELECT rectype_id,rectype_short_label,rectype_label,rectype_action FROM recovery_type;");
	v_v_row			recoveries_matrix;
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

		try {
			recovery->id		= boost::lexical_cast<int>(recovery_row[0]);
		} catch (const std::exception& lc_e) {
			rpc::ex_processing e;
			e.msg = "cannot cast ";
			e.msg += recovery_row[0];
			e.msg += " to integer - ";
			e.msg += e.what();

			throw e;
		}

		recovery->short_label	= recovery_row[1];
		recovery->label		= recovery_row[2];
		recovery->action	= build_rectype_action_from_string(recovery_row[3].c_str());

		_return.push_back(*recovery);
	}

	delete recovery;
	recoveries_matrix.clear(); // TODO: is it really useful ?
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_recovery_type(const char* domain_name, rpc::t_recovery_type& _return, const int rec_id) {
	std::string	query("SELECT rectype_id,rectype_short_label,rectype_label,rectype_action FROM recovery_type WHERE rectype_id = '");
	v_row		recovery_row;

	query += rec_id;
	query += "';";

#ifdef USE_MYSQL
	if ( this->database.query_one_row(recovery_row, query.c_str(), domain_name) == false ) {
		rpc::ex_job	e;
		e.msg = "The query failed";
		throw e;
	}
#endif
	try {
		_return.id		= boost::lexical_cast<int>(recovery_row[0]);
	} catch (const std::exception& lc_e) {
		rpc::ex_processing e;
		e.msg = "cannot cast ";
		e.msg += recovery_row[0];
		e.msg += " to integer - ";
		e.msg += e.what();

		throw e;
	}

	_return.short_label	= recovery_row[1];
	_return.label		= recovery_row[2];
	_return.action		= build_rectype_action_from_string(recovery_row[3].c_str());
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_node(const char* domain_name, rpc::t_node& _return, const char* node_name) {
	std::string	query("SELECT node_name,node_weight FROM node WHERE node_name = '");
	v_row		node_row;

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
		this->get_resources(domain_name, _return.resources);
		this->get_jobs(domain_name, _return.jobs, _return.name.c_str());
	}
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_nodes(const char* domain_name, rpc::v_nodes& _return) {
	std::string	query("SELECT node_name,node_weight FROM node;");
	v_v_row		nodes_matrix;
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

		try {
			node->weight	= boost::lexical_cast<rpc::integer>(node_row[1]);
		} catch ( ... ) {
			ERROR << "Error while casting int: " << node_row[1];
		}

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

rpc::integer	Domain::monitor_failed_jobs(const char* domain_name) {
	std::string	query("SELECT COUNT(*) FROM job WHERE job_state = 'failed';");
	v_row		result;

	this->database.query_one_row(result, query.c_str(), domain_name);

	return boost::lexical_cast<rpc::integer>(result.at(0));
}

///////////////////////////////////////////////////////////////////////////////

rpc::integer	Domain::monitor_waiting_jobs(const char* domain_name) {
	std::string	query("SELECT COUNT(*) FROM job WHERE job_state = 'waiting';");
	v_row		result;

	this->database.query_one_row(result, query.c_str(), domain_name);

	return boost::lexical_cast<rpc::integer>(result.at(0));
}

///////////////////////////////////////////////////////////////////////////////

const char*	Domain::get_name() const {
	return this->name.c_str();
}

///////////////////////////////////////////////////////////////////////////////

std::string Domain::get_current_planning_name() {
	std::string	result;

	result = get_name();
	result += "_";
	result += boost::lexical_cast<std::string>(this->planning_start_time);

	return result;
}

///////////////////////////////////////////////////////////////////////////////

void	Domain::get_available_planning_names(std::vector<std::string>& _return) {
	std::string	query;
	v_v_row		result;
	boost::regex	planning_db("^\\w+(_\\d+){0,1}$", boost::regex::perl);

	query = "SHOW DATABASES";

	this->database.query_full_result(result, query.c_str(), NULL);

	BOOST_FOREACH(v_row line, result) {
		if ( boost::regex_match(line.at(0), planning_db) == true && line.at(0).compare("information_schema") != 0 )
			_return.push_back(line.at(0));
	}
}

///////////////////////////////////////////////////////////////////////////////
/*
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
*/
///////////////////////////////////////////////////////////////////////////////

bool	Domain::contains_data(const char* node_name) {
	if ( node_name == NULL ) {
		rpc::ex_processing e;
		e.msg = "the given node_name is NULL";
		throw e;
	}

	v_row	result;
	std::string	query = "SELECT COUNT(*) FROM job WHERE job_name_name LIKE '";
	query += node_name;
	query += "';";

	this->database.query_one_row(result, query.c_str(), this->name.c_str());

	if ( boost::lexical_cast<int>(result[0]) > 0 )
		return true;

	return false;
}
