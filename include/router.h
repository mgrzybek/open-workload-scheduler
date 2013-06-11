/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: router.h
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

#ifndef ROUTER_H
#define ROUTER_H

#include <map>
#include <string>
#include <fstream>

#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/multi_index/member.hpp>
#include <boost/multi_index_container.hpp>
#include <boost/multi_index/ordered_index.hpp>

#include "common.h"
#include "rpc_client.h"
#include "config.h"

// namespace ows {

class Rpc_Client;

/**
 * p_host_keys / m_host_keys
 *
 * Defines the { hostname => public_key } map and pair
 */
typedef std::pair<std::string, std::string>		p_host_key;
typedef	std::map<std::string, std::string>		m_host_keys;
typedef std::pair<m_host_keys::const_iterator, m_host_keys::const_iterator>	p_m_host_keys_iter;

/**
 * mutable_pair
 *
 * Defines the routing table data type :
 * { destination => { weight <=> gateway } }
 */
template <typename T1,typename T2> struct mutable_pair {
	typedef T1 first_type;
	typedef T2 second_type;

	mutable_pair():first(T1()),second(T2()){}
	mutable_pair(const T1& f,const T2& s):first(f),second(s){}
	mutable_pair(const std::pair<T1,T2>& p):first(p.first),second(p.second){}

	T1			first;
	mutable	T2	second;
};

/**
 * p_weighted_gateway
 *
 * Defines the { cost <=> gateway } mutable_pair
 */
typedef mutable_pair<unsigned int, std::string>		p_weighted_gateway;

/**
 * m_weighted_gateway
 *
 * Defines the { p_weighted_gateway <=> node } index
 */
typedef boost::multi_index::multi_index_container<
	p_weighted_gateway,
	boost::multi_index::indexed_by<
		boost::multi_index::ordered_non_unique<
			boost::multi_index::member<
				p_weighted_gateway,unsigned int, &p_weighted_gateway::first
			>
		>,
		boost::multi_index::ordered_non_unique<
			boost::multi_index::member<
				p_weighted_gateway,std::string, &p_weighted_gateway::second
			>
		>
	>
> m_weighted_gateway;

/**
 * m_routing_table / p_routing_table
 *
 * Defines the { node => m_weighted_gateway } map
 */
typedef	std::pair<std::string,m_weighted_gateway>	p_routing_table;
typedef	std::map<std::string,m_weighted_gateway>	m_routing_table;

class Router {
public:
	/**
	 * Router
	 *
	 * Initializes its Rpc_Client
	 * Gets the Config object
	 *
	 * @param	c	Config object
	 */
	Router(Config* c);

	/**
	 * ~Router
	 *
	 * Deletes its Rpc_Client
	 */
	~Router();

	/**
	 * get_node
	 *
	 * Gets the node hosted by the given domain
	 *
	 * @param	domain_name	the hosting domain
	 * @param	node		the output
	 * @param	target_node_name	the node to query
	 */
	bool	get_node(const std::string& domain_name, rpc::t_node& node, const char* target_node_name);

	/**
	 * get_next_hop
	 *
	 * TODO: Not implemented yet
	 */
	char*	get_next_hop(const std::string*);

	/**
	 * update_peers_list
	 *
	 * Gets the hosts_keys file into memory
	 * Says hello() to each peer
	 * Updates the routing table if succeeded
	 *
	 * @param	keys_file	by default /etc/ows/peers.key
	 */
	bool	update_peers_list();

	/**
	 * get_direct_peers
	 *
	 * Gets the peers that are reachable directly
	 *
	 * @return	a pair of const_iterators to the host_keys map (begin and end)
	 */
	p_m_host_keys_iter	get_direct_peers();

	/**
	 * update_routing_table
	 *
	 * TODO: Not implemented yet
	 */
	bool	update_routing_table();

	/**
	 * insert_route
	 *
	 * Inserts a route in the routing table with the given 3-uple
	 *
	 * @param	destination	the target node
	 * @param	gateway		the direct hop
	 * @param	weight		hops' number
	 *
	 * @return	true on success
	 */
	bool	insert_route(const std::string&, const std::string&, const u_int&);

	/**
	 * delete_route
	 *
	 * Deletes a route from the routing table. Three ways are provided :
	 * - remove a gateway for a given destination
	 * - delete a gateway from the whole table (destination == NULL)
	 * - delete a destination (gateway == NULL)
	 *
	 * @param	destination	the node to reach
	 * @param	gateway		the gateway to use
	 *
	 * @return false (table empty or destination not found) or true (success)
	 */
	bool	delete_route(const std::string* destination, const std::string* gateway);

	/**
	 * get_route
	 *
	 * Gets the lighter route to reeach the destination
	 *
	 * @param	destination	the node to reach
	 *
	 * @return	the (gateway, weight) couple
	 */
	p_weighted_gateway*	get_route(const std::string& destination);

	/**
	 * get_gateway
	 *
	 * Gets the lightest gateway to reach the destination
	 *
	 * @param	destination	the node to reach
	 *
	 * @return	the lightest usable gateway
	 */
	std::string*	get_gateway(const std::string& destination);

	/**
	 * get_gateway
	 *
	 * Gets the lightest gateway to reach the destination
	 * Creates a std::string to call get_gateway(const std::string&)
	 *
	 * @param	destination	the node to reach
	 *
	 * @return	the lightest usable gateway
	 */
	std::string*	get_gateway(const char* destination);

	/**
	 * delete_gateway
	 *
	 * Removes a gateway from the routing table
	 *
	 * TODO : Not implemented yet
	 */
	bool	delete_gateway(const std::string&);

	/**
	 * reach_master
	 *
	 * Calls reach_master against the direct peers
	 *
	 * @return	true	the master has been reached
	 */
	bool	reach_master();

	/**
	 * reach_master
	 *
	 * Calls reach_master against the direct peers
	 *
	 * @return	true	the master has been reached
	 */
	bool	reach_master(const char* target);

	/**
	 * set_master_node
	 *
	 * Sets the master node if empty
	 *
	 * @param	node	the node's name
	 *
	 * @return	true	the node has been set
	 */
	bool	set_master_node(const char* node);

	/**
	 * get_master_node
	 *
	 * Gets the master node's name
	 *
	 * @return	its name
	 */
	std::string*	get_master_node();

	/**
	 * get_reachable_peers_number
	 *
	 * Gets hosts_keys's size
	 *
	 * @return	hosts_keys's size
	 */
	u_int	get_reachable_peers_number();

private:

	/**
	 * hosts_keys
	 *
	 * The neighbours' public keys
	 * TODO: use it
	 */
	m_host_keys	hosts_keys;

	/**
	 * routing_table
	 *
	 * The routing table
	 */
	m_routing_table		routing_table;

	/**
	 * config
	 *
	 * The map containing the paramaters read from the .cfg file
	 */
	Config*		config;

	/**
	 * rpc_client
	 *
	 * The RPC interface.
	 * Created by the constructor
	 */
	Rpc_Client*	rpc_client;

	/**
	 * updates_mutex
	 *
	 * TODO: use it
	 */
	boost::mutex	updates_mutex;

	/**
	 * master_node
	 *
	 * The name of the node managing the domain
	 */
	std::string	master_node;
};

// } // namespace ows

#endif // ROUTER_H

