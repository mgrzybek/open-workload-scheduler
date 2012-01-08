/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: shell.cpp
 * Description: contains the main() function of a basic CLI.
 *
 * @author Mathieu Grzybek on 2011-04-26
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

#include "shell.h"

int		main (int argc, char* const argv[]) {
	char*		config_file		= NULL;
	bool		debug_mode		= false;
//	bool		check_mode		= false;
	Config		conf_params;
	v_args		line;

	cli_def		*cli;

	/*
	 * Initialisation
	 *
	 * Read the config file (-f option)
	 */
	if ( argc < 3 ) {
		usage();
		return EXIT_FAILURE;
	}

	for ( int i = 1 ; i < argc ; i++ ) {
		if ( strcmp(argv[i], "-f" ) == 0 && i < argc ) {
			config_file = argv[i+1];
			continue;
		}
		if ( strcmp(argv[i], "-v" ) == 0 ) {
			debug_mode = true;
			continue;
		}
	}

	if ( config_file == NULL ) {
		std::cerr << "No config file given" << std::endl;
		usage();
		return EXIT_FAILURE;
	}

	if ( conf_params.parse_file(config_file) == false ) {
		std::cout << "Cannot parse " << config_file << std::endl;
		return EXIT_FAILURE;
	}

	/*
	 * Let's start the shell stuff
	 */
/*	std::cout << "OWS Shell" << std::endl;
	while (1) {
		line = process_cli("> ");

		if ( line[0].compare("connect") == 0) {
			line.erase(line.begin());
			connect(&line, &conf_params);
		}

		if ( line[0].compare("quit") == 0 )
			break;
	}
*/

	cli = cli_init();
	cli_set_banner(cli, "libcli test environment");
	cli_set_hostname(cli, "router");
	cli_regular(cli, regular_callback);
	cli_regular_interval(cli, 5); // Defaults to 1 second
	cli_set_idle_timeout_callback(cli, 60, idle_timeout); // 60 second idle timeout


	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * process_cli
 *
 * Reads the input and splits it
 * It splits the input :
 * - 0 -> the command
 * - >0 -> the arguments
 *
 * @arg		:	the commands map
 * @return	:	array of args
 */
v_args	process_cli(const char* prompt) {
	std::string	input;
	v_args		split_result;

	std::cout << prompt;
	if ( not getline(std::cin, input).eof() )
		boost::split(split_result, input, boost::is_any_of(" "));
	else
		split_result.push_back("quit");

	return split_result;
}

/*
 * get_params
 */
void	get_params(m_param* params) {
	std::string	input;
	m_param::iterator	it;

	it = params->begin();
	while (it != params->end()) {
		std::cout << it->first << ":";
		getline(std::cin, it->second);
		std::cout << "input is:" << it->second << std::endl;
		it++;
	}
/*
	BOOST_FOREACH(m_param::value_type couple, *params) {
		std::cout << couple.first << ": ";
		getline(std::cin, input);
		std::cout << "input is:" << couple.second << std::endl;
	}
*/
}

/*
 * build_v_jobs_from_string
 */
rpc::v_job_ids	build_v_jobs_from_string(const std::string* input) {
	v_args		split_result;
	rpc::v_job_ids	jobs;

	if ( input->empty() == true )
		return jobs;

	boost::split(split_result, *input, boost::is_any_of(","));

	BOOST_FOREACH(std::string j, split_result) {
		jobs.push_back(boost::lexical_cast<int>(j));
	}

	return jobs;
}

/*
 * usage
 *
 * Prints the usage page on stdout
 *
 */
void	usage() {
	std::cout << "Usage: shell -f <config_file> [ -c || -v ]" << std::endl;
	std::cout << "	<config_file>	: the main configuration file" << std::endl;
	std::cout << "	-c		: check the configuration and exit" << std::endl;
	std::cout << "	-v		: verbose mode" << std::endl;
}

/*
 * quit
 */
void	quit() {
	exit(EXIT_SUCCESS);
}

/*
 * help
 */
bool	help(const v_args* args) {
	std::cout << "Help :" << std::endl;
	return true;
}

/*
 * sql
 */
bool	sql(Rpc_Client* client, const std::string* hostname) {
	std::string	input;

	while (1) {
		std::cout << "sql> ";
		getline(std::cin, input);
		if ( input.compare("exit") == 0 or input.length() == 0 )
			break;
		client->get_client()->sql_exec(input);
	}
	return true;
}

/*
 * hello
 */
bool	hello(Rpc_Client* client, const char* hostname) {
	rpc::t_hello	hello_result;
	rpc::t_node		node;

	node.name = hostname;

	try {
		client->get_client()->hello(hello_result, node);
	} catch (const rpc::e_routing e) {
		std::cerr << "e_routing: " << e.msg << std::endl;
		return false;
	} catch (const std::exception e) {
		std::cerr << "exception: " << e.what() << std::endl;
		return false;
	}

	if ( hello_result.name.empty() == true and hello_result.domain.empty() == true) {
		std::cerr << "RPC call failed" << std::endl;
		return false;
	}

	std::cout << "domain: " << hello_result.domain << std::endl;
	std::cout << "name: " << hello_result.name << std::endl;
	std::cout << "master: ";

	if ( hello_result.is_master == true )
		std::cout << "yes";
	else
		std::cout << "no";

	std::cout << std::endl;

	return true;
}

/*
 * print_jobs
 */
void	print_jobs(const rpc::v_jobs& jobs) {
	BOOST_FOREACH(rpc::t_job j, jobs) {
		std::cout
		<< "id: " << j.id << std::endl
		<< "name: " << j.name << std::endl
		<< "node_name: " << j.node_name << std::endl
		<< "domain: " << j.domain << std::endl
		<< "cmd_line: " << j.cmd_line << std::endl
		<< "weight: " << j.weight << std::endl;

		// TODO: print the prv et nxt job_ids

		std::cout
		<< "start_time: " << j.start_time << std::endl
		<< "stop_time: " << j.stop_time << std::endl
		<< "return_code: " << j.return_code << std::endl
		<< std::endl;
	}
}

/*
 * get_jobs
 */
bool	get_jobs(Rpc_Client* client, const std::string& running_node) {
	rpc::v_jobs	result;

	try {
		client->get_client()->get_jobs(result, running_node);
	} catch (const rpc::e_job e) {
		std::cout << "Error: " << e.msg << std::endl;
		return false;
	}

	print_jobs(result);

	return true;
}

/*
 * add_job
 */
bool	add_job(Rpc_Client* client, const std::string* hostname, const std::string* domain_name, const std::string* name, const std::string* node_name, std::string* cmd_line, int weight, rpc::v_job_ids& pj, rpc::v_job_ids& nj ) {
	rpc::t_job	job;

	job.name = name->c_str();
	job.node_name = node_name->c_str();
	job.cmd_line = cmd_line->c_str();
	job.weight = weight;
	job.prv = pj;
	job.nxt = nj;

	try {
		if ( client->get_client()->add_job(job) == false ) {
			std::cerr << "RPC call failed" << std::endl;
			return false;
		} else
			std::cout << "Command succeded" << std::endl;
	} catch (const rpc::e_job e) {
		std::cerr << "RPC call failed: " << e.msg << std::endl;
		return false;
	}

	return true;
}

/*
 * connect
 *
 */
bool	connect(const v_args* args, Config* conf_params) {
	v_args	split_result;
	v_args	line;
	m_param	params;

	std::string	prompt;
	const char*	hostname = NULL;
	int		port;

	Router		router(conf_params);
	Rpc_Client	client(conf_params, &router);

//	if ( router.update_peers_list() == false )
//		return false;

	/*
	 * case 1: split_result = hostname
	 *	-> port = args[1] if not empty or default port
	 * case 2: split_result = hostname, port
	 *	-> use [0] and [1]
	 */
	boost::split(split_result, args->front(), boost::is_any_of(": "));
	if ( split_result.size() == 1 ) {
		hostname = split_result[0].c_str();
		if ( args->size() == 2 )
			port = boost::lexical_cast<int>(args->at(1));
		else
			port = boost::lexical_cast<int>(*conf_params->get_param("bind_port"));
	} else {
		hostname = args->front().c_str();
		port = boost::lexical_cast<int>(split_result[1]);
	}

	if ( client.open(hostname, port) == false ) {
		std::cout << "Cannot connect" << std::endl;
		return false;
	}

	prompt = hostname;
	prompt += ">" ;

	// creates the cli
	while (1) {
		line = process_cli(prompt.c_str());
		if ( line[0].compare("hello") == 0 ) {
			hello(&client, hostname);
			continue;
		}
		if ( line[0].compare("add_job") == 0 ) {
			params["domain_name"]	= "";
			params["name"]			= "";
			params["node_name"]		= "";
			params["cmd_line"]		= "";
			params["weight"]		= "";
			params["pj"]			= "";
			params["nj"]			= "";

			get_params(&params);

			rpc::v_job_ids	pj	= build_v_jobs_from_string(&params["pj"]);
			rpc::v_job_ids	nj	= build_v_jobs_from_string(&params["nj"]);
			int		weight	= boost::lexical_cast<int>(params["weight"]);

			add_job(&client, &args->at(0), &params["domain_name"], &params["name"], &params["node_name"], &params["cmd_line"], weight, pj, nj);
			continue;
		}
		if ( line[0].compare("get_jobs") == 0 ) {
			get_jobs(&client, hostname);
		}
		if ( line[0].compare("quit") == 0 or line[0].compare("exit") == 0 )
			break;
		if ( line[0].compare("sql") == 0 ) {
			sql(&client, &args->at(0));
		}
	}


	return true;
}
