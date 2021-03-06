/*
 * manager.c
 *
 *  Created on: 29.04.2012
 *      Author: YaroslavLitvinov
 */

#include "defines.h"
#include "sqluse_cli.h"
#include "comm_man.h"
#include "distr_common.h"
#include "histanlz.h"
#include "dsort.h"
#include "logfile.h"

#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <assert.h>
#include <stdlib.h>
#include <string.h>


int main(int argc, char *argv[]){
	struct file_records_t file_records;
	if ( argc > 2 ){
		char logname[50];
		sprintf(logname, "%s%s%s.log", CLIENT_LOG, MANAGER, argv[2]);
		OPEN_LOG(logname, MANAGER, atoi(argv[2]));
		file_records.vfs_path = argv[1];
		int err = get_all_files_from_dbtable(DB_PATH, MANAGER, &file_records);
		if ( err != 0 ) return 1;
	}
	else{
		printf("usage: 1st arg: should be path to VFS folder, 2nd: unique node integer id\n");fflush(0);
		return 1;
	}

	int nodeid = atoi(argv[2]);

	WRITE_LOG(LOG_UI, "Manager node started");

	/*crc reading from all source nodes*/
	uint32_t crc_array[SRC_NODES_COUNT];
	memset(crc_array, '\0', SRC_NODES_COUNT*sizeof(uint32_t));
	struct file_record_t* crc_read_r = match_file_record_by_fd( &file_records, MANAGER_FD_READ_CRC);
	assert(crc_read_r);
	WRITE_FMT_LOG(LOG_DEBUG, "open crc reading fd=%d", MANAGER_FD_READ_CRC);
	read_crcs(crc_read_r->fpath, crc_array);
	/*crc read ok*/

	/******** Histograms reading*/
	struct file_record_t* hist_record = match_file_record_by_fd( &file_records, MANAGER_FD_READ_HISTOGRAM);
	assert(hist_record);
	struct Histogram histograms[SRC_NODES_COUNT];
	WRITE_LOG(LOG_DETAILED_UI, "Recv histograms");
	recv_histograms( hist_record->fpath, (struct Histogram*) &histograms, SRC_NODES_COUNT );
	WRITE_LOG(LOG_DETAILED_UI, "Recv histograms OK");
	/******** Histograms reading OK*/

	/*Sort histograms by src_nodeid, because recv order is unexpected,
	 * but histogram analizer algorithm is required deterministic order*/
	qsort( histograms, SRC_NODES_COUNT, sizeof(struct Histogram), histogram_srcid_comparator );

	WRITE_LOG(LOG_DETAILED_UI, "Analize histograms and request detailed histograms");
	struct request_data_t** range = alloc_range_request_analize_histograms( &file_records,
			ARRAY_ITEMS_COUNT, nodeid, histograms, SRC_NODES_COUNT );
	WRITE_LOG(LOG_DETAILED_UI, "Analize histograms and request detailed histograms OK");

#if defined(LOG_ENABLE) && LOG_LEVEL>=LOG_DEBUG
	for (int i=0; i < SRC_NODES_COUNT; i++ )
	{
		WRITE_FMT_LOG( LOG_DEBUG, "SOURCE PART N %d:\n", i );
		print_request_data_array( range[i], SRC_NODES_COUNT );
	}
#endif

	/// sending range requests to every source node
	for (int i=0; i < SRC_NODES_COUNT; i++ ){
		///
		int src_nodeid = range[0][i].src_nodeid;
		int src_write_fd = src_nodeid - FIRST_SOURCE_NODEID + MANAGER_FD_WRITE_RANGE_REQUEST;
		WRITE_FMT_LOG(LOG_DEBUG, "write_sorted_ranges fdw=%d", src_write_fd );

		struct file_record_t* rangereq_record = match_file_record_by_fd( &file_records, src_write_fd);
		assert(rangereq_record);
		write_range_request( rangereq_record->fpath, range, SRC_NODES_COUNT, i );
	}

	for ( int i=0; i < SRC_NODES_COUNT; i++ ){
		free(histograms[i].array);
		free( range[i] );
	}
	free(range);

	struct file_record_t* results_record = match_file_record_by_fd( &file_records, MANAGER_FD_READ_SORT_RESULTS);
	assert(results_record);
	struct sort_result *results = read_sort_result( results_record->fpath, SRC_NODES_COUNT );
	/*sort results by nodeid, because receive order not deterministic*/
	qsort( results, SRC_NODES_COUNT, sizeof(struct sort_result), sortresult_comparator );
	int sort_ok = 1;
	uint32_t crc_test_unsorted = 0;
	uint32_t crc_test_sorted = 0;
	for ( int i=0; i < SRC_NODES_COUNT; i++ ){
		crc_test_unsorted = (crc_test_unsorted+crc_array[i]% CRC_ATOM) % CRC_ATOM;
		crc_test_sorted = (crc_test_sorted+results[i].crc% CRC_ATOM) % CRC_ATOM;
		if ( i>0 ){
			if ( !(results[i].max > results[i].min && results[i-1].max < results[i].min) )
				sort_ok = 0;
		}
		WRITE_FMT_LOG(LOG_UI, "results[%d], nodeid=%d, min=%u, max=%u\n",
				i, results[i].nodeid, results[i].min, results[i].max);
		fflush(0);
	}

	WRITE_FMT_LOG( LOG_UI, "Distributed sort complete, Test %d, %d, %d\n", sort_ok, crc_test_unsorted, crc_test_sorted );
	if ( crc_test_unsorted == crc_test_sorted ){
		WRITE_LOG(LOG_UI, "crc OK");
	}
	else{
		WRITE_LOG(LOG_UI, "crc FAILED");
	}
	return 0;
}
