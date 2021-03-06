/*
 * main_src.c
 *
 *  Created on: 30.04.2012
 *      Author: YaroslavLitvinov
 */

#include "defines.h"
#include "sort.h"
#include "dsort.h"
#include "comm_src.h"
#include "sqluse_cli.h"
#include "logfile.h"

#include <unistd.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>



int main(int argc, char *argv[]){
	struct file_records_t file_records;
	int sourcefd = -1;
	if ( argc > 2 ){
		char logname[50];
		sprintf(logname, "%s%s%s.log", CLIENT_LOG, SOURCE, argv[2]);
		OPEN_LOG(logname, SOURCE, atoi(argv[2]));
		file_records.vfs_path = argv[1];
		int err = get_all_files_from_dbtable(DB_PATH, SOURCE, &file_records);
		WRITE_FMT_LOG(LOG_ERR, "get_all_files_from_dbtable err=%d", err);
		if ( err != 0 ) return 1;
	}
	else{
		printf("usage: 1st arg: should be path to VFS folder, 2nd: unique node integer id\n");fflush(0);
		return 1;
	}

	int nodeid = atoi(argv[2]);

	BigArrayPtr unsorted_array = NULL;
	BigArrayPtr partially_sorted_array = NULL;
	/*get unsorted data*/
	char inputfile[100];
	memset(inputfile, '\0', 100);
	sprintf(inputfile, SOURCE_FILE_FMT, nodeid );
	sourcefd = open(inputfile, O_RDONLY);
	if ( sourcefd >= 0 ){
		const size_t data_size = sizeof(BigArrayItem)*ARRAY_ITEMS_COUNT;
		unsorted_array = malloc( data_size );
		if ( unsorted_array ){
			const ssize_t readed = read(sourcefd, unsorted_array, data_size);
			assert(readed == data_size );
		}
		close(sourcefd);
	}
	else{
		WRITE_FMT_LOG(LOG_ERR, "Can not open input file %s", inputfile);
		exit(0);
	}

	/*sort data locally*/
	partially_sorted_array = alloc_merge_sort( unsorted_array, ARRAY_ITEMS_COUNT );

	//if first part of sorting in single thread are completed
	if ( test_sort_result( unsorted_array, partially_sorted_array, ARRAY_ITEMS_COUNT ) ){
		if ( ARRAY_ITEMS_COUNT ){
			WRITE_FMT_LOG(LOG_UI, "Single process sorting complete min=%u, max=%u: TEST OK.\n",
					partially_sorted_array[0], partially_sorted_array[ARRAY_ITEMS_COUNT-1] );
		}

		/*send crc of sorted array to the manager node*/
		uint32_t crc = array_crc( partially_sorted_array, ARRAY_ITEMS_COUNT );
		WRITE_FMT_LOG(LOG_DEBUG, "crc=%u", crc);
		struct file_record_t* write_crc_r = match_file_record_by_fd( &file_records, SOURCE_FD_WRITE_CRC);
		WRITE_FMT_LOG(LOG_DEBUG, "SOURCE_FD_WRITE_CRC fd=%p", write_crc_r);
		assert(write_crc_r);
		write_crc( write_crc_r->fpath, crc );
		WRITE_LOG(LOG_DEBUG, "crc wrote");
		/*send of crc complete*/

		int histogram_len = 0;
		HistogramArrayPtr histogram_array = alloc_histogram_array_get_len(
				partially_sorted_array, 0, ARRAY_ITEMS_COUNT, 1000, &histogram_len );

		struct Histogram single_histogram;
		single_histogram.src_nodeid = nodeid;
		single_histogram.array_len = histogram_len;
		single_histogram.array = histogram_array;
		//send histogram to manager

		struct file_record_t* write_hist_r = match_file_record_by_fd( &file_records, SOURCE_FD_WRITE_HISTOGRAM);
		assert(write_hist_r);
		write_histogram( write_hist_r->fpath, &single_histogram );

		struct file_record_t* read_dhist_req_r = match_file_record_by_fd( &file_records, SOURCE_FD_READ_D_HISTOGRAM_REQ);
		struct file_record_t* write_dhist_req_r = match_file_record_by_fd( &file_records, SOURCE_FD_WRITE_D_HISTOGRAM_REQ);
		assert(read_dhist_req_r);
		assert(write_dhist_req_r);
		read_requests_write_detailed_histograms( read_dhist_req_r->fpath, write_dhist_req_r->fpath, nodeid,
			partially_sorted_array, ARRAY_ITEMS_COUNT );

		WRITE_LOG(LOG_DETAILED_UI, "\n!!!!!!!Histograms Sending complete!!!!!!.\n");

		struct request_data_t req_data_array[SRC_NODES_COUNT];
		init_request_data_array( req_data_array, SRC_NODES_COUNT);
		//////////////////////////////
		struct file_record_t* read_sequnce_req_r = match_file_record_by_fd( &file_records, SOURCE_FD_READ_SEQUENCES_REQ);
		assert(read_sequnce_req_r);
		read_range_request( read_sequnce_req_r->fpath, req_data_array );
		//////////////////////////////

		//////////////////////////////
		for ( int i=0; i < SRC_NODES_COUNT; i++ ){
			int dst_nodeid = req_data_array[i].dst_nodeid;
			int dst_write_fd = dst_nodeid - FIRST_DEST_NODEID + SOURCE_FD_WRITE_SORTED_RANGES_REQ;
			int dst_read_fd = dst_nodeid - FIRST_DEST_NODEID + SOURCE_FD_READ_SORTED_RANGES_REQ;
			WRITE_FMT_LOG(LOG_DEBUG, "write_sorted_ranges fdw=%d, fdr=%d", dst_write_fd, dst_read_fd );

			struct file_record_t* write_ranges_req_r = match_file_record_by_fd( &file_records, dst_write_fd);
			struct file_record_t* read_ranges_req_r = match_file_record_by_fd( &file_records, dst_read_fd);
			assert(write_ranges_req_r);
			assert(read_ranges_req_r);

			WRITE_FMT_LOG(LOG_DEBUG, "req_data_array[i].dst_nodeid=%d", req_data_array[i].dst_nodeid );
			write_sorted_ranges( write_ranges_req_r->fpath, read_ranges_req_r->fpath,
					&req_data_array[i], partially_sorted_array );
		}

		WRITE_FMT_LOG(LOG_DETAILED_UI, "[%d]Sending Ranges Complete-OK", nodeid);
		//////////////////////////////

		free(unsorted_array);
		free(partially_sorted_array);
	}
	else{
		WRITE_LOG(LOG_UI, "Single process sorting failed: TEST FAILED.\n");
		exit(0);
	}
}

