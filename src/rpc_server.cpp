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
	this->client		= new Rpc_Client();
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

///////////////////////////////////////////////////////////////////////////////

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

		// TODO: use the multi-threaded version
		apache::thrift::server::TSimpleServer server(processor, serverTransport, transportFactory, protocolFactory);
		server.serve();
	} catch (std::exception const& e) {
		std::cerr << "Something failed: " << e.what() << std::endl;
		throw e;
	}
#endif //USE_THRIFT
}

///////////////////////////////////////////////////////////////////////////////

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
		if ( result != NULL ) {
			if ( result->compare("yes") == 0 ) {
				_return.is_master = true;
			} else {
				_return.is_master = false;
			}
		}
	} else {
		// Get the better gateway to reach the host
		gateway = this->router->get_gateway(target_node.name);

		if ( gateway == NULL ) {
			rpc::ex_routing e;
			e.msg = "Cannot reach target node ";
			e.msg += target_node.name;
			throw e;
		} else {
			try {
				this->client->open(target_node.name.c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
				this->client->get_handler()->hello(_return, target_node);
				this->client->close();
			} catch (rpc::ex_routing e) {
				this->client->close();
				throw e;
			}
		}
	}
}

void	ows_rpcHandler::reach_master(rpc::t_route& _return) {
	std::string*	master_node_name;
	p_weighted_gateway* p_wg = NULL;

	if ( this->config->get_param("is_master")->compare("yes") == 0 ) {
		_return.destination_node.name = this->config->get_param("node_name")->c_str();
		_return.hops = 0;
	} else {
		master_node_name = this->router->get_master_node();

		if ( master_node_name == NULL ) {
			_return.destination_node.name = "";
			_return.hops = -1;
		} else {
			p_wg = this->router->get_route(master_node_name->c_str());
			if ( p_wg != NULL) {
				_return.destination_node.name = p_wg->second;
				_return.hops = p_wg->first;
			} else {
				rpc::ex_routing e;
				e.msg = "Cannot reach the master node";
				throw e;
			}
		}
	}
}

void	ows_rpcHandler::get_current_planning_name(std::string& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	std::string*	gateway;

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the master?
			 * - yes: give the plannings
			 * - no: forward
			 */
			if ( this->config->get_param("is_master")->compare("yes") != 0 ) {
				gateway = this->router->get_gateway(this->config->get_master_node()->c_str());
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_current_planning_name(_return, domain_name, calling_node, target_node);
					this->client->close();
				} catch (rpc::ex_planning e) {
					this->client->close();
					throw e;
				}
				break;
			}

			_return = this->domain->get_current_planning_name();

			break;
		}
		case PASSIVE: {
			rpc::ex_routing	e;
			e.msg = this->config->get_param("node_name")->c_str();
			e.msg += " is not the master_node";
			throw e;
			break;
		}
	}
}

void	ows_rpcHandler::get_available_planning_names(std::vector<std::string>& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	std::string*	gateway;

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the master?
			 * - yes: give the plannings
			 * - no: forward
			 */
			if ( this->config->get_param("is_master")->compare("yes") != 0 ) {
				gateway = this->router->get_gateway(this->config->get_master_node()->c_str());
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_available_planning_names(_return, domain_name, calling_node, target_node);
					this->client->close();
				} catch (rpc::ex_planning e) {
					this->client->close();
					throw e;
				}
				break;
			}

			this->domain->get_available_planning_names(_return);

			break;
		}
		case PASSIVE: {
			rpc::ex_routing	e;
			e.msg = this->config->get_param("node_name")->c_str();
			e.msg += " is not the master_node";
			throw e;
			break;
		}
	}
}

