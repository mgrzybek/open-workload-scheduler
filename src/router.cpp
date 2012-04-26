/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: router.cpp
 * Description: deals with communications between neighbours.
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

#include "router.h"

Router::Router(Config* c) {
	this->config		= c;
	this->rpc_client	= new Rpc_Client(c, this);

}

Router::~Router() {
	delete this->rpc_client;
	this->config = NULL;
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::get_node(const std::string& domain_name, rpc::t_node& node) {
	rpc::t_node	local;
	rpc::t_node	target;

	local.name	= this->config->get_param("node_name")->c_str();
	target.name = this->get_master_node()->c_str();

	try {
		this->rpc_client->open(this->get_gateway(*this->get_master_node())->c_str(), boost::lexical_cast<int>(this->config->get_param("port")));
		this->rpc_client->get_handler()->get_node(node, domain_name, local, target, local);
	} catch (std::exception& e) {
		std::cerr << "Cannot get the planning" << std::endl;
		return false;
	}
	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::update_peers_list() {
	std::string		line;
	rpc::t_node		peer;
	std::string		public_key;
	size_t			position	= 0;
	rpc::t_hello	hello_result;
	std::string*	peers_keys	= this->config->get_param("peers_keys");

	if ( peers_keys == NULL or peers_keys->empty() == true ) {
		std::cerr << "peers_keys is empty" << std::endl;
		return false;
	}

	peer.weight = 0;

	std::ifstream	f	(peers_keys->c_str(), std::ifstream::in);


	if ( f.is_open() )
		while ( ! f.eof() ) {
			getline(f, line);

			if ( line.compare(0, 1, "#") == 0 or line.empty() == true )
				continue;

			position	= line.find_first_of("\t");

			if ( position == std::string::npos ) {
				f.close();
				return false;
			}

			peer.name		= line.substr(0, position);
			public_key		= line.substr(position+1, line.length());

			if ( position != 0 ) {
				try {
					this->rpc_client->open(peer.name.c_str(), boost::lexical_cast<int>(this->config->get_param("port")));
					this->rpc_client->get_handler()->hello(hello_result, peer);
				} catch (std::exception& e) {
					this->hosts_keys.erase(peer.name);
				}

				this->hosts_keys.insert(p_host_key(peer.name, public_key));
				this->insert_route(peer.name, peer.name, peer.weight);

				if ( hello_result.is_master == true )
					this->set_master_node(hello_result.name.c_str());

			}
		}
	else
		return false;

	f.close();
	return true;
}

///////////////////////////////////////////////////////////////////////////////

m_host_keys::const_iterator	Router::get_direct_peers() {
	m_host_keys::const_iterator iter = this->hosts_keys.begin();
	return iter;
}

///////////////////////////////////////////////////////////////////////////////

/*
bool	Router::update_routing_table() {
	for ( t_routing_table::const_iterator iter = routing_table.begin() ; iter != routing_table.end() ; ++iter ) {
	}
	return false;
}
*/

///////////////////////////////////////////////////////////////////////////////

bool	Router::insert_route(const std::string& destination, const std::string& gateway, const u_int& weight) {
	m_routing_table::iterator	iter_t;
	m_weighted_gateway*			mm_g;

	iter_t = this->routing_table.find(destination);

	if ( iter_t != this->routing_table.end() ) {
		mm_g = &(iter_t->second);
		mm_g->insert(p_weighted_gateway(weight, gateway));
	} else {
		mm_g = new m_weighted_gateway;
		mm_g->insert(p_weighted_gateway(weight, gateway));
		this->routing_table.insert(p_routing_table(destination, *mm_g));
		delete mm_g;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::delete_route(const std::string* destination, const std::string* gateway) {
	m_routing_table::iterator		iter_t;
	m_weighted_gateway::iterator	iter_g;
	m_weighted_gateway*				mm_g;

	if ( this->routing_table.empty() == true )
		return false;

	if ( destination != NULL and gateway == NULL )
		this->routing_table.erase(*destination);
	else if ( destination != NULL and gateway != NULL ) {
		iter_t = this->routing_table.find(*destination);

		if ( iter_t == this->routing_table.end() )
			return false;

		mm_g =&(iter_t->second);
		mm_g->get<1>().erase(*gateway);
	} else if ( destination == NULL and gateway != NULL) {
		for ( iter_t = this->routing_table.begin() ; iter_t != this->routing_table.end() ; iter_t++ ) {
			mm_g = &(iter_t->second);
			mm_g->get<1>().erase(*gateway);
		}
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

p_weighted_gateway*		Router::get_route(const std::string& destination) {
	m_routing_table::iterator			iter_dst;
	m_weighted_gateway::const_iterator	iter_gtw;

	iter_dst = this->routing_table.find(destination);

	if ( iter_dst != this->routing_table.end() ) {
		iter_gtw = iter_dst->second.begin();
		return new p_weighted_gateway(iter_gtw->first, iter_gtw->second);
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////
/*
std::string*	Router::get_gateway(const std::string* destination) {
	m_routing_table::iterator			iter_dst;
	m_weighted_gateway::const_iterator	iter_gtw;

	iter_dst = this->routing_table.find(*destination);

	if ( iter_dst != this->routing_table.end() ) {
		iter_gtw = iter_dst->second.begin();
		return &(iter_gtw->second);
	}

	return NULL;
}
*/
///////////////////////////////////////////////////////////////////////////////

std::string*	Router::get_gateway(const std::string& destination) {
	m_routing_table::iterator			iter_dst;
	m_weighted_gateway::const_iterator	iter_gtw;

	iter_dst = this->routing_table.find(destination);

	if ( iter_dst != this->routing_table.end() ) {
		iter_gtw = iter_dst->second.begin();
		return &(iter_gtw->second);
	}

	return NULL;
}

///////////////////////////////////////////////////////////////////////////////

std::string*	Router::get_gateway(const char* destination) {
	std::string	buffer(destination);
	return this->get_gateway(buffer);
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::reach_master() {
	if ( this->master_node.empty() == false )
		return true;

	// Call reach_master against the direct peers
	BOOST_FOREACH(p_host_key host, this->hosts_keys ) {
		if ( this->reach_master(host.first.c_str()) == true )
			return true;
	}
	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::reach_master(const char* target) {
	rpc::t_route	result;

	this->rpc_client->open(target, boost::lexical_cast<int>(this->config->get_param("port")));
	this->rpc_client->get_handler()->reach_master(result);

	// We may have found the master, so update the routing table
	if ( result.destination_node.name.empty() == false and result.hops >= 0 ) {
		this->set_master_node(result.destination_node.name.c_str());
		this->insert_route(result.destination_node.name, target, result.hops);
		return true;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Router::set_master_node(const char* node) {
	if ( node == NULL )
		return false;

	if ( this->master_node.empty() == false )
		return false;
	else
		this->master_node.assign(node);

	return true;
}

///////////////////////////////////////////////////////////////////////////////

std::string*	Router::get_master_node() {
	return	&(this->master_node);
}

///////////////////////////////////////////////////////////////////////////////

u_int	Router::get_reachable_peers_number() {
	return this->hosts_keys.size();
}

///////////////////////////////////////////////////////////////////////////////
