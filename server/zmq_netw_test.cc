/*
 * Tests for zeromq networking
 *  Author: YaroslavLitvinov
 *  Date: 7.03.2012
 */

#include <stdlib.h>
#include <zmq.h>

#include "gtest/gtest.h"
extern "C" {
#include "logfile.h"
#include "zmq_netw.h"
#include "fs_inmem.h"
}

const char *__endpoint1 = "ipc:///tmp/test1\0";
#define TEST_DATA_SIZE 1000000

/*helpers*/
struct zeromq_pool *CreateArrayPoolNoZmq();
void FreeArrayPoolNoZmq( struct zeromq_pool *);
char* alloc_fill_random(int bytes);


// Test harness for routines in zmq_netw.c
class ZmqNetwTests : public ::testing::Test {
public:
	void TestSockfArrayAdd100items();
	void TestSockfArrayAddDuplicateditems();
	void TestSockfArrayFinditems();
	void TestInitTermZmqNetwPool();
	void TestSockfOpenClose();
	void TestSockfCommunicationPushPull();
	void TestSockfCommunicationReqRep();

protected:
  // Sets up the test fixture.
  void SetUp();
  // Tears down the test fixture.
  void TearDown();
  /**@return bytes count really writed*/
  int TestWriteLog( int detail_level, int verbose, const char* logstr );
 private:
  //struct zeromq_pool *m_zmq_pool;
};


void ZmqNetwTests::SetUp()
{
	/*init zmq networking*/
	//m_zmq_pool = (struct zermoq_pool*) malloc(sizeof(struct zeromq_pool));
	//EXPECT_NE(m_zmq_pool, (struct zeromq_pool*)NULL);
	//init_zeromq_pool(m_zmq_pool);
}

void ZmqNetwTests::TearDown()
{
	//zeromq_term(m_zmq_pool);
}

struct zeromq_pool *CreateArrayPoolNoZmq(){
	struct zeromq_pool * pool = (struct zeromq_pool*) malloc(sizeof(struct zeromq_pool));
	assert(pool);
	pool->count_max = ESOCKF_ARRAY_GRANULARITY;
	pool->sockf_array = (struct sock_file_t*) malloc(pool->count_max * sizeof(struct sock_file_t));
	EXPECT_NE(pool->sockf_array, (void*)NULL);
	if ( pool->sockf_array ) {
		memset(pool->sockf_array, '\0', sizeof(struct sock_file_t)*pool->count_max);
		for (int i=0; i < pool->count_max; i++)
			pool->sockf_array[i].unused = 1;
		return pool;
	}
	else{
		/*no memory allocated*/
		return NULL;
	}
}

void FreeArrayPoolNoZmq( struct zeromq_pool *pool){
	if ( pool ){
		if ( pool->sockf_array ){
			free(pool->sockf_array);
		}
		free(pool);
	}
}

char* alloc_fill_random(int bytes){
	char *data = (char*)malloc( bytes );
	if ( !data ) return data;
	pid_t pid = getpid();
	//fill array by random numbers
	srand((time_t)pid );
	for (int i=0; i<bytes; i++){
		data[i]=rand();
	}
	return data;

}

TEST_F(ZmqNetwTests, TestSockfArrayAdd100items) {
	WRITE_LOG(LOG_DEBUG, "TestSockfArrayAdd1000items::create array");
	struct zeromq_pool *pool = CreateArrayPoolNoZmq();
	EXPECT_NE((void*)NULL, pool);
	WRITE_LOG(LOG_DEBUG, "create array ok");
	struct sock_file_t sockf;
	for (int i=0; i < 100; i++){
		memset(&sockf, '\0', sizeof(struct sock_file_t));
		sockf.fs_fd = i;
		int err = add_sockf_copy_to_array( pool, &sockf);
		if ( ERR_OK != err ){
			WRITE_FMT_LOG(LOG_ERR, "add to array failed err=%d", err);
			FAIL();
		}
	}
	WRITE_LOG(LOG_DEBUG, "free array..");
	FreeArrayPoolNoZmq(pool);
	WRITE_LOG(LOG_DEBUG, "free array OK");
}

TEST_F(ZmqNetwTests, TestSockfArrayAddDuplicateditems) {
	WRITE_LOG(LOG_DEBUG, "TestSockfArrayAddDuplicateditems::create array");
	struct zeromq_pool *pool = CreateArrayPoolNoZmq();
	EXPECT_NE((void*)NULL, pool);
	WRITE_LOG(LOG_DEBUG, "create array ok");
	struct sock_file_t sockf;
	//add items
	sockf.fs_fd = 800;
	EXPECT_EQ( ERR_OK, add_sockf_copy_to_array( pool, &sockf) );
	//add duplicated items
	sockf.fs_fd = 800;
	EXPECT_EQ( ERR_ALREADY_EXIST, add_sockf_copy_to_array( pool, &sockf) );
	WRITE_LOG(LOG_DEBUG, "free array..");
	FreeArrayPoolNoZmq(pool);
	WRITE_LOG(LOG_DEBUG, "free array OK");
	WRITE_LOG(LOG_DEBUG, "create array");
}

