test:
	cc -std=c99 -I src src/core/learner.c src/core/logging.c src/structures/vector.c src/structures/sparse_vector.c src/distributed/client/client.c tests/test_vector.c tests/test_sparse_vector.c tests/test_learner.c -o bin/run_tests
	./bin/run_tests

server:
    cc -std=c99 -I src src/core/learner.c src/core/logging.c src/structures/vector.c src/structures/sparse_vector.c src/distributed/protocol.c src/distributed/server/server.c src/distributed/server/gazetteer.c -o bin/run_tests
