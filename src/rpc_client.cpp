/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: rpc_client.cpp
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

#include "rpc_client.h"

Rpc_Client::Rpc_Client(Config* c, Router* r) {
	this->config	= c;
	this->router	= r;
#ifdef USE_THRIFT
	this->client	= NULL;
#endif // USE_THRIFT
}

Rpc_Client::Rpc_Client() {
#ifdef USE_THRIFT
	this->client	= NULL;
#endif // USE_THRIFT
}

Rpc_Client::~Rpc_Client() {
	this->config	= NULL;
	this->router	= NULL;
#ifdef USE_THRIFT
	if ( this->client != NULL ) {
		delete this->client;
		this->client = NULL;
		if ( this->transport != NULL )
			this->transport->close();

	}
#endif //USE_THRIFT
}

#ifdef USE_THRIFT

bool	Rpc_Client::open(const char* hostname, const int& port) {
	boost::shared_ptr<apache::thrift::transport::TSocket>		socket(new apache::thrift::transport::TSocket(hostname, port));
	boost::shared_ptr<apache::thrift::transport::TTransport>	transport(new apache::thrift::transport::TBufferedTransport(socket));
	boost::shared_ptr<apache::thrift::protocol::TProtocol>		protocol(new apache::thrift::protocol::TBinaryProtocol(transport));

	this->client = new rpc::ows_rpcClient(protocol);

	try {
		transport->open();
	} catch (const apache::thrift::transport::TTransportException e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}

	return true;
}

rpc::ows_rpcClient*	Rpc_Client::get_client() const {
	return this->client;
}

//TODO: find a clean transport->close()
bool    Rpc_Client::close() {
	delete this->client;
	this->client = NULL;
/*
	try {
		this->transport->close();
	} catch (const apache::thrift::transport::TTransportException e) {
		std::cerr << "Error: " << e.what() << std::endl;
		return false;
	}
*/
	return true;
}

#endif // USE_THRIFT
