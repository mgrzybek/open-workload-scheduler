/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: job.cpp
 * Description: describes a Job object, ie. what to run on the nodes.
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

#include "job.h"

///////////////////////////////////////////////////////////////////////////////

Job::Job(Domain* d, const std::string& name, const std::string& node_name, const std::string& cmd_line, const int& weight, const v_job_ids& pj, const v_job_ids& nj) {
	if ( d == NULL )
		throw "Empty domain";
	if ( name.empty() == true )
		throw "Empty name";
	if ( node_name.empty() == true )
		throw "Empty node_name";
	if ( cmd_line.empty() == true )
		throw "Empty command line";

	this->domain	= d;
	this->name.assign(name.c_str());
	this->node_name.assign(node_name.c_str());
	this->cmd_line.assign(cmd_line.c_str());

	this->weight	= weight;

	this->prev = pj;
	this->next = nj;
}

///////////////////////////////////////////////////////////////////////////////

Job::Job(Domain* d, int id, std::string& name, std::string& node_name, std::string& cmd_line, int weight) {
	if ( d == NULL )
		throw "Empty domain";
	if ( name.empty() == true )
		throw "Empty name";
	if ( node_name.empty() == true )
		throw "Empty node_name";
	if ( cmd_line.empty() == true )
		throw "Empty command line";

	this->id		= id;
	this->domain	= d;
	this->name		= name.c_str();
	this->node_name	= node_name.c_str();
	this->cmd_line	= cmd_line.c_str();

	this->weight	= weight;
}

///////////////////////////////////////////////////////////////////////////////
/*
Job::Job(rpc::t_job& j) {
	this->start_time	= j.start_time;
	this->stop_time		= j.stop_time;
	this->return_code	= j.return_code;

	this->id	= j.id;
	this->name	= j.name.c_str();
	this->node_name	= j.node_name.c_str();

	// domain

	this->cmd_line	= j.cmd_line.c_str();
	this->weight	= j.weight;

	this->prev		= j.prv;
	this->next		= j.nxt;
}
*/
///////////////////////////////////////////////////////////////////////////////

Job::~Job() {
//	delete this->start_time;
//	delete this->stop_time;

	this->prev.clear();
	this->next.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool	Job::run() {
	if ( this->update_state(rpc::e_job_state::RUNNING) == false ) {
		std::cerr << "Error: cannot update job's state to RUNNING" << std::endl;
		return false;
	}

	try {
		this->start_time	= time(NULL);
		this->return_code	= system(this->cmd_line.c_str());
		this->stop_time		= time(NULL);
	} catch (const std::exception e) {
		std::cerr << "Exception: cannot execute " << this->name << " : " << e.what() << std::endl;
		return false;
	}

	// TODO: remove it
	std::cout << "Job " << this->name.c_str() << " returned code " << this->return_code << std::endl;

	try {
		if ( this->return_code == 0 ) {
			if ( this->update_state(rpc::e_job_state::SUCCEDED, this->start_time, this->stop_time) == false ) {
				std::cerr << "Error: cannot update job's state to SUCCEDED" << std::endl;
				return false;
			}
			return true;
		}

		if ( this->update_state(rpc::e_job_state::FAILED, this->start_time, this->stop_time) == false ) {
			std::cerr << "Error: cannot update job's state to FAILED" << std::endl;
			return false;
		}
	} catch (const std::exception e) {
		std::cerr << "Exception: cannot update job's state " << this->name << " : " << e.what() << std::endl;
		return false;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Job::update_state(const rpc::e_job_state::type js) {
	return this->domain->update_job_state(this, js);
}

///////////////////////////////////////////////////////////////////////////////

// TODO: fix it
bool	Job::update_state(const rpc::e_job_state::type js, time_t start_time, time_t stop_time) {
	return this->domain->update_job_state(this, js, start_time, stop_time);
}

///////////////////////////////////////////////////////////////////////////////

const int	Job::get_id() const {
	return this->id;
}

///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_name() const {
	return this->name;
}

///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_cmd_line() const {
	return this->cmd_line;
}

///////////////////////////////////////////////////////////////////////////////

const char*	Job::get_node_name() const {
	return this->node_name.c_str();
}
///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_node_name2() const {
	return this->node_name;
}

///////////////////////////////////////////////////////////////////////////////

const int	Job::get_weight() const {
	return this->weight;
}

///////////////////////////////////////////////////////////////////////////////

const Domain*	Job::get_domain() const {
	return this->domain;
}

///////////////////////////////////////////////////////////////////////////////

const v_job_ids	Job::get_next() const {
	return this->next;
}

///////////////////////////////////////////////////////////////////////////////

const v_job_ids	Job::get_prev() const {
	return this->prev;
}

///////////////////////////////////////////////////////////////////////////////

void		Job::set_id(const int i) {
	this->id = i;
}
