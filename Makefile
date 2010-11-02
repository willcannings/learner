CFLAGS=-std=c99 -I src -D_GNU_SOURCE -include configure.h
CC=gcc


# programs
test: test_sparse_vector.o test_vector.o test_paged_file.o tests/test_learner.c
	$(CC) $(CFLAGS) tests/test_learner.c obj/test_sparse_vector.o obj/test_vector.o obj/test_paged_file.o obj/logging.o obj/learner.o obj/sparse_vector.o obj/vector.o obj/matrix.o obj/paged_file.o -lm -o bin/run_tests
	./bin/run_tests

server: client.o server.o keyed_values.o read_thread.o process_thread.o config.o
	$(CC) $(CFLAGS) obj/client.o obj/server.o obj/keyed_values.o obj/read_thread.o obj/process_thread.o obj/learner.o obj/logging.o obj/config.o -ltokyocabinet -o bin/server

client_test: client.o tests/client_test.c
	$(CC) $(CFLAGS) tests/client_test.c obj/client.o obj/learner.o obj/logging.o -o bin/client_test

# cleaning
clean:
	rm -rf bin/*
	rm -rf obj/*
	rm -f configure.h
	rm -f configure_errors


# core
core_headers: src/core/errors.h src/core/globals.h src/core/logging.h src/learner.h
core: logging.o learner.o core_headers
learner.o: logging.o src/core/learner.c core_headers
	$(CC) $(CFLAGS) -c src/core/learner.c -o obj/learner.o

logging.o: src/core/logging.c core_headers
	$(CC) $(CFLAGS) -c src/core/logging.c -o obj/logging.o


# structures
sparse_vector.o: src/structures/sparse_vector.c src/structures/sparse_vector.h core
	$(CC) $(CFLAGS) -c src/structures/sparse_vector.c -o obj/sparse_vector.o

vector.o: src/structures/vector.c src/structures/vector.h core
	$(CC) $(CFLAGS) -c src/structures/vector.c -o obj/vector.o

matrix.o: src/structures/matrix.c src/structures/matrix.h core
	$(CC) $(CFLAGS) -c src/structures/matrix.c -o obj/matrix.o


# data store
paged_file.o: src/datastore/paged_file.c src/datastore/paged_file.h core
	$(CC) $(CFLAGS) -c src/datastore/paged_file.c -o obj/paged_file.o


# distributed
protocol: src/distributed/protocol/protocol.h src/distributed/protocol/protomsg.h \
	src/distributed/protocol/learner_request_message.h src/distributed/protocol/learner_response_message.h
client.o: src/distributed/client/client.c protocol core
	$(CC) $(CFLAGS) -c src/distributed/client/client.c -o obj/client.o

server.o: src/distributed/server/server.c src/distributed/server/server.h src/distributed/server/globals.h src/distributed/server/config.h protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/server.c -o obj/server.o

keyed_values.o: src/distributed/server/keyed_values.c src/distributed/server/server.h protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/keyed_values.c -o obj/keyed_values.o

read_thread.o: src/distributed/server/read_thread.c protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/read_thread.c -o obj/read_thread.o

process_thread.o: src/distributed/server/process_thread.c protocol core
	$(CC) $(CFLAGS) -c src/distributed/server/process_thread.c -o obj/process_thread.o

config.o: src/distributed/server/config.c core
	$(CC) $(CFLAGS) -c src/distributed/server/config.c -o obj/config.o



# tests
test_sparse_vector.o: tests/test_sparse_vector.c tests/tests.h sparse_vector.o matrix.o core
	$(CC) $(CFLAGS) -c tests/test_sparse_vector.c -o obj/test_sparse_vector.o

test_vector.o: tests/test_vector.c tests/tests.h vector.o core
	$(CC) $(CFLAGS) -c tests/test_vector.c -o obj/test_vector.o

test_paged_file.o: tests/test_paged_file.c tests/tests.h paged_file.o core
	$(CC) $(CFLAGS) -c tests/test_paged_file.c -o obj/test_paged_file.o