void	ows_rpcHandler::get_planning(rpc::t_planning& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node, const rpc::t_node& node_to_get) {
	std::string*	gateway;

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the master?
			 * - yes: give the planning
			 * - no: forward
			 */
			if ( this->config->get_param("is_master")->compare("yes") != 0 ) {
				gateway = this->router->get_gateway(this->config->get_master_node()->c_str());
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_planning(_return, domain_name, calling_node, target_node, node_to_get);
					this->client->close();
				} catch (rpc::ex_planning e) {
					this->client->close();
					throw e;
				}
				break;
			}

			this->domain->get_planning(_return, domain_name.c_str(), node_to_get.name.c_str());

			break;
		}
		case PASSIVE: {
			rpc::ex_routing	e;
			e.msg = this->config->get_param("node_name")->c_str();
			e.msg += " is not the master_node";
			throw e;
			break;
		}
	}
}
/*
bool	ows_rpcHandler::set_planning(const rpc::t_node& calling_node, const rpc::t_planning& planning) {
	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			rpc::ex_routing e;
			e.msg = "Cannot set_planning in ACTIVE mode";
			throw e;
			break;
		}
		case PASSIVE: {
			if ( this->config->get_param("is_master")->compare("yes") == 0 ) {
				rpc::ex_routing e;
				e.msg = "Cannot set_planning on the master";
				throw e;
			}

			this->check_master_node(calling_node.name, planning.hosting_node.name);

			return this->domain->set_planning(planning);

			break;
		}
	}
	return false;
}
*/
bool	ows_rpcHandler::add_node(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& hosting_node, const rpc::t_node& node_to_add) {
	std::string*	gateway;
	bool			result;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node, hosting_node);

	if ( node_to_add.name.empty() == true or node_to_add.weight < 0 ) {
		rpc::ex_node e;
		e.msg = "The node to add does not contain the minimal attributes";
		throw e;
	}

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(hosting_node.name) != 0 ) {
				gateway = this->router->get_gateway(hosting_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					result = this->client->get_handler()->add_node(domain_name, calling_node, hosting_node, node_to_add);
					this->client->close();
				} catch (rpc::ex_node e) {
					this->client->close();
					throw e;
				}
				return result;
			}

			return this->domain->add_node(domain_name.c_str(), node_to_add);
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master and am I the target?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, hosting_node.name);
			return this->domain->add_node(hosting_node.name.c_str(), node_to_add.name, node_to_add.weight);
			break;
		}
	}
	return false;
}

void	ows_rpcHandler::get_node(rpc::t_node& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node, const rpc::t_node& node_to_get) {
	std::string*	gateway;

	this->check_routing_args(domain_name, calling_node, target_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: get the node
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(target_node.name) != 0 ) {
				gateway = this->router->get_gateway(target_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_node(_return, domain_name, calling_node, target_node, node_to_get);
					this->client->close();
				} catch (rpc::ex_node e) {
					this->client->close();
					throw e;
				}
			}

			this->domain->get_node(domain_name.c_str(), _return, node_to_get.name.c_str());
			break;
		}
		case PASSIVE: {break;}
	}
}

void ows_rpcHandler::get_nodes(rpc::v_nodes& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	std::string*	gateway;

	this->check_routing_args(domain_name, calling_node, target_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: get the node
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(target_node.name) != 0 ) {
				gateway = this->router->get_gateway(target_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_nodes(_return, domain_name, calling_node, target_node);
					this->client->close();
				} catch (rpc::ex_node e) {
					this->client->close();
					throw e;
				}
			}

			this->domain->get_nodes(domain_name.c_str(), _return);
			break;
		}
		case PASSIVE: {break;}
	}
}

void	ows_rpcHandler::get_jobs(rpc::v_jobs& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	std::string*	gateway;

	this->check_routing_args(domain_name, calling_node, target_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(target_node.name) != 0 ) {
				gateway = this->router->get_gateway(target_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_jobs(_return, domain_name, calling_node, target_node);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

			this->domain->get_jobs(domain_name.c_str(), _return, target_node.name.c_str());
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, target_node.name);
			this->domain->get_jobs(domain_name.c_str(), _return, target_node.name.c_str());
			break;
		}
	}
}
// TODO: check if we need to keep the domain_name argument
void	ows_rpcHandler::get_ready_jobs(rpc::v_jobs& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	std::string*	gateway;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node, target_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(target_node.name) != 0 ) {
				gateway = this->router->get_gateway(target_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_ready_jobs(_return, domain_name, calling_node, target_node);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

			this->domain->get_ready_jobs(_return, target_node.name.c_str());
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, target_node.name);
			this->domain->get_ready_jobs(_return, target_node.name.c_str());
			break;
		}
	}
}

void	ows_rpcHandler::get_job(rpc::t_job& _return, const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node, const rpc::t_job& job_to_get) {
	std::string*	gateway;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node, target_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(target_node.name) != 0 ) {
				gateway = this->router->get_gateway(target_node.name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					this->client->get_handler()->get_job(_return, domain_name, calling_node, target_node, job_to_get);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

			this->domain->get_job(domain_name.c_str(), _return, target_node.name.c_str(), job_to_get.name.c_str());
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, target_node.name);
			this->domain->get_job(domain_name.c_str(), _return, target_node.name.c_str(), job_to_get.name.c_str());
			break;
		}
	}
}

bool	ows_rpcHandler::add_job(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j) {
	std::string*	gateway = NULL;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node);
	this->check_job_arg(j);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(j.node_name) != 0 ) {
				gateway = this->router->get_gateway(j.node_name);
				if ( gateway == NULL ) {
					e.msg = "The node is not in the routing table";
					throw e;
				}

				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					return this->client->get_handler()->add_job(domain_name, calling_node, j);
					this->client->close();
				} catch (rpc::ex_job e) {
					std::cerr << "Error: " << e.msg << std::endl;
					this->client->close();
					throw e;
				}
				break;
			}

			return this->domain->add_job(domain_name.c_str(), j);

			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, j.node_name);
			return this->domain->add_job(domain_name.c_str(), j);
			break;
		}
	}
	return false;
}

