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
/*
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
	this->job.name.assign(name.c_str());
	this->job.node_name.assign(node_name.c_str());
	this->job.cmd_line.assign(cmd_line.c_str());

	this->job.weight	= weight;

	this->job.prv = pj;
	this->job.nxt = nj;
}
*/
///////////////////////////////////////////////////////////////////////////////
/*
Job::Job(Domain* d, int id, std::string& name, std::string& node_name, std::string& cmd_line, int weight) {
	if ( d == NULL )
		throw "Empty domain";
	if ( name.empty() == true )
		throw "Empty name";
	if ( node_name.empty() == true )
		throw "Empty node_name";
	if ( cmd_line.empty() == true )
		throw "Empty command line";

	this->job.id		= id;
	this->domain	= d;
	this->job.name		= name.c_str();
	this->job.node_name	= node_name.c_str();
	this->job.cmd_line	= cmd_line.c_str();

	this->job.weight	= weight;
}
*/
///////////////////////////////////////////////////////////////////////////////

Job::Job(Domain* d, const rpc::t_job& j) {
	rpc::ex_job e;

	if ( d == NULL ) {
		e.msg = "Empty domain";
		throw e;
	}
	if ( j.name.empty() == true ) {
		e.msg = "Empty name";
		throw e;
	}
	if ( j.node_name.empty() == true ) {
		e.msg = "Empty node_name";
		throw e;
	}
	if ( j.cmd_line.empty() == true ) {
		e.msg = "Empty command line";
		throw e;
	}
	if ( j.domain.empty() == true ) {
		e.msg = "Empty domain name";
		throw e;
	}

	this->domain = d;
	this->job = j;
}

///////////////////////////////////////////////////////////////////////////////

Job::~Job() {
//	delete this->start_time;
//	delete this->stop_time;

	this->job.prv.clear();
	this->job.nxt.clear();
}

///////////////////////////////////////////////////////////////////////////////

bool	Job::run() {
	if ( this->job.state == rpc::e_job_state::RUNNING ) {
		ERROR << job.name << " is already running";
		return false;
	}

	if ( this->update_state(rpc::e_job_state::RUNNING) == false ) {
		ERROR << "cannot update job's state to RUNNING" ;
		return false;
	}

	try {
		FILE*	p_job = NULL;

		this->job.start_time	= static_cast<long int>(time(NULL));

		if ( (p_job = popen(this->job.cmd_line.c_str(), "r")) != NULL )
			this->job.return_code = pclose(p_job);
		else
			this->job.return_code = 1;

		this->job.stop_time	= static_cast<long int>(time(NULL));

		INFO << "job " << job.name << " started at " << this->job.start_time << " (" << build_human_readable_time(this->job.start_time) << ")"
			  << " and stopped at " << this->job.stop_time << " (" << build_human_readable_time(this->job.stop_time) << ")";
	} catch (const std::exception e) {
		ERROR << "Exception: cannot execute " << this->job.name << " : " << e.what() ;
		return false;
	}

	INFO << "job " << this->job.name.c_str() << " returned code " << this->job.return_code;

	try {
		if ( this->job.return_code == 0 ) {
			if ( this->update_state(rpc::e_job_state::SUCCEDED, static_cast<time_t>(this->job.start_time), static_cast<time_t>(this->job.stop_time)) == false ) {
				ERROR << "cannot update job's state to SUCCEDED";
				return false;
			}
			return true;
		}

		if ( this->update_state(rpc::e_job_state::FAILED, static_cast<time_t>(this->job.start_time), static_cast<time_t>(this->job.stop_time)) == false ) {
			ERROR << "cannot update job's state to FAILED" ;
			return false;
		}
	} catch (const std::exception e) {
		ERROR << "Exception: cannot update job's state " << this->job.name << " : " << e.what() ;
		return false;
	}

	return false;
}

///////////////////////////////////////////////////////////////////////////////

bool	Job::update_state(const rpc::e_job_state::type js) {
	return this->domain->update_job_state(this->job.domain.c_str(), this, js);
}

///////////////////////////////////////////////////////////////////////////////

// TODO: fix it
bool	Job::update_state(const rpc::e_job_state::type js, time_t start_time, time_t stop_time) {
	return this->domain->update_job_state(this->job.domain.c_str(), this, js, start_time, stop_time);
}

///////////////////////////////////////////////////////////////////////////////

const rpc::t_job*	Job::get_job() const {
	return &this->job;
}

///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_name() const {
	return this->job.name;
}

///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_cmd_line() const {
	return this->job.cmd_line;
}

///////////////////////////////////////////////////////////////////////////////

const char*	Job::get_node_name() const {
	return this->job.node_name.c_str();
}
///////////////////////////////////////////////////////////////////////////////

const std::string	Job::get_node_name2() const {
	return this->job.node_name;
}

///////////////////////////////////////////////////////////////////////////////

int	Job::get_weight() const {
	return this->job.weight;
}

///////////////////////////////////////////////////////////////////////////////

rpc::e_job_state::type	Job::get_state() const {
	return this->job.state;
}

///////////////////////////////////////////////////////////////////////////////

const Domain*	Job::get_domain() const {
	return this->domain;
}

///////////////////////////////////////////////////////////////////////////////

const rpc::v_job_names	Job::get_next() const {
	return this->job.nxt;
}

///////////////////////////////////////////////////////////////////////////////

const rpc::v_job_names	Job::get_prev() const {
	return this->job.prv;
}
