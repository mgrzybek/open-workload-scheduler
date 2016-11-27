/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: rpc_server.h
 * Description: used to deliver RPC services.
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

#ifndef RPC_SERVER_H
#define RPC_SERVER_H

#include <fstream>
#include <string>
#include <iostream>

#include <boost/lexical_cast.hpp>

// common.h must be included before using the USE_* macros
#include "common.h"
//#include "cfg.h"
#include "router.h"
#include "domain.h"
#include "rpc_client.h"

#ifdef USE_THRIFT
// RPC Stuff
#include "gen-cpp/ows_rpc.h"

// Common Stuff
#include <protocol/TBinaryProtocol.h>
#include <server/TSimpleServer.h>
#include <transport/TServerSocket.h>
#include <transport/TBufferTransports.h>
#endif // USE_THRIFT

// namespace ows {

///////////////////////////////////////////////////////////////////////////////

class Rpc_Object {
public:
	/**
	 * Rpc_Object
	 *
	 * The constructor
	 *
	 * @param	config	the configuration object to use
	 * @param	router	the routing engine to use
	 */
	Rpc_Object(Config*, Router*);

	/**
	 * ~Rpc_Object
	 *
	 * The destructor
	 */
	~Rpc_Object();

protected:
	/**
	 * config
	 *
	 * The configuration object to use to get the settings
	 */
	Config*		config;

	/**
	 * router
	 *
	 * The routing engine to use to communicate
	 */
	Router*		router;

	/**
	 * client
	 *
	 * The RPC client object to use to call RPCs
	 */
	Rpc_Client*	client; // TODO: create rpc_clients on demand to by multi-thread compatible
};

///////////////////////////////////////////////////////////////////////////////

/*
 * TODO: know who is calling to be able to choose how to process the rpc:
 * - is it the master wanting me to update something? -> ACL
 * - loop detection: is node A calling node A?
 */

class Rpc_Server : public Rpc_Object {
public:
	/**
	 * Rpc_Server
	 *
	 * The constructor
	 *
	 * @param	domain	the domain to use
	 * @param	config	the configuration object to use
	 * @param	router	the routing engine to use
	 */
	Rpc_Server(Domain*, Config*, Router*);

	/**
	 * Rpc_Server
	 *
	 * The constructor
	 *
	 * @param	config	the configuration object to use
	 * @param	router	the routing engine to use
	 */
	Rpc_Server(Config*, Router*);

	/**
	 * ~Rpc_Server
	 *
	 * The destructor
	 */
	~Rpc_Server();

	/**
	 * run
	 *
	 * Used to start the server
	 */
	void	run();

	/**
	 * get_master
	 *
	 * Tries to reach the master using the network
	 *
	 * @return	true on success
	 */
	bool	get_master();

	/**
	 * get_planning
	 *
	 * Gets the planning and save it into a file
	 * TODO: is it useful?
	 * @return	the FILE object
	 */
	FILE*	get_planning();

	/**
	 * get_domain
	 *
	 * Gets the Domain object
	 * TODO: is it useful?
	 *
	 * @return	the domain
	 */
	Domain*	get_domain();

private:
	/**
	 * domain
	 *
	 * The domain to server
	 */
	Domain*	domain;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_THRIFT

class ows_rpcHandler : virtual public rpc::ows_rpcIf, public Rpc_Object {
public:
	/**
	 * ows_rpcHandler
	 *
	 * The constructor
	 *
	 * @param	d	the domain to use
	 * @param	c	the configuration object to use
	 * @param	r	the routing engine to use
	 */
	ows_rpcHandler(Domain* d, Config* c, Router* r);

	/*
	 * Please see model.thrift to get the headers
	 */

	// Routing methods
	void hello(rpc::t_hello& _return, const rpc::t_node& target_node);
	void reach_master(rpc::t_route& _return);

	// Planning methods
	void get_current_planning_name(std::string& _return, const rpc::t_routing_data& routing);
	void get_available_planning_names(std::vector<std::string>& _return, const rpc::t_routing_data& routing);
	void get_planning(rpc::t_planning& _return, const rpc::t_routing_data& routing, const rpc::t_node& node_to_get);
	bool set_planning(const rpc::t_node& calling_node, const rpc::t_planning& planning);

	// Nodes methods
	bool add_node(const rpc::t_routing_data& routing, const rpc::t_node& node_to_add); // TODO: fix the weight value
	void get_node(rpc::t_node& _return, const rpc::t_routing_data& routing, const rpc::t_node& node_to_get);
	void get_nodes(rpc::v_nodes& _return, const rpc::t_routing_data& routing);
	bool remove_node(const rpc::t_routing_data& routing, const rpc::t_node& node_to_remove);

	// Jobs methods
	void get_jobs(rpc::v_jobs& _return, const rpc::t_routing_data& routing);
	void get_ready_jobs(rpc::v_jobs& _return, const rpc::t_routing_data& routing);
	void get_job(rpc::t_job& _return, const rpc::t_routing_data& routing, const rpc::t_job& job_to_get);
	bool add_job(const rpc::t_routing_data& routing, const rpc::t_job& j);
	bool update_job(const rpc::t_routing_data& routing, const rpc::t_job& j);
	bool remove_job(const rpc::t_routing_data& routing, const rpc::t_job& j);
	bool update_job_state(const rpc::t_routing_data& routing, const rpc::t_job& j);

	// SQL methods
	void sql_exec(const std::string& query);

	// Monitoring
	rpc::integer monitor_failed_jobs(const rpc::t_routing_data& routing);
	rpc::integer monitor_waiting_jobs(const rpc::t_routing_data& routing);

private:

	Domain*	domain;

	/**
	 * master_node_check
	 *
	 * Used in passive mode to check 2 elements:
	 * - is the calling node the master?
	 * - am I the target of the RPC?
	 *
	 * @param	calling_node_name	the calling node
	 * @param	job_node_name		the job hosting node
	 *
	 * @throw	ex_routing		the check failed
	 */
	void	check_master_node(const std::string& calling_node_name, const std::string& job_node_name);

		/**
	 * check_routing_args
	 *
	 * Used to check if the args are empty or usable
	 *
	 * @param	domain_name	the string that should not be empty
	 * @param	calling_node	its name and its domain_name should be filled in
	 * @param	target_node	the node to reach
	 *
	 * @throw	ex_routing	the raised exception
	 */
	void	check_routing_args(const rpc::t_routing_data& routing);

	/**
	 * check_routing_args
	 *
	 * Used to check if the args are empty or usable
	 *
	 * @param	domain_name	the string that should not be empty
	 * @param	calling_node	its name and its domain_name should be filled in
	 *
	 * @throw	ex_routing	the raised exception
	 */
	void	check_routing_args(const std::string& domain_name, const rpc::t_node& calling_node);

	/**
	 * check_job_arg
	 *
	 * Used to check if the job given as argument is valid
	 *
	 * @param	job	the job to check
	 *
	 * @throw	ex_job	the check failed
	 */
	void	check_job_arg(const rpc::t_job job);

	/**
	 * check_auth
	 *
	 * Used to check if the client is authorized to perform the call
	 *
	 * @param	domain_name	the string that should not be empty
	 * @param	calling_node	its name and its domain_name should be filled in
	 * @param   procedure type  get, add, remove, update, monitoring
	 *
	 * @throw	ex_auth	the raised exception
	 */
	void	check_auth();
};

#endif // USE_THRIFT

///////////////////////////////////////////////////////////////////////////////

// } // namespace ows

#endif // RPC_SERVER_H

