/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: config.h
 * Description: used to parse the config files and give parameters to objects.
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

#ifndef CONFIG_H
#define CONFIG_H

#include <fstream>
#include <map>
#include <string>
#include <iostream>

#include <boost/thread/mutex.hpp>
#include <boost/regex.hpp>

#include "common.h"

/*
 * m_config
 *
 * Defines the { key => value } map
 */
typedef std::map<std::string,std::string> m_config;

/*
 * m_syntax_regex
 *
 * Defines the { key => regexp } map
 * The associated regex is used to check the key's value
 */
typedef std::map<std::string,boost::regex> m_syntax_regex;

//namespace ows {

class Config {
public:
	Config();
	~Config();

	/*
	 * parse_file
	 *
	 * @arg : the file
	 * @return : true (sucess) / false (failure)
	 *
	 * Comments use "#". They can be at the beginning of the line or after
	 * a key=value couple
	 * Spaces and tabulations are deleted
	 */
	bool			parse_file(const char*);

	/*
	 * get_param
	 *
	 * @arg : the key
	 * @return : the value or NULL
	 *
	 * Gets the value associated to a key
	 */
	std::string*	get_param(const char*);

private:
	/*
	 * check_syntax
	 *
	 * @arg : key
	 * @arg : value
	 * @return : true (yes) / false (no)
	 *
	 * Checks if the stored key matches its regexp
	 */
	bool			check_syntax(const char* key, const char* value);

	/*
	 * config_options
	 *
	 * The config file's content
	 */
	m_config		config_options;

	/*
	 * syntex_regex
	 *
	 * TODO: implement it
	 * The matching regex of each paramater
	 */
	m_syntax_regex	syntax_regex;

	/*
	 * master_node
	 *
	 * The name of the node managing the domain
	 */
	std::string		master_node;
};

//} // namespace ows

#endif // CONFIG_H
