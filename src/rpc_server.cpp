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

#include "rpc_server.h"

Rpc_Object::Rpc_Object(Config* c, Router* r) {
	this->config		= c;
	this->router		= r;
	this->client		= new Rpc_Client(c, r);
}

Rpc_Object::~Rpc_Object() {
	this->config	= NULL;
	this->router	= NULL;
	if ( this->client != NULL ) {
		delete this->client;
		this->client = NULL;
	}
}

///////////////////////////////////////////////////////////////////////////////

Rpc_Server::Rpc_Server(Config *c, Router* r) : Rpc_Object(c, r) {
}

Rpc_Server::Rpc_Server(Domain* d, Config *c, Router* r) : Rpc_Object(c, r) {
	this->domain = d;
}

Rpc_Server::~Rpc_Server() {
	this->domain = NULL;
}

void	Rpc_Server::run() {
//	std::string*	address	= this->config->get_param("bind_address");
	u_int			port	= boost::lexical_cast<u_int>(*this->config->get_param("bind_port"));

#ifdef USE_THRIFT
	try {
		boost::shared_ptr<ows_rpcHandler>								handler(new ows_rpcHandler(this->domain, this->config, this->router));
		boost::shared_ptr<apache::thrift::TProcessor>					processor(new rpc::ows_rpcProcessor(handler));
		boost::shared_ptr<apache::thrift::transport::TServerTransport>	serverTransport(new apache::thrift::transport::TServerSocket(port));
		boost::shared_ptr<apache::thrift::transport::TTransportFactory>	transportFactory(new apache::thrift::transport::TBufferedTransportFactory());
		boost::shared_ptr<apache::thrift::protocol::TProtocolFactory>	protocolFactory(new apache::thrift::protocol::TBinaryProtocolFactory());

		apache::thrift::server::TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
		server.serve();
	} catch (std::exception const& e) {
		std::cerr << "Something failed: " << e.what() << std::endl;
		throw e;
	}
#endif //USE_THRIFT
}

Domain*	Rpc_Server::get_domain() {
	return this->domain;
}

///////////////////////////////////////////////////////////////////////////////

#ifdef USE_THRIFT

ows_rpcHandler::ows_rpcHandler(Domain* d, Config* c, Router* r) : Rpc_Object(c, r) {
	this->domain	= d;
}

void	ows_rpcHandler::hello(rpc::t_hello& _return, const rpc::t_node& target_node) {
	std::string*	result	= NULL;
	std::string*	gateway;

	if ( target_node.name.compare(this->config->get_param("node_name")->c_str()) == 0 ) {
		_return.name = target_node.name.c_str();

		result = this->config->get_param("domain_name");
		_return.domain = result->c_str();

		result = this->config->get_param("is_master");
		if ( result != NULL )
			if ( result->compare("yes") == 0 )
				_return.is_master = true;
			else
				_return.is_master = false;
	} else {
		// Get the better gateway to reach the host
		gateway = this->router->get_gateway(target_node.name);

		if ( gateway == NULL ) {
			rpc::e_routing e;
			e.msg = "Cannot reach target node ";
			e.msg += target_node.name;
			throw e;
		} else {
			try {
				this->client->open(target_node.name.c_str(), boost::lexical_cast<int>(this->config->get_param("port")));
				this->client->get_client()->hello(_return, target_node);
				this->client->close();
			} catch (rpc::e_routing e) {
				this->client->close();
				throw e;
			}
		}
	}
}

void	ows_rpcHandler::reach_master(rpc::t_route& _return) {
	std::string*	master_node_name;
	p_weighted_gateway* p_wg = NULL;

	master_node_name = this->router->get_master_node();

	if ( master_node_name == NULL ) {
		_return.node_name = "";
		_return.hops = -1;
	} else {
		p_wg = this->router->get_route(master_node_name->c_str());
		_return.node_name = p_wg->second;
		_return.hops = p_wg->first;
	}
}

bool	ows_rpcHandler::add_node(const std::string& running_node, const rpc::t_node& node) {
	return this->domain->add_node(running_node.c_str(), node.name, node.weight);
}

void	ows_rpcHandler::get_jobs(rpc::v_jobs& _return, const std::string& running_node) {
	_return = this->domain->get_jobs(running_node.c_str());
}

void	ows_rpcHandler::get_ready_jobs(rpc::v_jobs& _return, const std::string& running_node) {
	_return = this->domain->get_ready_rpc_jobs(running_node.c_str());
}

bool	ows_rpcHandler::add_job(const rpc::t_job& j) {
	v_job_ids	pj;
	v_job_ids	nj;

	BOOST_FOREACH(rpc::integer j_int, j.prv) {
		pj.push_back((int)j_int);
		std::cout << j_int << std::endl;
	}

	BOOST_FOREACH(rpc::integer j_int, j.nxt) {
		nj.push_back((int)j_int);
		std::cout << j_int << std::endl;
	}

	//TODO: create a new Job constructor Job(rpc::t_job& j)
	Job job(this->domain, j.name, j.node_name, j.cmd_line, (int&)j.weight, pj, nj);

	return this->domain->add_job(&job);
}

bool	ows_rpcHandler::remove_job(const rpc::t_job& j) {
	return this->domain->remove_job(j.node_name, (int)j.id);
}

bool	ows_rpcHandler::update_job_state(const rpc::t_job& j, const rpc::e_job_state::type js) {
	return this->domain->update_job_state(j.node_name, j.id, js);
}

void	ows_rpcHandler::sql_exec(const std::string& query) {
	this->domain->sql_exec(query);
}
/*
e_job_state	ows_rpcHandler::cast_e_job_state(const rpc::e_job_state::type& js) {
	switch(js) {
		case rpc::e_job_state::WAITING: {
			return WAITING;
		}
		case rpc::e_job_state::RUNNING: {
			return RUNNING;
		}
		case rpc::e_job_state::SUCCEDED: {
			return SUCCEDED;
		}
		case rpc::e_job_state::FAILED: {
			return FAILED;
		}
		default: {
			throw "Error: js is not from type rpc::e_job_state::type";
		}
	}
	// To avoid a warning: "Control reaches end of non-void function"
	return FAILED;
}
*/
#endif // USE_THRIFT
