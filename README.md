# Open Workload Scheduler
This project aims to be a workload scheduler.

**FOR DEV ONLY, IT IS A PROOF OF CONCEPT.
A LOT OF FUNCTIONALITIES ARE NOT IMPLEMENTED YET.**

##  Why?

* **2010-02**: When I worked for an IT consulting company I have used a scheduler called TWS. One of my collegues, who was an advanced user, told me there were no open source equivalent to it. That is why we began to brainstorm on the key functionalities of such a
piece of software.
* **2011-12**: Nowadays I am still working on it during my leisure time. It allows me to :
	- keep/improve my programing skills ;
	- use some cool stuff like routing or distributed computing.

##  Definitions

* **Node**: a scheduled server.
* **Domain**: a group of nodes.
* **Master**: the main domain's node.
* **Client**: an unprivileged domain's node.
* **Shell**: a CLI to communicate with the nodes.
* **Proxy**: a gateway between a domain and the users.
* **Job**: a task to perform on a predefined node.
* **Macro job**: a group of linked jobs.
* **Day**: a periodic time within plannings are run.
* **Planning**: the jobs to run during the day on a single domain.

##  Architecture

###  Common stuff

* **Private/Public keys**: a node only knows a reduced number of neighbours. The connections are encrypted.
* **Routing**: a node tries to find three kinds of neighbours:
	1. its direct neighbours ;
	2. the domain's master to get its planning ;
	3. the nodes owning remote resources.
* **Mode 1**: full p2p
	- There is no master node. The planning is distributed.
* **Mode 2**: active mode
	- The nodes try to reach the master to get their planning.
* **Mode 3**: passive mode
	- The nodes are passive and do not get their planning. They just wait the master to give jobs to run.
* **Users' access**:
	- The CLI ;
	- The proxy manages ACLs and delivers services to GUIs.

###  The shell

Please see [ows-cli](https://www.github.com/mgrzybek/ows-cli).

### The proxy

Not written yet.

## Usage

```
$ ./master
Usage: master -f <config_file> [ -d || -v ]
	<config_file>	: the main configuration file
	-d		: daemon mode
	-c		: check the configuration and exit
	-v		: verbose mode
$
$ ./client
Usage: client -f <config_file> [ -d || -v ]
	<config_file>	: the main configuration file
	-d		: daemon mode
	-c		: check the configuration and exit
	-v		: verbose mode
$ 
```

## Building

### List of includes

* boost/algorithm/string.hpp
* boost/filesystem.hpp
* boost/foreach.hpp
* boost/lexical_cast.hpp
* boost/multi_index/member.hpp
* boost/multi_index/ordered_index.hpp
* boost/multi_index_container.hpp
* boost/regex.hpp
* boost/thread.hpp
* boost/thread/mutex.hpp
* boost/thread/thread.hpp
* csignal
* cstdlib
* cstring
* fstream
* iostream
* libcli.h
* limits
* log4cpp/Category.hh
* log4cpp/PropertyConfigurator.hh
* map
* mysql.h
* netdb.h
* netinet/in.h
* protocol/TBinaryProtocol.h
* resolv.h
* server/TSimpleServer.h
* sqlite3.h
* stdint.h
* string.h
* string
* sys/socket.h
* sys/types.h
* time.h
* transport/TBufferTransports.h
* transport/TServerSocket.h
* transport/TSocket.h
* unistd.h
* vector

### Commands

* Install boost
* Install thrift
* Install qmake

```
$ git clone git://github.com/mgrzybek/open-workload-scheduler.git
```

* Generate the Thrift's source code

```
$ cd open-workload-scheduler/src
$ thrift --gen cpp model.thrift 
```

* Patch the generated sources (you might need to increase the "-p" value and specify the files to patch)

```
$ cd ..
$ patch -p1 < patch/platform.path
```

* Create the Makefile you want to build (client or master)

```
$ qmake client.pro
$ qmake master.pro
```

* Run make (debug or release)

```
$ make -f Makefile.Debug
$ make -f Makefile.Release
```