TEST_F(ZmqNetwTests, TestSockfArrayFinditems) {
	WRITE_LOG(LOG_DEBUG, "TestSockfArrayFinditems::create array");
	struct zeromq_pool *pool = CreateArrayPoolNoZmq();
	EXPECT_NE((void*)NULL, pool);
	WRITE_LOG(LOG_DEBUG, "create array ok");
	struct sock_file_t sockf;
	for (int i=0; i < 100; i++){
		memset(&sockf, '\0', sizeof(struct sock_file_t));
		sockf.fs_fd = i;
		int err = add_sockf_copy_to_array( pool, &sockf);
		if ( ERR_OK != err ){
			WRITE_FMT_LOG(LOG_ERR, "add to array failed err=%d", err);
			FAIL();
		}
	}
	EXPECT_EQ( (struct sock_file_t*)NULL, sockf_by_fd( pool, 101) );
	EXPECT_NE( (struct sock_file_t*)NULL, sockf_by_fd( pool, 1) );
	WRITE_LOG(LOG_DEBUG, "free array..");
	FreeArrayPoolNoZmq(pool);
	WRITE_LOG(LOG_DEBUG, "free array OK");
	WRITE_LOG(LOG_DEBUG, "create array");
}

TEST_F(ZmqNetwTests, TestInitTermZmqNetwPool) {
	struct zeromq_pool *zpool = (struct zeromq_pool *) malloc(sizeof(struct zeromq_pool *));
	EXPECT_EQ(ERR_OK, init_zeromq_pool(zpool) );
	EXPECT_EQ(ERR_OK, zeromq_term(zpool) );
	free(zpool);
}


TEST_F(ZmqNetwTests, TestSockfOpenClose) {
	int test_items_count = 2;
	struct zeromq_pool *zpool = (struct zeromq_pool *) malloc(sizeof(struct zeromq_pool *));
	EXPECT_EQ(ERR_OK, init_zeromq_pool(zpool) );
	/*zmq init ok, create db_records struct*/
	struct db_records_t db_records = {NULL, 0, DB_RECORDS_GRANULARITY, 0};
	db_records.array = (struct db_record_t*)malloc( sizeof(struct db_record_t)*db_records.maxcount );
	memset(db_records.array, '\0', sizeof(struct db_record_t)*db_records.maxcount);
	/*add test item into db_records*/
	struct db_record_t *record = &db_records.array[db_records.count++];
	/*set record data*/
	record->fd = 3;
	record->fmode = 'w';
	record->ftype = EFILE_MSQ;
	record->method = EMETHOD_CONNECT;
	record->endpoint = (char*) __endpoint1;
	record->sock = ZMQ_PUSH;
	/*records added*/
	struct sock_file_t* sockf = open_sockf( zpool, &db_records, 3);
	EXPECT_NE( (struct sock_file_t*)NULL, sockf);
	close_sockf(zpool, sockf);
	EXPECT_EQ(ERR_OK, zeromq_term(zpool) );
	free(zpool);
}

TEST_F(ZmqNetwTests, TestSockfCommunicationPushPull) {
	int test_items_count = 2;
	struct zeromq_pool *zpool = (struct zeromq_pool *) malloc(sizeof(struct zeromq_pool *));
	EXPECT_EQ(ERR_OK, init_zeromq_pool(zpool) );
	/*zmq init ok, create db_records struct*/
	struct db_records_t db_records = {NULL, 0, DB_RECORDS_GRANULARITY, 0};
	db_records.array = (struct db_record_t*)malloc( sizeof(struct db_record_t)*db_records.maxcount );
	memset(db_records.array, '\0', sizeof(struct db_record_t)*db_records.maxcount);
	/*add test item into db_records*/
	struct db_record_t *record1 = &db_records.array[db_records.count++];
	/*set record1 data*/
	record1->fd = 3;
	record1->fmode = 'w';
	record1->ftype = EFILE_MSQ;
	record1->method = EMETHOD_CONNECT;
	record1->endpoint = (char*)__endpoint1;
	record1->sock = ZMQ_PUSH;
	struct db_record_t *record2 = &db_records.array[db_records.count++];
	/*set record2 data*/
	record2->fd = 4;
	record2->fmode = 'r';
	record2->ftype = EFILE_MSQ;
	record2->method = EMETHOD_BIND;
	record2->endpoint = (char*)__endpoint1;
	record2->sock = ZMQ_PULL;
	/*records added*/
	struct sock_file_t* w_sockf = open_sockf( zpool, &db_records, 3);
	EXPECT_NE( (struct sock_file_t*)NULL, w_sockf);
	struct sock_file_t* r_sockf = open_sockf( zpool, &db_records, 4);
	EXPECT_NE( (struct sock_file_t*)NULL, r_sockf);
	char *buf = alloc_fill_random(TEST_DATA_SIZE);
	char *buf2 = (char*)malloc(TEST_DATA_SIZE);
	if ( buf ){
		/*write sockf*/
		EXPECT_EQ( TEST_DATA_SIZE, write_sockf(w_sockf, buf, TEST_DATA_SIZE) );
		EXPECT_EQ( TEST_DATA_SIZE, read_sockf(r_sockf, buf2, TEST_DATA_SIZE) );
		EXPECT_EQ( 0, strncmp(buf, buf2, TEST_DATA_SIZE) );
	}
	close_sockf(zpool, w_sockf);
	close_sockf(zpool, r_sockf);
	EXPECT_EQ(ERR_OK, zeromq_term(zpool) );
	free(zpool);
}


