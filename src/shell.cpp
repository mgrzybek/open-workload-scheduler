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
//	Config		conf_params;
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

	cli = cli_init();
	cli_set_banner(cli, "libcli test environment");
	cli_set_hostname(cli, "ows");
//	cli_regular(cli, regular_callback);
	cli_regular_interval(cli, 5); // Defaults to 1 second
	cli_set_idle_timeout_callback(cli, 60, idle_timeout); // 60 second idle timeout

	// Connect command
	cli_register_command(cli, NULL, "connect", cmd_connect, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Connects to the given hostname");

//	cli_loop(cli, 1);
//	cli_done(cli);

	int s, x;
	int on = 1;
	struct sockaddr_in addr;

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return 1;
	}
	setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);
	addr.sin_port = htons(8023);

	if (bind(s, (struct sockaddr *) &addr, sizeof(addr)) < 0) {
		perror("bind");
		return 1;
	}

    if (listen(s, 50) < 0) {
		perror("listen");
		return 1;
	}

    printf("Listening on port %d\n", 8023);
    while ((x = accept(s, NULL, 0))) {
		cli_loop(cli, x);
		shutdown(x, 2);
		close(x);
	}

	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

int idle_timeout(struct cli_def *cli) {
    cli_print(cli, "Custom idle timeout");
    return CLI_QUIT;
}

///////////////////////////////////////////////////////////////////////////////
/*
int regular_callback(struct cli_def *cli) {
	cli_print(cli, "Regular callback - %u times so far", regular_count);
	cli_reprompt(cli);
    return CLI_OK;
}
*/
///////////////////////////////////////////////////////////////////////////////

/*
 * get_params
 */
bool		get_params(m_param* params, int argc, char* argv[]) {
	uint				counter = 0;
	std::string			input;
	v_args				split_result;
	m_param::iterator	it;

	if ( (uint)argc < params->size() )
		return false;

	for ( int i = 0 ; i < argc ; i++) {
		boost::split(split_result, argv[i], boost::is_any_of("="));

		it = params->begin();
		it = params->find(split_result.at(0));

		if ( it != params->end() ) {
			it->second = split_result[1];
			counter++;
		}
	}

	if ( counter < params->size() )
		return false;

	return true;
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
	std::cout
		<< "Usage: shell -f <config_file> [ -c || -v ]" << std::endl
		<< "	<config_file>	: the main configuration file" << std::endl
		<< "	-c		: check the configuration and exit" << std::endl
		<< "	-v		: verbose mode" << std::endl;
}

/*
 * build_string_from_job_state
 */
std::string	build_string_from_job_state(const rpc::e_job_state::type js) {
	std::string result;

	switch (js) {
		case rpc::e_job_state::WAITING: {
			result = "waiting";
			break;
		}
		case rpc::e_job_state::RUNNING: {
			result = "running";
			break;
		}
		case rpc::e_job_state::SUCCEDED: {
			result = "succeded";
			break;
		}
		case rpc::e_job_state::FAILED: {
			result = "failed";
			break;
		}
	}

	return result;
}

/*
 * build_job_state_from_string
 */
rpc::e_job_state::type	build_job_state_from_string(const char* state) {
	if ( strcmp(state, "waiting") == 0 )
		return rpc::e_job_state::WAITING;
	if ( strcmp(state, "running") == 0 )
		return rpc::e_job_state::RUNNING;
	if ( strcmp(state, "succeded") == 0 )
		return rpc::e_job_state::SUCCEDED;
	if ( strcmp(state, "failed") == 0 )
		return rpc::e_job_state::FAILED;

	throw "Error: string state is not related to a job's state";
	return rpc::e_job_state::FAILED;
}

/*
 * print_jobs
 */
void	print_jobs(struct cli_def* cli, const rpc::v_jobs& jobs) {
	BOOST_FOREACH(rpc::t_job j, jobs) {
		cli_print(cli,
				"id: %u\nname: %s\nnode_name: %s\ndomain: %s\ncmd_line: %s\nweight: %u",
				j.id,
				j.name.c_str(),
				j.node_name.c_str(),
				j.domain.c_str(),
				j.cmd_line.c_str(),
				j.weight);

		// TODO: print the prv et nxt job_ids

		cli_print(cli,
				  "status: %s\nstart_time: %lld\nstop_time: %lld\nreturn_code: %u",
				  build_string_from_job_state(j.state).c_str(),
				  j.start_time,
				  j.stop_time,
				  j.return_code);

		cli_print(cli, "=================================");
	}
}

/*
 * get_jobs
 */
