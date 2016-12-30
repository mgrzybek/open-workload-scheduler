/**
 * Project: OWS: an Open Source Workload Scheduler
 * File name: client.cpp
 * Description: contains the main() function of the client node.
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

// Generic stuff
#include <string>
#include <csignal>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/lexical_cast.hpp>

// Common stuff
#include "common.h"

// Config and routing stuff
//#include "cfg.h"
#include "router.h"
#include "rpc_server.h"

// Scheduler stuff
#include "day.h"
//#include "job.h"
#include "domain.h"
//#include "node.h"

///////////////////////////////////////////////////////////////////////////////

/**
 * @brief usage
 *
 * Prints the available options of the program to stdout.
 */
void	usage();

/**
 * @brief signal_handler
 *
 * Deals with the received signals when daemonized.
 *
 * @param	the signal received
 */
void	signal_handler(const int);

/**
 * @brief daemonnize
 *
 * Makes a clean fork :
 * - Forks cleanly
 * - Closes the file descriptors
 * - Writes the new PID to the given pid file
 * - Handles signals
 *
 * @param the PID file
 */
void	daemonnize(const char*);


///////////////////////////////////////////////////////////////////////////////

int	main (int argc, char * const argv[]) {
	char*	config_file	= NULL;
	bool	config_check	= false;
	bool	debug_mode	= false;
	bool	daemon_mode	= false;
	//	bool		check_mode	= false;
	Config	conf_params;
	Router	router(&conf_params);

	/*
	 * Initialisation
	 *
	 * Read the config file (-f option):
	 * - bind_address
	 * - bind_port (default: 555)
	 * - data_path (default: /var/lib/ows)
	 * - etc_path (default: /etc/ows)
	 */
	if ( argc < 3 ) {
		usage();
		return EXIT_FAILURE;
	}

	for ( int i = 1 ; i < argc ; i++ ) {
		if ( strcmp(argv[i], "-c" ) == 0 ) {
			config_check = true;
			continue;
		}
		if ( strcmp(argv[i], "-f" ) == 0 && i < argc ) {
			config_file = argv[i+1];
			continue;
		}
		if ( strcmp(argv[i], "-v" ) == 0 ) {
			debug_mode = true;
			continue;
		}
		if ( strcmp(argv[i], "-d" ) == 0 ) {
			daemon_mode = true;
			continue;
		}
	}

	if ( config_file == NULL ) {
		EMERG << "No config file given";
		usage();
		return EXIT_FAILURE;
	}

	// TODO: dael with "-c" argument to check the config file
	if ( conf_params.parse_file(config_file) == false ) {
		EMERG << "Cannot parse " << config_file;
		return EXIT_FAILURE;
	}

	/*
	 * Logging stuff
	 *
	 * We use macros to send messages. See common.h (INFO, NOTICE...)
	 */
	log4cpp::PropertyConfigurator::configure(conf_params.get_param("log4cpp_properties")->c_str());
	log4cpp::Category& root_logger = log4cpp::Category::getRoot();

//	if (daemon_mode == true)
//		daemonize();

	/*
	 * Peers Discovery
	 *
	 * - Get the (host, public key) list
	 * - Call Router.reach_master() : if true -> continue, else waiting loop (30 seconds and retry)
	 *
	 */
	router.update_peers_list();
	while ( router.get_reachable_peers_number() < 1 ) {
		WARN << "Cannot reach any peer" << std::endl;
		router.update_peers_list();
		sleep(30);
	}

	while ( router.reach_master() == false ) {
		WARN << "Cannot reach the master";
		sleep(30);
	}

	/*
	 * Planning loading
	 *
	 * - Call Rpc_Client.get_planning() until it succeeds
	 * - Create the domain
	 * - Call Domain.load_planning();
	 */
	rpc::t_node	node;
	Domain		domain(&conf_params);

	switch (conf_params.get_running_mode()) {
		case P2P: {
			/*
			 * Check if the domain contains data
			 * Or ask the direct peers for data
			 */
			if ( domain.contains_data(conf_params.get_param("node_name")->c_str()) == false ) {
				p_m_host_keys_iter iters = router.get_direct_peers();
				rpc::t_node	node;

				node.name = conf_params.get_param("node_name")->c_str();

				for ( auto iter = iters.first ; iter != iters.second ; iter++ ) {
					if ( router.get_node(*conf_params.get_param("domain_name"), node, iter->second.c_str()) == true )
						break;
				}
				if ( domain.add_node(conf_params.get_param("domain_name")->c_str(), node) == false ) {
					EMERG << "Cannot load the planning";
					return EXIT_FAILURE;
				}
			}
			break;
		}
		case ACTIVE: {
			while ( router.get_node(conf_params.get_param("domain_name")->c_str(), node, router.get_master_node()->c_str()) == false ) {
				WARN << "Cannot get the planning";
				sleep(30);
			}
			if ( domain.add_node(conf_params.get_param("domain_name")->c_str(), node) == false ) {
				EMERG << "Cannot load the planning";
				return EXIT_FAILURE;
			}
			break;
		}
		case PASSIVE: {
			/*
			 * Nothing is done. The master gives orders
			 */
			break;
		}
	}

	/*
	 * Ports listening
	 *
	 * - create a Rpc_Server object
	 * - create a dedicated thread
	 */
	Rpc_Server		server(&domain, &conf_params, &router);
	boost::thread	server_thread(boost::bind(&Rpc_Server::run, &server));

	/*
	 * Domain preparation
	 *
	 * - Try to reach the node needed by the planning : build the routing table
	 */

	/*
	 * Domain routine
	 *
	 * - Check for ready jobs every minute
	 * - Or wait master's orders
	 */
	v_jobs	jobs;
	boost::thread_group	running_jobs;

	switch (conf_params.get_running_mode()) {
		case (P2P): {break;};
		case (ACTIVE): {
			while (1) {
				domain.get_ready_jobs(jobs, conf_params.get_param("node_name")->c_str());
				DEBUG << "jobs size: " << jobs.size();
				BOOST_FOREACH(Job j, jobs) {
					running_jobs.create_thread(boost::bind(&Job::run, &j));
				}
				sleep(60);
			}
			break;
		}
		case (PASSIVE): {
			break;
		}
	}

	running_jobs.join_all();
	server_thread.join();

	INFO << "Clean shutdown";
	log4cpp::Category::shutdown();
	return EXIT_SUCCESS;
}

