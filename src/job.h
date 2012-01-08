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

// Should be placed before including domain.h
enum e_job_state { WAITING, RUNNING, SUCCEDED, FAILED };

#include <vector>

#include "common.h"
#include "domain.h"

class Domain;

typedef std::vector<int>	v_job_ids;

class Job {
public:
	Job(Domain* d, const std::string& name, const std::string& node_name, const std::string& cmd_line, const int& weight, const v_job_ids& pj, const v_job_ids& nj);
	Job(Domain* d, int id, std::string& name, std::string& node_name, std::string& cmd_line, int weight);
//	Job(rpc::t_job& j);
	~Job();

	/*
	 * run
	 *
	 * Runs the job and updates its state in the database
	 *
	 * @arg none
	 *
	 * @return true	: state updated sucessfuly
	 */
	bool		run();

	/*
	 * update_state
	 *
	 * Updates the state
	 *
	 * @arg	js		: the job's state
	 *
	 * @return true	: sucess
	 */
	bool		update_state(const e_job_state js);

	/*
	 * getters
	 */
	int			get_id()  const;
	std::string	get_name() const;
	std::string	get_cmd_line() const;
	const char*	get_node_name() const;
	int			get_weight() const;
	Domain*		get_domain() const;

	v_job_ids	get_next() const;
	v_job_ids	get_prev() const;

	/*
	 * setters
	 */
	void		set_id(const int);

private:
	/*
	 * start_time
	 *
	 * When the job started
	 */
	time_t		start_time;

	/*
	 * stop_time
	 *
	 * When the job stopped
	 */
	time_t		stop_time;

	/*
	 * return_code
	 *
	 * The code returned by the job's command line
	 */
	int			return_code;

	/*
	 * id
	 *
	 * The job's id (database)
	 */
	int			id;

	/*
	 * name
	 *
	 * The job's namme
	 */
	std::string	name;

	/*
	 * node_name
	 *
	 * The node hosting the job
	 */
	std::string	node_name;

	/*
	 * domain
	 *
	 * The domain owning the job
	 */
	Domain*		domain;

	/*
	 * cmd_line
	 *
	 * The command line to execute on the node
	 */
	std::string	cmd_line;

	/*
	 * weight
	 *
	 * The amont of needed resources to run the job
	 */
	int			weight;

	/*
	 * prev
	 *
	 * The jobs that must be run before the job
	 */
	v_job_ids	prev;

	/*
	 * next
	 *
	 * The jobs waiting for this job to success
	 */
	v_job_ids	next;
};

// } // namespace ows

#endif // JOB_H
