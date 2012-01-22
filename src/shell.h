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

#include "thrift/ows_rpc.h"
#include "thrift/model_types.h"
#include "router.h"
#include "config.h"
#include "rpc_client.h"

typedef std::vector<std::string>				v_args;

typedef	std::pair<std::string, std::string>		p_param;
typedef	std::map<std::string, std::string>		m_param;

/*
typedef	std::pair<std::string, bool(*)(const v_args*)>	p_cmd_list;
typedef	std::map<std::string, bool(*)(const v_args*)>		m_cmd_list;

typedef	std::pair<std::string, bool(*)(Rpc_Client*, const v_args*)>	p_cnt_list;
typedef	std::map<std::string, bool(*)(Rpc_Client*, const v_args*)>		m_cnt_list;
*/

///////////////////////////////////////////////////////////////////////////////

Config		conf_params;
Rpc_Client	client;
std::string	connected_node_name;

int		cmd_connect(struct cli_def *cli, const char *command, char *argv[], int argc);
int		cmd_hello(struct cli_def *cli, const char *command, char *argv[], int argc);

int		cmd_add_job(struct cli_def *cli, const char *command, char *argv[], int argc);
int		cmd_remove_job(struct cli_def *cli, const char *command, char *argv[], int argc);
int		cmd_update_job_state(struct cli_def *cli, const char *command, char *argv[], int argc);

int		cmd_get_jobs(struct cli_def *cli, const char *command, char *argv[], int argc);
int		cmd_get_ready_jobs(struct cli_def *cli, const char *command, char *argv[], int argc);

int		idle_timeout(struct cli_def *cli);
int		regular_callback(struct cli_def *cli);

///////////////////////////////////////////////////////////////////////////////

int		main (int argc, char* const argv[]);

bool	get_params(m_param* params, int argc, char* argv[]);


void	usage();
void	print_jobs(struct cli_def* cli, const rpc::v_jobs& jobs);

rpc::v_job_ids	build_v_jobs_from_string(const std::string* input);

#endif // SHELL_H
