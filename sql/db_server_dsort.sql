create table channels(nodename text, file_type text, sock text, method text, endpoint text, fmode character(1), fpath text, fd int);
insert into channels values('source', 'std', 'STDIN', '', '', 'r', '/in/stdin', 0);
insert into channels values('source', 'std', 'STDOUT', '', '', 'w', '/out/stdout', 1);
insert into channels values('source', 'std', 'STDERR', '', '', 'w', '/out/stderr', 2);
insert into channels values('source', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/histograms', 'w', '/out/histogram', 3);
insert into channels values('source', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/detailed-histogram-%d', 'r', '/in/detailed_histogram_request', 4);
insert into channels values('source', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/detailed-histogram-%d', 'w', '/out/detailed_histogram_reply', 5);
insert into channels values('source', 'msq', 'PULL', 'bind', 'ipc:///tmp/zmq/range-request-%d', 'r', '/in/sequences_request', 6);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-7', 'w', '/out/ranges0', 7);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-8', 'w', '/out/ranges1', 8);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-9', 'w', '/out/ranges2', 9);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-10', 'w', '/out/ranges3', 10);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-11', 'w', '/out/ranges4', 11);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-7', 'r', '/in/ranges0', 12);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-8', 'r', '/in/ranges1', 13);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-9', 'r', '/in/ranges2', 14);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-10', 'r', '/in/ranges3', 15);
insert into channels values('source', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/ranges-11', 'r', '/in/ranges4', 16);

insert into channels values('dest', 'std', 'STDIN', '', '', 'r', '/in/stdin', 0);
insert into channels values('dest', 'std', 'STDOUT', '', '', 'w', '/out/stdout', 1);
insert into channels values('dest', 'std', 'STDERR', '', '', 'w', '/out/stderr', 2);
insert into channels values('dest', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/ranges-%d', 'r', '/in/ranges', 3);
insert into channels values('dest', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/ranges-%d', 'w', '/out/ranges', 4);
insert into channels values('dest', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/sort-result', 'w', '/out/sorted_range', 5);

insert into channels values('manager', 'std', 'STDIN', '', '', 'r', '/in/stdin', 0);
insert into channels values('manager', 'std', 'STDOUT', '', '', 'w', '/out/stdout', 1);
insert into channels values('manager', 'std', 'STDERR', '', '', 'w', '/out/stderr', 2);
insert into channels values('manager', 'msq', 'PULL', 'bind', 'ipc:///tmp/zmq/histograms', 'r', '/in/histograms', 3);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-2', 'w', '/out/detailed_histogram_request0', 4);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-3', 'w', '/out/detailed_histogram_request1', 5);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-4', 'w', '/out/detailed_histogram_request2', 6);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-5', 'w', '/out/detailed_histogram_request3', 7);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-6', 'w', '/out/detailed_histogram_request4', 8);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-2', 'r', '/in/detailed_histogram_reply0', 9);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-3', 'r', '/in/detailed_histogram_reply1', 10);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-4', 'r', '/in/detailed_histogram_reply2', 11);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-5', 'r', '/in/detailed_histogram_reply3', 12);
insert into channels values('manager', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/detailed-histogram-6', 'r', '/in/detailed_histogram_reply4', 13);
insert into channels values('manager', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/range-request-2', 'w', '/out/range-request0', 14);
insert into channels values('manager', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/range-request-3', 'w', '/out/range-request1', 15);
insert into channels values('manager', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/range-request-4', 'w', '/out/range-request2', 16);
insert into channels values('manager', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/range-request-5', 'w', '/out/range-request3', 17);
insert into channels values('manager', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/range-request-6', 'w', '/out/range-request4', 18);
insert into channels values('manager', 'msq', 'PULL', 'bind', 'ipc:///tmp/zmq/sort-result', 'r', '/in/sorted_range', 19);

insert into channels values('test1PUSH', 'msq', 'PUSH', 'connect', 'ipc:///tmp/zmq/test1', 'w', '/out/test1', 10);
insert into channels values('test1PULL', 'msq', 'PULL', 'bind', 'ipc:///tmp/zmq/test1', 'r', '/in/test1', 11);
insert into channels values('test2REQ', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/test2', 'w', '/out/test2', 12);
insert into channels values('test2REQ', 'msq', 'REQ', 'connect', 'ipc:///tmp/zmq/test2', 'r', '/in/test2', 13);
insert into channels values('test2REP', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/test2', 'r', '/in/test2', 14);
insert into channels values('test2REP', 'msq', 'REP', 'bind', 'ipc:///tmp/zmq/test2', 'w', '/out/test2', 15);