TEST_F(ZmqNetwTests, TestSockfCommunicationReqRep) {
	int test_items_count = 2;
	struct zeromq_pool *zpool = (struct zeromq_pool *) malloc(sizeof(struct zeromq_pool *));
	EXPECT_EQ(ERR_OK, init_zeromq_pool(zpool) );
	/*zmq init ok, create db_records struct*/
	struct db_records_t db_records = {NULL, 0, DB_RECORDS_GRANULARITY, 0};
	db_records.array = (struct db_record_t*)malloc( sizeof(struct db_record_t)*db_records.maxcount );
	memset(db_records.array, '\0', sizeof(struct db_record_t)*db_records.maxcount);
	/*add test item into db_records*/
	struct db_record_t *record1 = &db_records.array[db_records.count++];
	/*set record1 data*/
	record1->fd = 3;
	record1->fmode = 'w';
	record1->ftype = EFILE_MSQ;
	record1->method = EMETHOD_CONNECT;
	record1->endpoint = (char*)__endpoint1;
	record1->sock = ZMQ_REQ;
	struct db_record_t *record2 = &db_records.array[db_records.count++];
	/*set record2 data*/
	record2->fd = 4;
	record2->fmode = 'r';
	record2->ftype = EFILE_MSQ;
	record2->method = EMETHOD_CONNECT;
	record2->endpoint = (char*)__endpoint1;
	record2->sock = ZMQ_REQ;
	struct db_record_t *record3 = &db_records.array[db_records.count++];
	/*set record3 data*/
	record3->fd = 5;
	record3->fmode = 'r';
	record3->ftype = EFILE_MSQ;
	record3->method = EMETHOD_BIND;
	record3->endpoint = (char*)__endpoint1;
	record3->sock = ZMQ_REP;
	struct db_record_t *record4 = &db_records.array[db_records.count++];
	/*set record4 data*/
	record4->fd = 6;
	record4->fmode = 'w';
	record4->ftype = EFILE_MSQ;
	record4->method = EMETHOD_BIND;
	record4->endpoint = (char*)__endpoint1;
	record4->sock = ZMQ_REP;
	/*records added*/
	struct sock_file_t* w_sockf_req = open_sockf( zpool, &db_records, 3);
	EXPECT_NE( (struct sock_file_t*)NULL, w_sockf_req);
	struct sock_file_t* r_sockf_req = open_sockf( zpool, &db_records, 4);
	EXPECT_NE( (struct sock_file_t*)NULL, r_sockf_req);
	struct sock_file_t* r_sockf_rep = open_sockf( zpool, &db_records, 5);
	EXPECT_NE( (struct sock_file_t*)NULL, r_sockf_rep);
	struct sock_file_t* w_sockf_rep = open_sockf( zpool, &db_records, 6);
	EXPECT_NE( (struct sock_file_t*)NULL, w_sockf_rep);
	char *buf = alloc_fill_random(TEST_DATA_SIZE);
	char *buf2 = (char*)malloc(TEST_DATA_SIZE);
	if ( buf ){
		/*write sockf*/
		EXPECT_EQ( TEST_DATA_SIZE, write_sockf(w_sockf_req, buf, TEST_DATA_SIZE) );
		EXPECT_EQ( TEST_DATA_SIZE, read_sockf(r_sockf_rep, buf2, TEST_DATA_SIZE) );
		EXPECT_EQ( 0, strncmp(buf, buf2, TEST_DATA_SIZE) );
		memset(buf2, '\0', TEST_DATA_SIZE);
		EXPECT_EQ( TEST_DATA_SIZE, write_sockf(w_sockf_rep, buf, TEST_DATA_SIZE) );
		EXPECT_EQ( TEST_DATA_SIZE, read_sockf(r_sockf_req, buf2, TEST_DATA_SIZE) );
		EXPECT_EQ( 0, strncmp(buf, buf2, TEST_DATA_SIZE) );
	}
	close_sockf(zpool, r_sockf_req);
	close_sockf(zpool, w_sockf_req);
	close_sockf(zpool, w_sockf_rep);
	close_sockf(zpool, r_sockf_rep);
	EXPECT_EQ(ERR_OK, zeromq_term(zpool) );
	free(zpool);
}



int main(int argc, char **argv) {
	OPEN_LOG( "log/zmq_netw_test.log", "netw_test", 0);
	testing::InitGoogleTest(&argc, argv);
	return RUN_ALL_TESTS();
}
