test:
	cc -std=c99 -I src src/learner.c src/logging.c src/vector.c src/sparse_vector.c tests/test_vector.c tests/test_sparse_vector.c tests/test_learner.c -o bin/run_tests
	./bin/run_tests
