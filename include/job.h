/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: job.h
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

#ifndef JOB_H
#define JOB_H

// namespace ows {

#include <vector>

#include "common.h"
#include "ows_config.h"
#include "domain.h"

#include "gen-cpp/model_types.h"

class Domain;

//typedef std::vector<int>	v_job_ids;

class Job {
public:
	/**
	 * Job
	 *
	 * The constructor
	 *
	 * @param	d	the hosting domain
	 * @param	j	the job to create from rpc::t_job type
	 */
	Job(Domain* d, const rpc::t_job& j);

	/**
	 * ~Job
	 *
	 * The destructor
	 */
	~Job();

	/**
	 * run
	 *
	 * Runs the job and updates its state in the database
	 *
	 * @return	true	state updated sucessfuly
	 */
	bool	run();

	/**
	 * update_state
	 *
	 * Updates the state
	 * TODO: is it crappy to use several methods?
	 *
	 * @param	js	the job's state
	 *
	 * @return	true	on sucess
	 */
	bool	update_state(const rpc::e_job_state::type js);

	/**
	 * update_state
	 *
	 * Updates the state
	 * TODO: is it crappy to use several methods?
	 *
	 * @param	js	the job's state
	 * @param	start_time	when the job started
	 * @param	stop_time	when the job ended
	 *
	 * @return	true on sucess
	 */
	bool	update_state(const rpc::e_job_state::type js, time_t start_time, time_t stop_time);

	/**
	 * get_
	 *
	 * @return
	 */
	const rpc::t_job*	get_job() const;

	/**
	 * get_id
	 *
	 * @return	the job's id
	 */
	int	get_id()  const;

	/**
	 * get_name
	 *
	 * @return	the job's name
	 */
	const std::string	get_name() const;

	/**
	 * get_cmd_line
	 *
	 * @return	the job's command line to start on a node
	 */
	const std::string	get_cmd_line() const;

	/**
	 * get_node_name
	 *
	 * @return	the name of the node hosting the job
	 */
	const char*	get_node_name() const;

	/**
	 * get_node_name2
	 *
	 * @return	the name of the node hosting the job
	 */
	const std::string	get_node_name2() const;

	/**
	 * get_weight
	 *
	 * @return	the weight of the job
	 */
	int	get_weight() const;

	/**
	 * get_state
	 *
	 * @return	the state of the job
	 */
	rpc::e_job_state::type	get_state() const;

	/**
	 * get_domain
	 *
	 * @return	the domain hosting the job
	 */
	const Domain*	get_domain() const;

	/**
	 * get_next
	 *
	 * @return	the jobs' names to start after the current job
	 */
	const rpc::v_job_names	get_next() const;

	/**
	 * get_prev
	 *
	 * @return	the jobs' names to end before starting the current job
	 */
	const rpc::v_job_names	get_prev() const;

	/**
	 * set_id
	 *
	 * TODO: implement me
	 *
	 * @param	id	the job's id
	 */
	void	set_id(const int);

private:
	/**
	 * domain
	 *
	 * Pointer to the Domain object that cannot be stored in a t_job
	 */
	Domain*		domain;

	/**
	 * job
	 *
	 * We directly use the rpc::t_job to store data
	 * This prevents us to code twice
	 */
	rpc::t_job	job;
};

// } // namespace ows

#endif // JOB_H
