/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: rpc_client.h
 * Description: used to call remote procedures using the selected protocol.
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

#ifndef RPC_CLIENT_H
#define RPC_CLIENT_H

#include <iostream>
#include <fstream>
#include <string>

#include <boost/regex.hpp>
#include <boost/lexical_cast.hpp>

// common.h must be included before using the USE_* macros
#include "common.h"
#include "job.h"
#include "router.h"
#include "config.h"

#ifdef USE_THRIFT
// RPC Stuff
#include "thrift/ows_rpc.h"

// Common Stuff
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#endif //USE_THRIFT

// namespace ows {

class Router;

/*
 * t_hello
 *
 * Defines the values given by a hello request
 */
typedef struct {
	std::string	domain;
	std::string	name;
	bool		is_master;
} t_hello;

class Rpc_Client {
public:
	Rpc_Client(Config* c, Router* r);
	Rpc_Client();
	~Rpc_Client();

	/*
	 * is_reachable
	 *
	 * @arg :
	 * @return :
	 *
	 * Not implemented yet
	 */
	bool			is_reachable(const std::string*, const std::string*);

	/*
	 * sql
	 *
	 * @arg : the gateway to use
	 * @arg : the sql query to execute
	 * @return : nothing
	 *
	 * The result is printed on the server
	 */
	void			sql_exec(const std::string* running_node, const std::string* hostname, const std::string* sql);

	/*
	 * hello
	 *
	 * @arg : the gateway to use
	 * @arg : the target
	 * @return : the hello's result
	 *
	 * Says "hello" to a host and receives an answer :
	 * - name:domain:is_master
	 */
	t_hello*		hello(const std::string* hostname);

	/*
	 * reach_master
	 *
	 * @arg : the target to use to try to reach the master
	 * @return : true (master is reachable) / false (not reachable)
	 *
	 * Tries to reach the master node using known peers
	 */
	bool			reach_master();

	/*
	 * add_job
	 *
	 * Adds a job to the domain
	 *
	 * @arg	target	: the hosting node
	 * @arg	j		: the job to add
	 * @return true	: success
	 */
	bool			add_job(const std::string* target, const Job* j);
	bool			add_job(const std::string* target, const std::string* domain_name, const std::string *name, const std::string* node_name, std::string* cmd_line, int weight, v_job_ids* pj, v_job_ids* nj);

	/*
	 * update_job
	 *
	 * Updates the given job (the id remains the same)
	 *
	 * @arg j		: the job to update
	 * @return true	: success
	 *
	 */
	bool			update_job(const Job*j);

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
	bool			update_job_state(const Job* j);
	bool			update_job_state(const std::string* running_node, const int job_id, const rpc::e_job_state js);

	/*
	 * remove_job
	 *
	 * Removes a job from the domain
	 *
	 * @arg					: the job to remove
	 * @arg j_id			: the jobs's id
	 * @return true			: success
	 */
	bool			remove_job(const Job* j);
	bool			remove_job(const int job_id);

#ifdef USE_THRIFT

	/*
	 * open
	 *
	 * Open a connection to a remote noode
	 *
	 * @arg	hostname	: target's name
	 * @arg	port		: the TCP port to use
	 * @return true		: success
	 */
	bool			open(const char* hostname, const int& port);

	/*
	 * get_client
	 *
	 * Gets the client handler
	 */
	rpc::ows_rpcClient*	get_client() const;

	/*
	 * close
	 *
	 * Closes the connection
	 *
	 * @return true	: success
	 */
	bool			close();

#endif //USE_THRIFT

protected:
	/*
	 * get_planning
	 *
	 * @arg : none
	 * @return : planning's file path
	 *
	 * Downloads the planning from the master and write it to a file
	 */
//	std::string		get_planning();

private:
	/*
	 * config
	 *
	 * The map containing the paramaters read from the .cfg file
	 */
	Config*					config;

	/*
	 * router
	 *
	 * Tells us how to reach nodes
	 */
	Router*					router;

#ifdef USE_THRIFT

	/*
	 * transport
	 *
	 * Thrift's connection handler
	 */
	boost::shared_ptr<apache::thrift::transport::TTransport>	transport;

	/*
	 * client
	 *
	 * Thrift's interface
	 */
	rpc::ows_rpcClient*		client;

#endif // USE_THRIFT

	/*
	 * build_url
	 *
	 * @arg : the target hostname
	 * @return : the complete URL
	 *
	 * Creates a complete URL :
	 * "http://" + target + ":" port + "/" + rpc_path
	 */
	std::string*			build_url(const std::string* target);
};

// } // namespace ows

#endif // RPC_CLIENT_H
