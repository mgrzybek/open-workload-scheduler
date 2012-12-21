/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: convertions.h
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

#include "gen-cpp/model_types.h"

/*
 * build_job_state_from_string
 *
 * Translates a 'stringed' job_state to an enumed job_state
 *
 * @arg state	: the state to convert
 * @return		: the 'stringed' state
 */
rpc::e_job_state::type	build_job_state_from_string(const char* state);

/*
 * build_string_from_job_state
 *
 * Translates an enumed job_state to a 'stringed' one
 *
 * @arg state	: the 'stringed' state
 * @return		: the state to convert
 */
std::string	build_string_from_job_state(const rpc::e_job_state::type& js);

/*
 * build_rectype_action_from_string
 *
 * Translates a 'stringed' rectype_action to an enumed one
 *
 * @arg rt_action	: the type to convert
 * @return			: the 'stringed' type
 */
rpc::e_rectype_action::type	build_rectype_action_from_string(const char* rt_action);

/*
 * build_string_from_rectype_action
 *
 * Translates a enumed rectype_action to an 'stringed' one
 *
 * @arg rt_action	: the type to convert
 * @return			: the 'stringed' type
 */
std::string	build_string_from_rectype_action(const rpc::e_rectype_action::type& rt_action);

/*
 * build_time_constraint_type_from_string
 *
 * Translates a 'stringed' time_constraint_type to an enumed one
 *
 * @arg tc_type	: the type to convert
 * @return		: the 'stringed' type
 */
rpc::e_time_constraint_type::type build_time_constraint_type_from_string(const char* type);

/*
 * build_string_from_time_constraint_type
 *
 * Translates an enumed time_constraint_type to an 'stringed' one
 *
 * @arg tc_type	: the enumed type to convert
 * @return		: the 'stringed' type
 */
std::string	build_string_from_time_constraint_type(const rpc::e_time_constraint_type::type& tc_t);
