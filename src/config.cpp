/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: config.cpp
 * Description: used to parse the config files and give parameters to objects
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

// A Thrift header has the same name
#include "include/config.h"

///////////////////////////////////////////////////////////////////////////////

Config::Config() {
	// Model : this->syntax_regex.insert(std::pair<std::string, boost::regex>("", boost::regex("", boost::regex::perl)));
	this->syntax_regex.insert(std::pair<std::string, boost::regex>("bind_port", boost::regex("^[0-9]{2,}$", boost::regex::perl)));
	//	this->syntax_regex.insert(std::pair<std::string, boost::regex>("bind_address", boost::regex("^([0-9]+)([[.period.]][0-9]+){3}$", boost::regex::perl)));
	//	this->syntax_regex.insert(std::pair<std::string, boost::regex>("node_name", boost::regex('[\w]', boost::regex::perl)));
}

Config::~Config() {
	this->config_options.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool	Config::parse_file(const char* file_path) {
	std::ifstream	f(file_path, std::ifstream::in);

	std::string	line;
	std::string	key;
	std::string	value;

	boost::regex	spaces("[[:space:]]+", boost::regex::perl);
	boost::regex	comment("^#.*?$", boost::regex::perl);
	boost::regex	comment_endl("#.*?$", boost::regex::perl);

	size_t		position = 0;

	if ( f.is_open() )
		while ( ! f.eof() ) {
			getline(f, line);
			line = boost::regex_replace(line, spaces, "");
			line = boost::regex_replace(line, comment_endl, "");

			if ( boost::regex_match(line, comment) == true or line.length() == 0 )
				continue;

			position = line.find_first_of("=");

			if ( position == std::string::npos ) {
				f.close();
				return false;
			}

			key	= line.substr(0, position);
			value	= line.substr(position+1, line.length());

			if ( key.length() == 0 or value.length() == 0 ) {
				f.close();
				return false;
			}

			if ( this->check_syntax(key.c_str(), value.c_str()) == false ) {
				f.close();
				return false;
			}

			this->config_options.insert(std::pair<std::string, std::string>(key, value));
		}
	else
		return false;

	std::map<std::string,std::string>::iterator it;
	std::cout << "config contains:" << std::endl;
	for ( it = this->config_options.begin() ; it != this->config_options.end(); it++ )
		std::cout << (*it).first << " => " << (*it).second << std::endl;

	f.close();

	return this->set_private_attributes();
}

///////////////////////////////////////////////////////////////////////////////

std::string*	Config::get_param(const char* field) {
	m_config::iterator	it;

	it = this->config_options.find(field);

	if ( it == this->config_options.end() )
		return NULL;

	return &it->second;
}

///////////////////////////////////////////////////////////////////////////////

const std::string*	Config::get_master_node() {
	return &this->master_node;
}

///////////////////////////////////////////////////////////////////////////////

e_running_mode	Config::get_running_mode() {
	return this->running_mode;
}

///////////////////////////////////////////////////////////////////////////////

bool	Config::check_syntax(const char* key, const char* value) {
	m_syntax_regex::iterator	it;

	if ( key == NULL or value == NULL )
		return false;

	it = this->syntax_regex.find(key);

	if ( it == this->syntax_regex.end() )
		return true;

	if ( boost::regex_match(value, it->second) == true )
		return true;
	else
		std::cerr << "Bad syntax for " << key << "=" << value << " which should match " << it->second << std::endl;

	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Config::set_private_attributes() {
	if ( this->get_param("running_mode")->compare("active") == 0 )
		this->set_running_mode(ACTIVE);
	else if ( this->get_param("running_mode")->compare("passive") == 0 )
		this->set_running_mode(PASSIVE);
	else if ( this->get_param("running_mode")->compare("p2p") == 0 )
		this->set_running_mode(P2P);
	else {
		std::cerr << "Error: bad running_mode" << std::endl;
		return false;
	}

	return true;
}

///////////////////////////////////////////////////////////////////////////////

void	Config::set_running_mode(const e_running_mode rm) {
	this->running_mode = rm;
}
