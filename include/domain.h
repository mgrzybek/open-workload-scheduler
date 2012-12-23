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
#include <time.h>

#include <boost/lexical_cast.hpp>
#include <boost/foreach.hpp>
#include <boost/regex.hpp>

#include "common.h"
#include "convertions.h"
#include "config.h"
#include "database.h"
#include "job.h"

#include "gen-cpp/ows_rpc.h"

// namespace ows {

class Job;

typedef	std::vector<Job>	v_jobs;

class Domain {
public:
	/**
	 * Domain
	 *
	 * The constructor
	 *
	 * @arg	the configuration object
	 */
	Domain(Config*);

	/**
	 * ~Domain
	 *
	 * The destructor
	 */
	~Domain();

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_planning
	 *
	 * Gets the planning from the master and save it
	 *
	 * @param	_return	the planning to use as output
	 * @param	domain_name	the name of the domain to get
	 * @param	node_name	the node to focus on
	 *
	 * @return	true on success
	 */
	bool	get_planning(rpc::t_planning& _return, const char* domain_name, const char* node_name);

	/**
	 * set_planning
	 *
	 * Sets the planning according to the template
	 *
	 * @return		: success or failure
	 */
	bool	set_next_planning();

	/**
	 * switch_planning
	 *
	 * Activates the next planning
	 *
	 * @return	: success or failure
	 */
	bool	switch_planning();

	/**
	 * dump_planning
	 *
	 * Provides a dump of the database schema
	 * TODO: determine dump's data type as its size can be huge
	 *
	 * @param	planning_name	its name
	 *
	 * @return	the dump
	 */
	std::string	dump_planning(const char* planning_name);

	/**
	 * drop_planning
	 *
	 * Drop the database schema representing the planning
	 * For security reasons a dump is generated
	 *
	 * @param	planning_name	its name
	 *
	 * @return	the dump
	 */
	std::string	drop_planning(const char* planning_name);

	/**
	 * get_next_planning_start_time
	 *
	 * Gets when the next planning should start
	 *
	 * @return	the start time
	 */
	time_t	get_next_planning_start_time();

	/**
	 * get_current_planning_remaining_time
	 *
	 * Gives the remaning time before the end of the current planning
	 *
	 * @return	the remaining time
	 */
	time_t	get_current_planning_remaining_time();

////////////////////////////////////////////////////////////////////////////////

	/**
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @param	domain_name	where the node is added
	 * @param	n		the node to add
	 *
	 * @return	true on success
	 */
	bool	add_node(const char* domain_name, const rpc::t_node& n);

	/**
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @param	domain_name	where the node is added
	 * @param	n		its name
	 *
	 * @return	true on success
	 */
	bool	add_node(const char* domain_name, const char* n);


	/**
	 * add_node
	 *
	 * Adds a node to the domain
	 * If it already exists, does not modify it
	 *
	 * @param	domain_name	where the node is added
	 * @param	n		its name
	 * @param	w		its weight, can be null
	 *
	 * @return	true	success
	 */
	bool	add_node(const char* domain_name, const std::string& n, const rpc::integer& w);

	/**
	 * get_node
	 *
	 * Get the node according to its name and its domain
	 *
	 * @param	domain_name	the name of the domain hosting the node
	 * @param	_return		the node to use as output
	 * @param	node_name	its name
	 */
	void	get_node(const char* domain_name, rpc::t_node& _return, const char* node_name);

	/**
	 * get_nodes
	 *
	 * Gets the nodes hosted by a domain
	 *
	 * @param	domain_name	the name of the domain hosting the nodes
	 * @param	_return		the nodes vector to use as output
	 */
	void	get_nodes(const char* domain_name, rpc::v_nodes& _return);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * add_job
	 *
	 * Adds a job to the domain
	 * Is updates the job's id
	 *
	 * @param	j	the job to add
	 * @return	true on success
	 */
	bool	add_job(const char* domain_name, const rpc::t_job& j);

	/**
	 * update_job
	 *
	 * Updates the given job (the id remains the same)
	 *
	 * @param	domain_name	the name of the domain hosting the nodes
	 * @param	j		the job to update
	 *
	 * @return true on success
	 */
	bool	update_job(const char* domain_name, const rpc::t_job&);

	/**
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	j		the job to remove
	 *
	 * @return	true on success
	 *
	 */
	bool	remove_job(const char* domain_name, const rpc::t_job& j);

	/**
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	j		the job to remove
	 *
	 * @return	true on success
	 *
	 */
	bool	remove_job(const char* domain_name, const Job* j);

	/**
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	running_node	the name of the node running the job
	 * @param	j_name		the jobs's name
	 *
	 * @return	true on success
	 *
	 */
	bool	remove_job(const char* domain_name, const std::string* running_node, const std::string& j_name);

	/**
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	running_node	the name of the node running the job
	 * @param	j_name		the jobs's name
	 *
	 * @return	true on success
	 *
	 */
	bool	remove_job(const char* domain_name, const std::string& running_node, const std::string& j_name);

