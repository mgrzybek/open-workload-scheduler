/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: model.thrift
 * Description: defines how the RPC are used:
 * -> the remote procedures themselves
 * -> the data structures and types.
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

namespace cpp	rpc
namespace perl	rpc
namespace py	rpc

/*
 * e_job_state
 */
enum	e_job_state {
	WAITING,
	RUNNING,
	SUCCEDED,
	FAILED
}

/*
 * e_rectype_action
 */
enum	e_rectype_action {
	RESTART,
	STOP_SCHEDULE
}

/*
 * e_time_constraint_type
 */
enum	e_time_constraint_type {
	AT,
	BEFORE,
	AFTER
}

typedef i16 integer
typedef list<string> v_job_ids

/*
 * t_resource
 *
 * Linked to a node
 * TODO: think of defining start_value and current_value
 */
struct	t_resource {
	1: required string	name,
	2: required integer	current_value,
	3: required integer	initial_value,
}
typedef list<t_resource>	v_resources

/*
 * t_time_constraint
 *
 * Linked to a job
 */
struct	t_time_constraint {
	1: required integer			id,
	2: required e_time_constraint_type	type,
	3: required i64				value,
}
typedef list<t_time_constraint>	v_time_constraints

/*
 * t_recovery_type
 */
struct	t_recovery_type {
	1: required integer		id,
	2: required string		short_label,
	3: required string		label,
	4: required e_rectype_action	action,
}
typedef list<t_recovery_type>	v_recovery_types

/*
 * t_job
 */
struct	t_job {
	/*
	 * start_time
	 *
	 * When the job started
	 */
	1: required i64		start_time,

	/*
	 * stop_time
	 *
	 * When the job stopped
	 */
	2: required i64		stop_time,

	/*
	 * return_code
	 *
	 * The code returned by the job's command line
	 */
	3: required integer	return_code,

	/*
	 * state
	 *
	 * Has the job already been run?
	 */
	4: required e_job_state	state,

	/*
	 * name
	 *
	 * The job's namme
	 */
	5: required string	name,

	/*
	 * node_name
	 *
	 * The node hosting the job
	 */
	6: required string	node_name,

	/*
	 * domain
	 *
	 * The domain owning the job
	 */
	7: required string	domain,

	/*
	 * cmd_line
	 *
	 * The command line to execute on the node
	 */
	8: required string	cmd_line,

	/*
	 * weight
	 *
	 * The amont of needed resources to run the job
	 */
	9: required integer	weight,

	/*
	 * prv
	 *
	 * The jobs that must be run before the job
	 */
	10: required v_job_ids	prv

	/*
	 * next
	 *
	 * The jobs waiting for this job to success
	 */
	11: required v_job_ids	nxt,

	/*
	 * time_constraints
	 *
	 * The list of the job's time constraints (at, before, after)
	 * TODO: update the add / update methods
	 */
	12: required v_time_constraints	time_constraints,

	/*
	 * recovery_type
	 *
	 * TODO: update the add / update methods
	 */
	13: required t_recovery_type	recovery_type,
}
typedef list<t_job>		v_jobs

/*
 * t_node
 */
struct	t_node {
	1: required string	name,
	2: required integer	weight,

	3: required string	domain_name,

	5: required v_resources	resources,
	6: required v_jobs	jobs,
}
typedef list<t_node>		v_nodes


/*
 * t_macro_job
 *
 * TODO: think of the way it works
 */
struct	t_macro_job {
	1: required integer	id,
	2: required string	name,
	3: required v_jobs	jobs,
}
typedef list<t_macro_job>	v_macro_jobs

/*
 * t_planning
 *
 * TODO: we could use a node instead (v_jobs -> v_*)
 */
struct	t_planning {
	1: required t_node		hosting_node,

	2: required v_nodes		nodes,

	3: required v_jobs		jobs,
	4: required v_recovery_types	recoveries,
	5: required v_resources		resources,
	6: required v_time_constraints	time_constraints,

	7: required v_macro_jobs	macro_jobs,
}

/*
 * t_hello
 *
 * Defines the values given by a hello request
 */
struct	t_hello {
	1: required string	domain,
	2: required string	name,
	3: required bool	is_master,
}

/*
 * t_route
 */
struct t_route {
	1: required t_node	destination_node,
	2: required integer	hops,
}

exception ex_routing {
	1: string	msg,
}

exception ex_job {
	1: string	msg,
}

exception ex_node {
	1: string	msg,
}

exception ex_planning {
	1: string	msg,
}

service ows_rpc {
	/*
	 * Routing
	 */
	t_hello	hello(
			1: required t_node	target_node
	) throws (1:ex_routing e);
	t_route	reach_master() throws (1:ex_routing e);

	/*
	 * Planning
	 */
	t_planning	get_planning() throws (1:ex_routing r, 2:ex_planning p); 

	bool		set_planning(
			1: required t_node	calling_node,
			2: required t_planning	planning,
	) throws (
			1:ex_routing	r,
			2:ex_planning	p
	);

	/*
	 * Node
	 */
	bool	add_node(
			1: required t_node	calling_node,
			2: required t_node	hosting_node,
			3: required t_node	node_to_add,
	) throws (
			1:ex_routing	r,
			2:ex_node	n
	);

	/*
	 * Jobs
	 */
	v_jobs	get_jobs(
			1: required t_node	calling_node,
			2: required t_node	target_node,
	) throws (
			1:ex_routing	r,
			2:ex_job	j
	);

	v_jobs	get_ready_jobs(
			1: required t_node	calling_node,
			2: required t_node	target_node,
	) throws (
			1:ex_routing	r,
			2:ex_job	j
	);

	bool	add_job(
			1: required t_node	calling_node,
			2: required t_job	j,
	) throws (
			1:ex_routing	r,
			2:ex_job	j
	);


	bool	remove_job(
			1: required t_node	calling_node,
			2: required t_job	j,
	) throws (
			1:ex_routing	r,
			2:ex_job	e
	);

//	bool	remove_job(
//			1: required t_node	calling_node,
//			2: required integer	j_id,
//	) throws (1:ex_job e);

	bool	update_job_state(
			1: required t_node	calling_node,
			2: required t_job	j,
	) throws (
			1:ex_routing	r,
			2:ex_job	e
	);

//	bool	update_job_state(
//			1: required string	running_node,
//			2: required integer	j_id,
//			3: required e_job_state	js
//	) throws (1:ex_job e);

	/*
	 * SQL
	 */
	oneway void	sql_exec(
			1: required string	query,
	);
}

