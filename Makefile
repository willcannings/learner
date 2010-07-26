test:
	cc -std=c99 -I src src/vector.c src/sparse_vector.c tests/vector.c tests/test.c -o bin/run_tests
	./bin/run_tests
