/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: config.h
 * Description: defines the macro / functions to use in the project:
 * - which backend to use
 * - which RPC protocol to use...
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

#include "common.h"

/*
 * build_job_state_from_string
 *
 * Translates a 'stringed' job_state to an enumed job_state
 *
 * @arg state	: the state to convert
 * @return		: the 'stringed' state
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