bool	ows_rpcHandler::update_job(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j) {
	std::string*	gateway = NULL;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node);
	this->check_job_arg(j);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(j.node_name) != 0 ) {
				gateway = this->router->get_gateway(j.node_name);
				if ( gateway == NULL ) {
					e.msg = "The node is not in the routing table";
					throw e;
				}

				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					return this->client->get_handler()->update_job(domain_name, calling_node, j);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

			return this->domain->update_job(domain_name.c_str(), j);

			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, j.node_name);
			return this->domain->add_job(domain_name.c_str(), j);
			break;
		}
	}
	return false;
}

bool	ows_rpcHandler::remove_job(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j) {
	std::string*	gateway;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(j.node_name) != 0 ) {
				gateway = this->router->get_gateway(j.node_name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					return this->client->get_handler()->remove_job(domain_name, calling_node, j);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

				return this->domain->remove_job(domain_name.c_str(), j);
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, j.node_name);
			return this->domain->remove_job(domain_name.c_str(), calling_node.name, j.name);
			break;
		}
	}
	return false;
}

bool	ows_rpcHandler::update_job_state(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_job& j) {
	std::string*	gateway;
	rpc::ex_routing	e;

	this->check_routing_args(domain_name, calling_node);

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			if ( this->config->get_param("node_name")->compare(j.node_name) != 0 ) {
				gateway = this->router->get_gateway(j.node_name);
				try {
					this->client->open(gateway->c_str(), boost::lexical_cast<int>(this->config->get_param("port")->c_str()));
					return this->client->get_handler()->update_job_state(domain_name, calling_node, j);
					this->client->close();
				} catch (rpc::ex_job e) {
					this->client->close();
					throw e;
				}
				break;
			}

			return this->domain->update_job_state(domain_name.c_str(), j);
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->check_master_node(calling_node.name, j.node_name);
			return this->domain->update_job_state(domain_name.c_str(), j);
			break;
		}
	}
	return false;
}

void	ows_rpcHandler::sql_exec(const std::string& query) {
//	std::string*	gateway;

	switch (this->config->get_running_mode()) {
		case P2P: {break;}
		case ACTIVE: {
			/*
			 * am I the hosting_node?
			 * - yes: add node_to_add
			 * - no: forward
			 */
			this->domain->sql_exec(query);
			break;
		}
		case PASSIVE: {
			/*
			 * is calling_node the master?
			 * - yes: do it
			 * - no: none
			 */
			this->domain->sql_exec(query);
			break;
		}
	}

}

void	ows_rpcHandler::check_master_node(const std::string& calling_node_name, const std::string& target_node_name) {
	rpc::ex_routing	e;

	if ( this->router->get_master_node()->compare(calling_node_name) != 0 or this->config->get_param("node_name")->compare(target_node_name) != 0 ) {
		e.msg = calling_node_name;
		e.msg += " is not the master_node";
		throw e;
	}
}

void	ows_rpcHandler::check_routing_args(const std::string& domain_name, const rpc::t_node& calling_node, const rpc::t_node& target_node) {
	rpc::ex_routing e;

	this->check_routing_args(domain_name, calling_node);

	if ( target_node.name.empty() == true ) {
		e.msg = "target_node's name is empty";
		throw e;
	}

	if ( target_node.domain_name.empty() == true ) {
		e.msg = "target_node's domain_name is empty";
		throw e;
	}
}

void	ows_rpcHandler::check_routing_args(const std::string& domain_name, const rpc::t_node& calling_node) {
	rpc::ex_routing e;

	if ( domain_name.empty() == true ) {
		e.msg = "domain_name is empty";
		throw e;
	}

	if ( calling_node.name.empty() == true ) {
		e.msg = "calling_node's name is empty";
		throw e;
	}

	if ( calling_node.domain_name.empty() == true ) {
		e.msg = "calling_node's domain_name is empty";
		throw e;
	}
}

void	ows_rpcHandler::check_job_arg(const rpc::t_job job) {
	rpc::ex_job e;

	if ( job.name.empty() == true ) {
		e.msg = "job_name is empty";
		throw e;
	}

	if ( job.domain.empty() == true ) {
		e.msg = "domain_name is empty";
		throw e;
	}

	if ( job.cmd_line.empty() == true ) {
		e.msg = "cmd_line is empty";
		throw e;
	}

	if ( job.node_name.empty() == true ) {
		e.msg = "node_name is empty";
		throw e;
	}

	if ( job.weight < 1 ) {
		e.msg = "weight must be higher than 0";
		throw e;
	}
}

#endif // USE_THRIFT
