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
#include "config.h"
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
	Rpc_Object(Config*, Router*);
	~Rpc_Object();

protected:
	Config*		config;
	Router*		router;
	Rpc_Client*	client;
};

///////////////////////////////////////////////////////////////////////////////

/*
 * TODO: know who is calling to be able to choose how to process the rpc:
 * - is it the master wanting me to update something? -> ACL
 * - loop detection: is node A calling node A?
 */

class Rpc_Server : public Rpc_Object {
public:
	Rpc_Server(Domain*, Config*, Router*);
	Rpc_Server(Config*, Router*);
	~Rpc_Server();

	void	run();

	bool	get_master();
	FILE*	get_planning();

	Domain*	get_domain();

private:
	Domain*	domain;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_THRIFT

class ows_rpcHandler : virtual public rpc::ows_rpcIf, public Rpc_Object {
public:
	ows_rpcHandler(Domain* d, Config* c, Router* r);

	// Routing methods
	void hello(rpc::t_hello& _return, const rpc::t_node& target_node);
	void reach_master(rpc::t_route& _return);

	// Planning methods
	void get_planning(rpc::t_planning& _return);
	bool set_planning(const rpc::t_node& calling_node, const rpc::t_planning& planning);

	// Nodes methods
	bool add_node(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& hosting_node, const rpc::t_node& node_to_add); // TODO: fix the weight value
	void get_node(rpc::t_node& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node, const rpc::t_node& node_to_get);

	// Jobs methods
	void get_jobs(rpc::v_jobs& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node);
	void get_ready_jobs(rpc::v_jobs& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node);
	bool add_job(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j);
	bool remove_job(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j);
	bool update_job_state(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j);

	// SQL methods
	void sql_exec(const std::string& query);

private:
	Domain*	domain;

	/*
	 * master_node_check
	 *
	 * Used in passive mode to check 2 elements:
	 * - is the calling node the master?
	 * - am I the target of the RPC?
	 *
	 * @arg calling_node_name	: the calling node
	 * @arg job_node_name		: the job hosting node
	 * @throw ex_routing		: the check failed
	 */
	void	check_master_node(const std::string& calling_node_name, const std::string& job_node_name);
};

#endif // USE_THRIFT

///////////////////////////////////////////////////////////////////////////////

// } // namespace ows

#endif // RPC_SERVER_H

