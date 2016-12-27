/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: node.h
 * Description: used to describe a node's caracteristics.
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

#ifndef NODE_H
#define NODE_H

// namespace ows {

#include "common.h"

// TODO: implement it

class Node {
public:
	Node();
	~Node();

private:

	/**
	 * root_logger
	 *
	 * This is a reference to the root logger
	 */
	log4cpp::Category&	root_logger = log4cpp::Category::getRoot();
};

// } // namespace ows

#endif // NODE_H
