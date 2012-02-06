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

#ifndef COMMON_H
#define COMMON_H

#include "thrift/model_types.h"

/*
 * Database selection
 *
 * You can only use one database engine.
 */

//	To use MySQL support
#define USE_MYSQL

//	To use SQLite support
//#define USE_SQLITE

/*
 * RPC selection
 *
 * You can only use one RPC engine.
 * I used to use xmlrpc-c but did not manage to build something efficient.
 */

// To use Apache Thrift
#define USE_THRIFT

// TODO: think of ZeroMQ + custom message encryption

/*
 * build_job_state_from_string
 *
 * Translates a 'stringed' job_state to an enumed job_state
 *
 * @arg state	: the state to convert
 * @return		: the 'stringed' state
 */
rpc::e_job_state::type	build_job_state_from_string(const char* state);
std::string	build_string_from_job_state(const rpc::e_job_state::type js);

#endif // COMMON_H
