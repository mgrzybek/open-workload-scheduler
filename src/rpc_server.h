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
#include "thrift/ows_rpc.h"

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
	Domain*		domain;
};

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_THRIFT

class ows_rpcHandler : virtual public rpc::ows_rpcIf, public Rpc_Object {
public:
	ows_rpcHandler(Domain* d, Config* c, Router* r);

	// Routing methods
	void hello(rpc::t_hello& _return, const rpc::t_node& target_node);
	void reach_master(rpc::t_route& _return);

	// Nodes methods
	bool add_node(const std::string& running_node, const rpc::t_node& node); // TODO: fix the weight value

	// Jobs methods
	void get_jobs(rpc::v_jobs& _return, const std::string& running_node);
	void get_ready_jobs(rpc::v_jobs& _return, const std::string& running_node);
	bool add_job(const rpc::t_job& j);
	bool remove_job(const rpc::t_job& j);
	bool update_job_state(const rpc::t_job& j, const rpc::e_job_state::type js);

	// SQL methods
	void sql_exec(const std::string& query);

private:
	Domain*	domain;

	// TODO: use rpc::e_job directly into the code to avoid using cast functions
	e_job_state	cast_e_job_state(const rpc::e_job_state::type& js);
};

#endif // USE_THRIFT

// } // namespace ows

#endif // RPC_SERVER_H

