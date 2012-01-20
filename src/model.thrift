/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: model.thrift
 * Description: defines how the RPC are used:
 * - the remote procedures themselves
 * - the data structures and types.
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

namespace cpp rpc

enum e_job_state {
	WAITING,
	RUNNING,
	SUCCEDED,
	FAILED
}

typedef i16 integer
typedef list<integer> v_job_ids

struct	t_node {
	1: required string	name,
	2: optional integer	weight,
}

struct	t_job {
	1: optional i64		start_time,
	2: optional i64		stop_time,
	3: optional integer	return_code,
	4: optional e_job_state	state,

	5: required integer	id,
	6: required string	name,
	7: required string	node_name,
	8: required string	domain,
	9: required string	cmd_line,
	10: required integer	weight,

	11: optional v_job_ids	prv,
	12: optional v_job_ids	nxt,
}

typedef list<t_job> v_jobs

struct	t_hello {
	1: required string	domain,
	2: required string	name,
	3: required bool	is_master,
}

struct t_route {
	1: required string	node_name,
	2: required integer	hops,
}

exception e_routing {
	1: string	msg,
}

exception e_job {
	1: string	msg,
}

exception e_node {
	1: string	msg,
}

service ows_rpc {
	/*
	 * Routing
	 */
	t_hello	hello(
			1: required t_node	target_node
	) throws (1:e_routing e);
	t_route	reach_master() throws (1:e_routing e);

	/*
	 * Node
	 */
	bool	add_node(
			1: required string	running_node,
			2: required t_node	node,
	) throws (1:e_node e);

	/*
	 * Jobs
	 */
	v_jobs	get_jobs(
			1: required string	running_node
	) throws (1:e_job e);
	v_jobs	get_ready_jobs(
			1: required string	running_node
	) throws (1:e_job e);
	bool	add_job(
			1: required t_job		j
	) throws (1:e_job e);
	bool	remove_job(
			1: required t_job		j
	) throws (1:e_job e);
//	bool	remove_job(
//			1: required string	running_node,
//			2: required integer	j_id,
//	) throws (1:e_job e);
	bool	update_job_state(
			1: required t_job		j,
			2: required e_job_state	js
	) throws (1:e_job e);
//	bool	update_job_state(
//			1: required string	running_node,
//			2: required integer	j_id,
//			3: required e_job_state	js
//	) throws (1:e_job e);

	/*
	 * SQL
	 */
	oneway void	sql_exec(
			1: required string	query,
	);
}