int	cmd_get_jobs(struct cli_def *cli, const char *command, char *argv[], int argc) {
	rpc::v_jobs	result;
	std::string	target;

	if ( argc == 0 )
		target = connected_node_name;
	else if ( argc == 1 )
		target = argv[0];
	else {
		cli_print(cli, "Error: missing one arg : running_node");
		return CLI_ERROR_ARG;
	}

	try {
		client.get_client()->get_jobs(result, target.c_str());
	} catch (const rpc::e_job e) {
		cli_print(cli, "Error: %s", e.msg.c_str());
		return CLI_ERROR;
	}

	print_jobs(cli, result);

	return CLI_OK;
}

/*
 * cmd_get_ready_jobs
 */
int		cmd_get_ready_jobs(struct cli_def *cli, const char *command, char *argv[], int argc) {
	rpc::v_jobs	result;
	std::string	target;

	if ( argc == 0 )
		target = connected_node_name;
	else if ( argc == 1 )
		target = argv[0];
	else {
		cli_print(cli, "Error: missing one arg : running_node");
		return CLI_ERROR_ARG;
	}

	if ( argc != 1 ) {
		cli_print(cli, "Error: missing one arg : running_node");
		return CLI_ERROR_ARG;
	}

	try {
		client.get_client()->get_ready_jobs(result, target);
	} catch (const rpc::e_job e) {
		cli_print(cli, "Error: %s", e.msg.c_str());
		return CLI_ERROR;
	}

	print_jobs(cli, result);

	return CLI_OK;
}

/*
 * cmd_add_job
 */
int	cmd_add_job(struct cli_def *cli, const char *command, char *argv[], int argc) {
	m_param	params;
	rpc::t_job	job;

	// TODO: ask attributes to create a job
	if ( argc == 0 ) {
		cli_print(cli, "Not implemented yet");
		return CLI_ERROR;
	}

	params["domain_name"]	= "";
	params["name"]			= "";
	params["node_name"]		= "";
	params["cmd_line"]		= "";
	params["weight"]		= "";
	params["pj"]			= "";
	params["nj"]			= "";

	if ( get_params(&params, argc, argv) == false ) {
		cli_print(cli, "Bad parameters");
		return CLI_ERROR;
	}

	job.name = params["name"].c_str();
	job.node_name = params["node_name"].c_str();
	job.cmd_line = params["cmd_line"].c_str();
	job.weight = boost::lexical_cast<int>(params["weight"].c_str());

	rpc::v_job_ids	pj	= build_v_jobs_from_string(&params["pj"]);
	rpc::v_job_ids	nj	= build_v_jobs_from_string(&params["nj"]);

	job.prv = pj;
	job.nxt = nj;

	try {
		if ( client.get_client()->add_job(job) == false ) {
			cli_print(cli, "RPC call failed");
			return CLI_ERROR;
		} else
			cli_print(cli, "Command succeded");
	} catch (const rpc::e_job e) {
		cli_print(cli, "%s", e.msg.c_str());
		return CLI_ERROR;
	}

	return CLI_OK;
}

/*
 * cmd_remove_job
 */
int		cmd_remove_job(struct cli_def *cli, const char *command, char *argv[], int argc) {
	m_param		params;
	rpc::t_job	job;

	// TODO: ask attributes to create a job
	if ( argc == 0 ) {
		cli_print(cli, "Not implemented yet");
		return CLI_ERROR;
	}

	params["id"] = "";
	params["node_name"] = "";

	if ( get_params(&params, argc, argv) == false ) {
		cli_print(cli, "Bad parameters");
		return CLI_ERROR;
	}

	job.id = boost::lexical_cast<int>(params["id"]);
	job.node_name = params["node_name"];

	try {
		if ( client.get_client()->remove_job(job) == false ) {
			cli_print(cli, "RPC call failed");
			return CLI_ERROR;
		} else
			cli_print(cli, "Command succeded");
	} catch (const rpc::e_job e) {
		cli_print(cli, "%s", e.msg.c_str());
		return CLI_ERROR;
	}

	return CLI_OK;
}

/*
 * cmd_update_job_state
 */
int		cmd_update_job_state(struct cli_def *cli, const char *command, char *argv[], int argc) {
	m_param		params;
	rpc::t_job	job;

	// TODO: ask attributes to create a job
	if ( argc == 0 ) {
		cli_print(cli, "Not implemented yet");
		return CLI_ERROR;
	}

	params["id"] = "";
	params["node_name"] = "";
	params["state"] = "";

	if ( get_params(&params, argc, argv) == false ) {
		cli_print(cli, "Bad parameters");
		return CLI_ERROR;
	}

	job.id = boost::lexical_cast<int>(params["id"]);
	job.node_name = params["node_name"];

	try {
		if ( client.get_client()->update_job_state(job, build_job_state_from_string(params["state"].c_str())) == false ) {
			cli_print(cli, "RPC call failed");
			return CLI_ERROR;
		} else
			cli_print(cli, "Command succeded");
	} catch (const rpc::e_job e) {
		cli_print(cli, "%s", e.msg.c_str());
		return CLI_ERROR;
	} catch (const apache::thrift::transport::TTransportException e ) {
		cli_print(cli, "%s", e.what());
		return CLI_ERROR;
	}

	return CLI_OK;
}

