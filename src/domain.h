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

#include "thrift/ows_rpc.h"

// namespace ows {

class Job;

typedef	std::vector<Job>				v_jobs;
#ifdef USE_SQLITE
typedef	std::map<std::string, Sqlite>	m_nodes;
typedef	std::pair<std::string, Sqlite>	p_nodes;
#endif

class Domain {
public:
	Domain(Config*);
	~Domain();

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
	bool				add_node(const char* running_node, const char* n);

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
	bool				add_node(const char* running_node, const std::string& n, const int&  w);


	/*
	 * add_job
	 *
	 * Adds a job to the domain
	 * Is updates the job's id
	 *
	 * @arg j		: the job to add
	 * @return true	: sucess
	 */
	bool				add_job(Job* j);

	/*
	 * update_job
	 *
	 * Updates the given job (the id remains the same)
	 *
	 * @arg			: the job to update
	 * @return true	: success
	 */
	bool				update_job(const Job*);

	/*
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @arg					: the job to remove
	 * @arg	running_node	: the name of the node running the job
	 * @arg j_id			: the jobs's id
	 * @return true			: success
	 */
	bool				remove_job(const Job*);
	bool				remove_job(const std::string* running_node, const int j_id);
	bool				remove_job(const std::string& running_node, const int j_id);

	/*
	 * update_job_state
	 *
	 * Updates the job's state
	 *
	 * @arg					; the job to update
	 * @arg running_node	: the name of the node running the job
	 * @arg j_id			: the job's id
	 * @arg js				: the new job's state
	 */
	bool				update_job_state(const Job*, const rpc::e_job_state::type);
	bool				update_job_state(const std::string& running_node, const int& j_id, const rpc::e_job_state::type& js);

	/*
	 * get_ready_jobs
	 *
	 * Gets the list of the jobs ready to be launched
	 *
	 * @arg	running_node	: the node to check
	 * @return				: the jobs' list
	 */
	v_jobs				get_ready_jobs(const char* running_node);
	rpc::v_jobs			get_ready_rpc_jobs(const char* running_node);

	/*
	 *
	 */
//	v_jobs				get_jobs(const char* running_node);
	rpc::v_jobs			get_jobs(const char* running_node);

	/*
	 * sql_exec
	 */
	/*
	 * SQL stuff for debug only !
	 */
	void				sql_exec(const std::string& running_node, const std::string& s);
	void				sql_exec(const std::string& s);

	/*
	 * Getters
	 */
	std::string*		get_name() const;

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
	std::string*	name;

	/*
	 * updates_mutex
	 *
	 * This mutex is used to prevent concurrent access to the database
	 */
	boost::mutex	updates_mutex;
};

// } // namespace ows

#endif // DOMAIN_H
