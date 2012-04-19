/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: domain.h
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

#ifndef DOMAIN_H
#define DOMAIN_H

#include <iostream>
#include <vector>
#include <string>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>

#include "common.h"
#include "config.h"
#include "database.h"
#include "job.h"

#include "gen-cpp/ows_rpc.h"

// namespace ows {

class Job;

typedef	std::vector<Job>				v_jobs;
#ifdef USE_SQLITE
typedef	std::map<std::string, Sqlite*>	m_nodes;
typedef	std::pair<std::string, Sqlite*>	p_nodes;
#endif

class Domain {
public:
	Domain(Config*);
	~Domain();

////////////////////////////////////////////////////////////////////////////////

	/*
	 * get_planning
	 *
	 * Gets the planning from the master and save it
	 *
	 * TODO: Two options are possible:
	 * 1. get the dump from the master:
	 * 	- MySQL: SQL dump
	 * 	- SQLite: the .db file
	 * 2. use the rpc function (add_job...)
	 */
	bool	get_planning(const std::string& node_name);

	/*
	 * set_planning
	 *
	 * Sets the planning according to the t_planning
	 *
	 * @arg planning	: the planning to insert
	 */
	bool	set_planning(const rpc::t_planning& planning);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @arg n				: the node to add
	 *
	 * @return true	: success
	 */
	bool	add_node(const rpc::t_node& n);

	/*
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @arg running_node	: where the job is run
	 * @arg n				: its name
	 *
	 * @return true	: success
	 */
	bool	add_node(const char* running_node, const char* n);


	/*
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @arg running_node	: the node where the node is added
	 * @arg n				: its name
	 * @arg w				: its weight, can be null
	 * @return true			: success
	 */
	bool	add_node(const char* running_node, const std::string& n, const int&  w);

	/*
	 * get_node
	 *
	 */
	void	get_node(rpc::t_node& _return, const char* node_name, const char* running_node);

	/*
	 * get_nodes
	 *
	 */
	void	get_nodes(rpc::v_nodes& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * add_job
	 *
	 * Adds a job to the domain
	 * Is updates the job's id
	 *
	 * @arg j		: the job to add
	 * @return true	: sucess
	 */
	bool	add_job(const rpc::t_job& j);

	/*
	 * update_job
	 *
	 * Updates the given job (the id remains the same)
	 *
	 * @arg			: the job to update
	 * @return true	: success
	 */
	bool	update_job(const rpc::t_job&);

	/*
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @arg					: the job to remove
	 * @arg	running_node	: the name of the node running the job
	 * @arg j_name			: the jobs's name
	 * @return true			: success
	 */
	bool	remove_job(const rpc::t_job& j);
	bool	remove_job(const Job*);
	bool	remove_job(const std::string* running_node, const std::string& j_name);
	bool	remove_job(const std::string& running_node, const std::string& j_name);

	/*
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @arg					; the job to update
	 * @arg running_node	: the name of the node running the job
	 * @arg j_name			: the job's name
	 * @arg js				: the new job's state
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const rpc::t_job&);
	bool	update_job_state(const Job*, const rpc::e_job_state::type);
	bool	update_job_state(const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js);
	bool	update_job_state(const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time);
	bool	update_job_state(const Job* j, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time);

	/*
	 * get_ready_jobs
	 *
	 * Gets the list of the jobs ready to be launched
	 *
	 * @arg _return			: the output
	 * @arg	running_node	: the node to check
	 */
	void	get_ready_jobs(v_jobs& _return, const char* running_node);
	void	get_ready_jobs(rpc::v_jobs& _return, const char* running_node);

	/*
	 * get_jobs
	 *
	 * Gets the jobs' list
	 *
	 * @arg _return			: the output
	 * @arg	running_node	: the node to check
	 */
//	v_jobs	get_jobs(const char* running_node);
	void	get_jobs(rpc::v_jobs& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * get_jobs_next
	 *
	 * Gets the next job_ids to run
	 *
	 * @arg	_return			: the output
	 * @arg	running_node	: the node to check
	 * @arg j_name			: the previous job_name
	 */
	void	get_jobs_next(rpc::v_job_names& _return, const char* running_node, const std::string& j_name);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * get_macro_jobs
	 *
	 * Gets the macro_jobs' list
	 *
	 * @arg _return		: the output
	 * @arg	running_node	: the node to check
	 */
	void	get_macro_jobs(rpc::v_macro_jobs& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * add_resource
	 *
	 */
	bool	add_resource(const rpc::t_resource& r, const char* node_name);

	/*
	 * get_resources
	 *
	 * Gets the resources' list
	 *
	 * @arg _return		: the output
	 */
	void	get_resources(rpc::v_resources& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * get_time_constraints
	 *
	 * Gets the time_constraints' list
	 *
	 * @arg _return		: the output
	 */
	void	get_time_constraints(rpc::v_time_constraints& _return, const char* running_node, const std::string& j_name);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * get_recovery_types
	 *
	 * Gets the recovery_types' list
	 *
	 * @arg _return			: the output
	 * @arg running_node	: the node hosting the object
	 */
	void	get_recovery_types(rpc::v_recovery_types& _return, const char* running_node);

	/*
	 * get_recovery_type
	 *
	 * Gets the recovery_type associated to the given id
	 *
	 * @arg _return			: the output
	 * @arg running_node	: the node hosting the object
	 * @arg rec_id			: the type's id
	 */
	void	get_recovery_type(rpc::t_recovery_type& _return, const char* running_node, const int rec_id);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * sql_exec
	 */
	/*
	 * SQL stuff for debug only !
	 */
	void	sql_exec(const std::string& running_node, const std::string& s);
	void	sql_exec(const std::string& s);

////////////////////////////////////////////////////////////////////////////////

	/*
	 * Getters
	 */
//	std::string*		get_name() const;

////////////////////////////////////////////////////////////////////////////////

#ifdef USE_SQLITE
	/*
	 * get_database
	 *
	 * Gets the node's database
	 *
	 * @arg	node_name	: the node's name
	 * @return			: the SQLite handler
	 */
	Sqlite*				get_database(const char* node_name);
#endif

////////////////////////////////////////////////////////////////////////////////

private:
	/*
	 * config
	 *
	 * The map containing the paramaters read from the .cfg file
	 */
	Config*			config;
#ifdef USE_MYSQL
	/*
	 * database
	 *
	 * The MySQL handler
	 */
	Mysql			database;
#endif
#ifdef USE_SQLITE
	/*
	 * databases
	 *
	 * The SQLite handlers {node <=> handler}
	 */
	m_nodes			databases;
#endif

	/*
	 * name
	 *
	 * The domain's name
	 */
	std::string		name;

	/*
	 * updates_mutex
	 *
	 * This mutex is used to prevent concurrent access to the database
	 */
	boost::mutex	updates_mutex;

	/*
	 * get_add_node_query
	 *
	 */
	void		get_add_node_query(std::string& _return, const rpc::t_node& node);
	void		get_add_job_query(std::string& _return, const rpc::t_job& job);
	void		get_add_resource_query(std::string& _return, const rpc::t_resource& resource);
	void		get_add_recovery_type_query(std::string& _return, const rpc::t_recovery_type& rc_type);
	void		get_add_macro_job_query(std::string& _return, const rpc::t_macro_job& macro_job);
};

// } // namespace ows

#endif // DOMAIN_H
