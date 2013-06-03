/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: convertions.cpp
 * Description: defines functions to use in the project:
 * - convert rpc:: types to std ones
 *
 * @author Mathieu Grzybek on 2012-06-27
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

#include "convertions.h"

rpc::e_job_state::type	build_job_state_from_string(const char* state) {
	if ( strcmp(state, "waiting") == 0 )
		return rpc::e_job_state::WAITING;
	if ( strcmp(state, "running") == 0 )
		return rpc::e_job_state::RUNNING;
	if ( strcmp(state, "succeded") == 0 )
		return rpc::e_job_state::SUCCEDED;
	if ( strcmp(state, "failed") == 0 )
		return rpc::e_job_state::FAILED;

	// TODO: throw a clean exception
	throw "Error: string state is not related to a job's state";
	return rpc::e_job_state::FAILED;
}

std::string	build_string_from_job_state(const rpc::e_job_state::type& js) {
	std::string	result;

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

rpc::e_rectype_action::type	build_rectype_action_from_string(const char* rt_action) {
	if ( strcmp(rt_action, "restart") == 0 )
		return rpc::e_rectype_action::RESTART;
	if ( strcmp(rt_action, "stop_schedule") == 0 )
		return rpc::e_rectype_action::STOP_SCHEDULE;

	// TODO: throw a clean exception
	throw "Error: string action is not related to a rectype action";
	return rpc::e_rectype_action::STOP_SCHEDULE;
}

std::string	build_string_from_rectype_action(const rpc::e_rectype_action::type& rt_action) {
	std::string	result;

	switch (rt_action) {
		case rpc::e_rectype_action::RESTART: {
			result = "restart";
			break;
		}
		case rpc::e_rectype_action::STOP_SCHEDULE: {
			result = "stop_schedule";
			break;
		}
	}
	// TODO: case default -> exception
	return result;
}

rpc::e_time_constraint_type::type build_time_constraint_type_from_string(const char* type) {
	if ( strcmp(type, "at") == 0 )
		return rpc::e_time_constraint_type::AT;
	if ( strcmp(type, "before") == 0 )
		return rpc::e_time_constraint_type::BEFORE;
	if ( strcmp(type, "after") == 0 )
		return rpc::e_time_constraint_type::AFTER;

	// TODO: throw a clean exception
	throw "error";
	return rpc::e_time_constraint_type::BEFORE;
}

std::string	build_string_from_time_constraint_type(const rpc::e_time_constraint_type::type& tc_t) {
	std::string	result;

	switch (tc_t) {
		case rpc::e_time_constraint_type::AT: {
			result = "at";
			break;
		}
		case rpc::e_time_constraint_type::BEFORE: {
			result = "before";
			break;
		}
		case rpc::e_time_constraint_type::AFTER: {
			result = "after";
			break;
		}
	}

	return result;
}

std::string	build_human_readable_time(const time_t& time) {
	std::string	result;
	char		buffer[80];
	struct tm*	timeinfo;

	timeinfo = localtime(&time);
	strftime(buffer,80,"%c",timeinfo);

	result = buffer;

	return result;
}