///////////////////////////////////////////////////////////////////////////////

/*
 * usage
 *
 * Prints the usage page on stdout
 *
 */
void	usage() {
	std::cout << "Usage: client -f <config_file> [ -d || -v ]" << std::endl
		<< "	<config_file>	: the main configuration file" << std::endl
		<< "	-d		: daemon mode" << std::endl
		<< "	-c		: check the configuration and exit" << std::endl
		<< "	-v		: verbose mode" << std::endl;
}

void	signal_handler(const int sig) {
	log4cpp::Category& root_logger = log4cpp::Category::getRoot();

	switch(sig) {
		case SIGHUP:
			INFO << "received SIGHUP, nothing done.";
			break;
		case SIGTERM:
			INFO << "received SIGTERM, shutting down.";
			log4cpp::Category::shutdown();
			exit(EXIT_SUCCESS);
			break;
	}
}

void	daemonize(const char* lock_file) {
	int			result;
	std::ofstream	f;

	result = fork();
	if (result < 0)
		exit(1); /* fork error */
	if (result > 0)
		exit(0); /* parent exits */

	/* child (daemon) continues */
	setsid(); /* obtain a new process group */

	for (result = getdtablesize() ; result >= 0 ; --result)
		close(result); /* close all descriptors */

	chdir("/tmp"); /* change running directory */

	/* first instance continues */
	f.open(lock_file);
	f << getpid();
	f.close();

	signal(SIGCHLD,SIG_IGN); /* ignore child */
	signal(SIGTSTP,SIG_IGN); /* ignore tty signals */
	signal(SIGTTOU,SIG_IGN);
	signal(SIGTTIN,SIG_IGN);
	signal(SIGHUP,signal_handler); /* catch hangup signal */
	signal(SIGTERM,signal_handler); /* catch kill signal */
}