	/**
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	j		the job to update
	 *
	 * @return	true on success
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const char* domain_name, const rpc::t_job& j);

	/**
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	j		the job to update
	 * @param	js		the new job's state
	 *
	 * @return	true on success
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const char* domain_name, const Job* j, const rpc::e_job_state::type js);

	/**
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	running_node	the name of the node running the job
	 * @param	j_name		the job's name
	 * @param	js		the new job's state
	 *
	 * @return	true on success
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const char* domain_name, const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js);

	/**
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	running_node	the name of the node running the job
	 * @param	j_name		the job's name
	 * @param	js		the new job's state
	 * @param	start_time	the start time of the job
	 * @param	stop_time	the stop time of the job
	 *
	 * @return	true on success
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const char* domain_name, const std::string& running_node, const std::string& j_name, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time);

	/**
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	j		the job to update
	 * @param	js		the new job's state
	 * @param	start_time	the start time of the job
	 * @param	stop_time	the stop time of the job
	 *
	 * @return	true on success
	 *
	 * TODO: use only the Job object (remove the second argument)
	 */
	bool	update_job_state(const char* domain_name, const Job* j, const rpc::e_job_state::type& js, time_t& start_time, time_t& stop_time);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_ready_jobs
	 *
	 * Gets the list of the jobs ready to be launched
	 *
	 * @param	_return		the output
	 * @param	running_node	the node to check
	 */
	void	get_ready_jobs(v_jobs& _return, const char* running_node);

	/**
	 * get_ready_jobs
	 *
	 * Gets the list of the jobs ready to be launched
	 *
	 * @param	_return		the output
	 * @param	running_node	the node to check
	 */
	void	get_ready_jobs(rpc::v_jobs& _return, const char* running_node);

	/**
	 * get_jobs
	 *
	 * Gets the jobs' list
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	_return		the output
	 * @param	running_node	the node to get
	 */
	void	get_jobs(const char* domain_name, rpc::v_jobs& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_job
	 *
	 * Gets the job
	 *
	 * @param	_return		the output
	 * @param	running_node	the node to check
	 * @param	job_name	the job to get
	 */
	void	get_job(const char* domain_name, rpc::t_job& _return, const char* running_node, const char* job_name);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_jobs_next
	 *
	 * Gets the next job_ids to run
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	_return		the output
	 * @param	running_node	the node to check
	 * @param	j_name		the previous job_name
	 */
	void	get_jobs_next(const char* domain_name, rpc::v_job_names& _return, const char* running_node, const std::string& j_name);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_macro_jobs
	 *
	 * Gets the macro_jobs' list
	 *
	 * @param	domain_name	the domain hosting the job
	 * @param	_return		the output
	 * @param	running_node	the node to get
	 */
	void	get_macro_jobs(const char* domain_name, rpc::v_macro_jobs& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * add_resource
	 *
	 * @param	domain_name	the domain hosting the resource
	 * @param	r		the resource to add
	 * @param	node_name	the node using this resource
	 */
	bool	add_resource(const char* domain_name, const rpc::t_resource& r, const char* node_name);

	/**
	 * get_resources
	 *
	 * Gets the resources' list
	 *
	 * @param	domain_name	the domain hosting the resource
	 * @param	_return		the output
	 * @param	node_name	the node using this resource
	 */
	void	get_resources(const char* domain_name, rpc::v_resources& _return, const char* running_node);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_time_constraints
	 *
	 * Gets the time_constraints' list
	 *
	 * @param _return		: the output
	 */
	void	get_time_constraints(const char* domain_name, rpc::v_time_constraints& _return, const char* running_node, const std::string& j_name);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * get_recovery_types
	 *
	 * Gets the recovery_types' list
	 *
	 * @param _return			: the output
	 * @param running_node	: the node hosting the object
	 */
	void	get_recovery_types(const char* domain_name, rpc::v_recovery_types& _return, const char* running_node);

	/**
	 * get_recovery_type
	 *
	 * Gets the recovery_type associated to the given id
	 *
	 * @param _return			: the output
	 * @param running_node	: the node hosting the object
	 * @param rec_id			: the type's id
	 */
	void	get_recovery_type(const char* domain_name, rpc::t_recovery_type& _return, const char* running_node, const int rec_id);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * sql_exec
	 */
	/**
	 * SQL stuff for debug only !
	 */
	void	sql_exec(const std::string& running_node, const std::string& s);
	void	sql_exec(const std::string& s);

////////////////////////////////////////////////////////////////////////////////

	/**
	 * Getters
	 */
	const char*		get_name() const;

	/**
	 * get_current_planning_name
	 *
	 * Get the current running domain's name (domain_name + "_" + start_time)
	 */
	std::string	get_current_planning_name();

	/**
	 * get_available_planning_names
	 *
	 * Get the whole available plannings (the past ones)
	 */
	void	get_available_planning_names(std::vector<std::string>& _return);

////////////////////////////////////////////////////////////////////////////////

private:
	/**
	 * config
	 *
	 * The map containing the paramaters read from the .cfg file
	 */
	Config*	config;
#ifdef USE_MYSQL
	/**
	 * database
	 *
	 * The MySQL handler
	 */
	Mysql	database;
#endif

	/**
	 * name
	 *
	 * The domain's name
	 */
	std::string	name;

	/**
	 * planning_start_time
	 *
	 * When the current planning started
	 */
	time_t	planning_start_time;

	/**
	 * initial_planning_start_time
	 *
	 * When the initial planning started for the first time
	 */
	time_t	initial_planning_start_time;

	/**
	 * planning_duration
	 *
	 * How long a day is
	 */
	time_t	planning_duration;

	/**
	 * updates_mutex
	 *
	 * This mutex is used to prevent concurrent access to the database
	 */
	boost::mutex	updates_mutex;

	/**
	 * get_add_node_query
	 *
	 */
	//void		get_add_node_query(std::string& _return, const rpc::t_node& node);
	//void		get_add_job_query(std::string& _return, const rpc::t_job& job);
	//void		get_add_resource_query(std::string& _return, const rpc::t_resource& resource);
	//void		get_add_recovery_type_query(std::string& _return, const rpc::t_recovery_type& rc_type);
	//void		get_add_macro_job_query(std::string& _return, const rpc::t_macro_job& macro_job);
};

// } // namespace ows

#endif // DOMAIN_H