/*
 * hello
 */
int		cmd_hello(struct cli_def *cli, const char *command, char *argv[], int argc) {
	rpc::t_hello	hello_result;
	rpc::t_node		node;

	if ( argc == 0 )
		node.name = connected_node_name.c_str();
	else
		node.name = argv[0];

	try {
		client.get_client()->hello(hello_result, node);
	} catch (const rpc::e_routing e) {
		cli_print(cli, "e_routing: %s", e.msg.c_str());
		return CLI_ERROR;
	} catch (const std::exception e) {
		cli_print(cli, "exception: %s", e.what());
		return CLI_ERROR;
	}

	if ( hello_result.name.empty() == true and hello_result.domain.empty() == true) {
		cli_print(cli, "RPC call failed");
		return CLI_ERROR;
	}

	if ( hello_result.is_master == true )
		cli_print(cli, "domain: %s\nname: %s\nmaster: yes", hello_result.domain.c_str(), hello_result.name.c_str());
	else
		cli_print(cli, "domain: %s\nname: %s\nmaster: no", hello_result.domain.c_str(), hello_result.name.c_str());

	return CLI_OK;
}

/*
 * connect
 *
 */
int		cmd_connect(struct cli_def *cli, const char *command, char *argv[], int argc) {
	v_args	split_result;
	v_args	line;
	m_param	params;

	std::string	prompt;
	std::string	hostname;
	int			port;

//	Router		router(&conf_params);
//	Rpc_Client	client(&conf_params, &router);

//	if ( router.update_peers_list() == false )
//		return false;

	if ( argc < 1 ) {
		cli_print(cli, "Missing hostname");
		return CLI_ERROR_ARG;
	}

	boost::split(split_result, argv[0], boost::is_any_of(": "));

	/*
	 * case 1: split_result = hostname
	 *	-> port = args[1] if not empty or default port
	 * case 2: split_result = hostname, port
	 *	-> use [0] and [1]
	 */
	if ( argc == 1 ) {
		if ( split_result.size() == 1 ) {
			hostname = argv[0];
			port = boost::lexical_cast<int>(*conf_params.get_param("bind_port"));
		} else {
			hostname = split_result[0];
			port = boost::lexical_cast<int>(split_result[1]);
		}
	} else {
		hostname = argv[0];
		port = boost::lexical_cast<int>(argv[1]);
	}

	if ( client.open(hostname.c_str(), port) == false ) {
		cli_print(cli, "Cannot connect to %s port %u", hostname.c_str(), port);
		return CLI_ERROR;
	}

	// TODO: fix this dirty hack -> use const args in libcli
	char* h = strdup(hostname.c_str());
	cli_set_hostname(cli, h);
	free(h);

	// Routing command
	cli_register_command(cli, NULL, "hello", cmd_hello, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Sends a 'hello' to a node to know its caracteristics");

	// Add commands
	cli_command*	cli_add = cli_register_command(cli, NULL, "add", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);
	cli_register_command(cli, cli_add, "job", cmd_add_job, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Adds a job to the connected node");

	// Remove command
	cli_command*	cli_remove = cli_register_command(cli, NULL, "remove", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);
	cli_register_command(cli, cli_remove, "job", cmd_remove_job, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Removes a job to the connected node (using id and node_name)");

	// Update command
	cli_command*	cli_update = cli_register_command(cli, NULL, "update", NULL, PRIVILEGE_PRIVILEGED, MODE_EXEC, NULL);
	cli_register_command(cli, cli_update, "job_state", cmd_update_job_state, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Updates a job's state to the connected node (using id, node_name and state)");

	// Get commands
	cli_command*	cli_get = cli_register_command(cli, NULL, "get", NULL, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, NULL);
	cli_register_command(cli, cli_get, "jobs", cmd_get_jobs, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Get the jobs of the connected node");
	cli_register_command(cli, cli_get, "ready_jobs", cmd_get_ready_jobs, PRIVILEGE_UNPRIVILEGED, MODE_EXEC, "Gets the 'ready to run' jobs of the connected node");

	// Sets the global node's name
	connected_node_name = hostname;

	// TODO: unregister commands before exiting
	return CLI_OK;
}
