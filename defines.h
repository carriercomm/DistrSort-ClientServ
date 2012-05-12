/*
 * defines.h
 *
 *  Created on: 08.05.2012
 *      Author: yaroslav
 */

#ifndef DEFINES_H_
#define DEFINES_H_

#define CLIENT_LOG "log/client\0"
#define SERVER_LOG "log/server\0"

#define DB_PATH "client_dsort.db\0"
#define SERVER_DB_PATH "server_dsort.db"

#define MANAGER "manager\0"
#define SOURCE "source\0"
#define DEST "dest\0"

/*Debugging*/
#define FUSE_CLIENT
#define FUSE_IO_MAX_CHUNK_SIZE (100*1024)
#define DEBUG_PRINT_ARRAY

#define ARRAY_ITEMS_COUNT 1000000
#define SRC_NODES_COUNT 3

#define FIRST_SOURCE_NODEID 2
#define LAST_SOURCE_NODEID (FIRST_SOURCE_NODEID+SRC_NODES_COUNT-1)
#define FIRST_DEST_NODEID 7
#define LAST_DEST_NODEID (FIRST_DEST_NODEID+SRC_NODES_COUNT-1)

#define MANAGER_FD_READ_HISTOGRAM 3
#define MANAGER_FD_WRITE_D_HISTOGRAM_REQ 4
#define MANAGER_FD_READ_D_HISTOGRAM_REQ 9
#define MANAGER_FD_WRITE_RANGE_REQUEST 14
#define MANAGER_FD_READ_SORT_RESULTS 19

#define SOURCE_FD_WRITE_HISTOGRAM 3
#define SOURCE_FD_READ_D_HISTOGRAM_REQ 4
#define SOURCE_FD_WRITE_D_HISTOGRAM_REQ 5
#define SOURCE_FD_READ_SEQUENCES_REQ 6
#define SOURCE_FD_WRITE_SORTED_RANGES_REQ 7
#define SOURCE_FD_READ_SORTED_RANGES_REQ 12

#define DEST_FD_READ_SORTED_RANGES_REP 3
#define DEST_FD_WRITE_SORTED_RANGES_REP 4
#define DEST_FD_WRITE_SORT_RESULT 5

#endif /* DEFINES_H_ */
