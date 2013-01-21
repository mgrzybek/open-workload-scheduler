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

#ifdef USE_THRIFT
// RPC Stuff
#include "ows_rpc.h"

// Common Stuff
#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>
#endif //USE_THRIFT

// namespace ows {

class Rpc_Client {
public:
	/**
	 * Rpc_Client
	 *
	 * The constructor
	 */
	Rpc_Client();

	/**
	 * Rpc_Client
	 *
	 * The destructor
	 */
	~Rpc_Client();

#ifdef USE_THRIFT

	/**
	 * open
	 *
	 * Open a connection to a remote noode
	 *
	 * @param	hostname	target's name
	 * @param	port		the TCP port to use
	 *
	 * @return	true		success
	 */
	bool	open(const char* hostname, const int& port);

	/**
	 * get_handler
	 *
	 * Gets the client handler
	 *
	 * @return	the Rpc_Client object
	 */
	rpc::ows_rpcClient*	get_handler() const;

	/**
	 * close
	 *
	 * Closes the connection
	 *
	 * @return	true on success
	 */
	bool	close();

	/**
	 * transport
	 *
	 * Thrift's connection handler
	 */
	boost::shared_ptr<apache::thrift::transport::TTransport>	transport;

	/**
	 * handler
	 *
	 * Thrift's interface
	 */
	rpc::ows_rpcClient*	handler;

#endif // USE_THRIFT

	/**
	 * build_url
	 *
	 * Creates a complete URL :
	 * "http://" + target + ":" port + "/" + rpc_path
	 *
	 * @param	target	the target hostname
	 *
	 * @return	the complete URL
	 */
	std::string*	build_url(const std::string* target);
};

// } // namespace ows

#endif // RPC_CLIENT_H
