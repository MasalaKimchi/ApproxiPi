CXX ?= clang++
PKG_CONFIG ?= pkg-config

CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CPPFLAGS += -Iinclude $(shell $(PKG_CONFIG) --cflags gmp mpfr) -MMD -MP
LDLIBS += $(shell $(PKG_CONFIG) --libs gmp mpfr)

SRC = \
	src/agm.cpp \
	src/binary_splitting.cpp \
	src/bbp.cpp \
	src/benchmark.cpp \
	src/borwein.cpp \
	src/candidate.cpp \
	src/chudnovsky.cpp \
	src/formula_spec.cpp \
	src/format.cpp \
	src/ramanujan.cpp \
	src/verification.cpp

OBJ = $(SRC:src/%.cpp=build/%.o)

.PHONY: all test smoke figures clean

all: bin/satox-bench bin/satox-tests

bin/satox-bench: $(OBJ) build/bench_main.o | bin
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

bin/satox-tests: $(OBJ) build/test_main.o | bin
	$(CXX) $(CXXFLAGS) $^ $(LDLIBS) -o $@

build/%.o: src/%.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/bench_main.o: bench/main.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build/test_main.o: tests/test_main.cpp | build
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

build bin results:
	mkdir -p $@

test: bin/satox-tests
	./bin/satox-tests

smoke: bin/satox-bench | results
	./bin/satox-bench --digits 1000 --guard 25 --out results

figures: bin/satox-bench | results
	./bin/satox-bench --digits 1000,10000,100000,1000000 --guard 25 --trials 3 --out results --candidates formulas/candidates.tsv --formula-dir candidates
	python3 tools/make_figures.py --input results/benchmark.csv --output docs/figures

clean:
	rm -rf build bin

-include $(OBJ:.o=.d) build/bench_main.d build/test_main.d
