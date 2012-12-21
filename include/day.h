/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: day.h
 * Description: describes the period within the planning is run.
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

#ifndef DAY_H
#define DAY_H

#include "common.h"

// namespace ows {

class Day {
public:
	Day();
	~Day();

	/*
	 * get_time
	 *
	 * Gives the day's time
	 * TODO: implement it
	 *
	 * @return the day's time
	 */
	time_t	get_time();

	/*
	 * is_freeze_time
	 *
	 * Can we still update the planning ?
	 * TODO: implement it
	 *
	 * @return true	: yes
	 */
	bool	is_freeze_time();

private:

	/*
	 * start_timesptamp
	 *
	 * The day's start time
	 */
	time_t	start_timestamp;

	/*
	 * length
	 *
	 * The day's duration
	 */
	time_t	length;

	/*
	 * freeze_duration
	 *
	 * During this time you cannot update the planning to let nodes update
	 * their database.
	 */
	time_t	freeze_duration;
};

// } // namespace

#endif // DAY_H
