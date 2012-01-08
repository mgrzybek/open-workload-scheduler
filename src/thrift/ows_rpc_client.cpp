#include "ows_rpc.h"  // As an example

#include <transport/TSocket.h>
#include <transport/TBufferTransports.h>
#include <protocol/TBinaryProtocol.h>

#include <iostream>

//using namespace apache::thrift;
//using namespace apache::thrift::protocol;
//using namespace apache::thrift::transport;

//using namespace rpc;

int main(int argc, char **argv) {
  boost::shared_ptr<apache::thrift::transport::TSocket> socket(new apache::thrift::transport::TSocket("localhost", 8080));
  boost::shared_ptr<apache::thrift::transport::TTransport> transport(new apache::thrift::transport::TBufferedTransport(socket));
  boost::shared_ptr<apache::thrift::protocol::TProtocol> protocol(new apache::thrift::protocol::TBinaryProtocol(transport));

 	rpc::ows_rpcClient client(protocol);
	transport->open();
 
	rpc::t_hello	hello_result;
	rpc::t_node	node;
	rpc::v_jobs	jobs_result;

	node.name = "localhost";

	client->hello(hello_result, node);
	std::cout << hello_result.domain.c_str() << std::endl;

	client->get_all_jobs(jobs_result, node.name);
	std::cout << jobs_result.size() << std::endl;

	transport->close();

	return 0;
}

