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

#include <boost/lexical_cast.hpp>
#include <boost/algorithm/string.hpp>

#include <libcli.h>

#include "router.h"
#include "config.h"
#include "rpc_client.h"
#include "thrift/ows_rpc.h"

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

class	Command {
public:
	virtual bool	execute(v_args& argv) = 0;

	std::string&	get_help_msg() const;
	std::string&	get_desc_msg() const;

private:
	std::string		help_msg;
	std::string		desc_msg;
};

///////////////////////////////////////////////////////////////////////////////

int		main (int argc, char* const argv[]);

v_args		process_cli(const char* prompt);
void		get_params(m_param* params);
rpc::v_job_ids	build_v_jobs_from_string(const std::string* input);

void	usage();
void	quit();

int		cmd_connect(const v_args* args, Config* conf_params);
bool	help(const v_args* args);

bool	hello(Rpc_Client* client, const std::string* hostname);

void	print_jobs(const rpc::v_jobs& jobs);
bool	get_jobs(Rpc_Client* client, const std::string& running_node);
bool	add_job(Rpc_Client* client, const std::string* hostname, const std::string* domain_name, const std::string* name, const std::string* node_name, std::string* cmd_line, int weight, v_jobs* pj, v_jobs* nj );

#endif // SHELL_H

