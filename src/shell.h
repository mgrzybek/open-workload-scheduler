/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: shell.cpp
 * Description: used to define the CLI.
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

#ifndef SHELL_H
#define SHELL_H

#include <iostream>
#include <string.h>
#include <map>

#include <netdb.h>
#include <netinet/in.h>
#include <unistd.h>
#include <limits>
#include <iostream>
#include <cstdlib>
#include <cstring>

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <libcli.h>

#include "gen-cpp/ows_rpc.h"
#include "gen-cpp/model_types.h"
#include "router.h"
#include "config.h"
#include "rpc_client.h"

///////////////////////////////////////////////////////////////////////////////

/*
 * v_args
 *
 * Define the key => value vector
 */
typedef std::vector<std::string>				v_args;

/*
 * p_param
 *
 * Defines the { param => value } pair
 */
typedef	std::pair<std::string, std::string>		p_param;

/*
 * m_param
 *
 * Define the { param => value } map
 */
typedef	std::map<std::string, std::string>		m_param;

///////////////////////////////////////////////////////////////////////////////

/*
 * For instance the shell is a telnet server. We need to make the cli_loop
 * function use the local input file descriptors.
 * The program is not thread-safe and must be used by only one client.
 *
 * TODO: use the command line
 */

/*
 * conf_params
 *
 * This object contains the parameters given by the configuration file
 */
Config		conf_params;

/*
 * client
 *
 * This object represents the connection to the peer
 */
Rpc_Client	client;

/*
 * connected_node_name
 *
 * This is the name of the
 */
std::string	connected_node_name;

///////////////////////////////////////////////////////////////////////////////

/*
 * cmd_connect
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_connect(struct cli_def *cli, const char *command, char *argv[], int argc);

/*
 * cmd_hello
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_hello(struct cli_def *cli, const char *command, char *argv[], int argc);

///////////////////////////////////////////////////////////////////////////////

/*
 * cmd_add_job
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_add_job(struct cli_def *cli, const char *command, char *argv[], int argc);

/*
 * cmd_remove_job
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_remove_job(struct cli_def *cli, const char *command, char *argv[], int argc);

/*
 * cmd_update_job_state
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_update_job_state(struct cli_def *cli, const char *command, char *argv[], int argc);

///////////////////////////////////////////////////////////////////////////////

/*
 * cmd_get_jobs
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_get_jobs(struct cli_def *cli, const char *command, char *argv[], int argc);

/*
 * cmd_get_ready_jobs
 *
 * @arg	: the cli object
 * @arg	: the name of the called command
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		cmd_get_ready_jobs(struct cli_def *cli, const char *command, char *argv[], int argc);

///////////////////////////////////////////////////////////////////////////////

/*
 * idle_timeout
 *
 * @arg	: the cli object
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		idle_timeout(struct cli_def *cli);

/*
 * regular_callback
 *
 * @arg	: the cli object
 * @return	: CLI_OK | CLI_ERROR | CLI_QUIT | CLI_ERROR_ARG
 */
int		regular_callback(struct cli_def *cli);

///////////////////////////////////////////////////////////////////////////////

/*
 * main
 *
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: EXIT_SUCCESS or EXIT_FAILURE
 */
int		main(int argc, char* const argv[]);

/*
 * get_params
 *
 * @arg	: the map to update
 * @arg	: the arguments
 * @arg	: the number of arguments
 * @return	: success (true) or failure (false)
 */
bool	get_params(m_param* params, int argc, char* argv[]);

/*
 * usage
 *
 * @arg	: none
 * @return	: none
 */
void	usage();

/*
 * print_jobs
 *
 * @arg	: the cli object
 * @return	: none
 */
void	print_jobs(struct cli_def* cli, const rpc::v_jobs& jobs);

/*
 * build_v_jobs_from_string
 *
 * @arg	: the string to convert
 * @return	: the jobs'id vector
 */
rpc::v_job_ids	build_v_jobs_from_string(const std::string* input);

/*
 * build_string_from_job_state
 *
 * @arg	: the job's state
 * @return	: the converted state
 */
std::string	build_string_from_job_state(const rpc::e_job_state::type js);

#endif // SHELL_H
