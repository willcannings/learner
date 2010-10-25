CFLAGS=-std=c99 -I src
CC=gcc

test : test_sparse_vector.o test_vector.o tests/test_learner.c
	$(CC) $(CFLAGS) tests/test_learner.c bin/test_sparse_vector.o bin/test_vector.o bin/logging.o bin/learner.o bin/sparse_vector.o bin/vector.o -o bin/run_tests
	./bin/run_tests


server : bin/client.o bin/server.o bin/keyed_values.o
	$(CC) $(CFLAGS) bin/client.o bin/server.o bin/keyed_values.o bin/learner.o bin/logging.o -ltokyocabinet -o bin/server


# core
core_headers: src/core/errors.h src/core/globals.h src/core/logging.h src/learner.h
core: bin/logging.o bin/learner.o core_headers
bin/learner.o: bin/logging.o src/core/learner.c core_headers
	$(CC) $(CFLAGS) -c src/core/learner.c -o bin/learner.o

bin/logging.o: src/core/logging.c core_headers
	$(CC) $(CFLAGS) -c src/core/logging.c -o bin/logging.o


# structures
bin/sparse_vector.o: src/structures/sparse_vector.c src/structures/sparse_vector.h core
	$(CC) $(CFLAGS) -c src/structures/sparse_vector.c -o bin/sparse_vector.o

bin/vector.o: src/structures/vector.c src/structures/vector.h core
	$(CC) $(CFLAGS) -c src/structures/vector.c -o bin/vector.o


# distributed
protocol: src/distributed/protocol/protocol.h src/distributed/protocol/protomsg.h \
	src/distributed/protocol/learner_request_message.h src/distributed/protocol/learner_response_message.h
bin/client.o: src/distributed/client/client.c protocol core
	$(CC) $(CFLAGS) -c src/distributed/client/client.c -o bin/client.o

bin/server.o: src/distributed/server/server.c src/distributed/server/server.h protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/server.c -o bin/server.o

bin/keyed_values.o: src/distributed/server/keyed_values.c src/distributed/server/server.h protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/keyed_values.c -o bin/keyed_values.o


# tests
test_sparse_vector.o: tests/test_sparse_vector.c tests/tests.h bin/sparse_vector.o core
	$(CC) $(CFLAGS) -c tests/test_sparse_vector.c -o bin/test_sparse_vector.o

test_vector.o: tests/test_vector.c tests/tests.h bin/vector.o core
	$(CC) $(CFLAGS) -c tests/test_vector.c -o bin/test_vector.o
