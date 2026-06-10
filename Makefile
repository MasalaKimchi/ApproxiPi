CXX ?= clang++
PKG_CONFIG ?= pkg-config

CXXFLAGS ?= -std=c++17 -O3 -Wall -Wextra -pedantic
CPPFLAGS += -Iinclude $(shell $(PKG_CONFIG) --cflags gmp mpfr) -MMD -MP
LDLIBS += $(shell $(PKG_CONFIG) --libs gmp mpfr)

# FLINT/Arb is optional: when present, the arb_const_pi external baseline is
# compiled in; otherwise it reports itself as unavailable.
HAVE_FLINT := $(shell $(PKG_CONFIG) --exists flint && echo 1)
ifeq ($(HAVE_FLINT),1)
CPPFLAGS += -DSATOX_HAVE_FLINT $(shell $(PKG_CONFIG) --cflags flint)
LDLIBS += $(shell $(PKG_CONFIG) --libs flint)
endif

SRC = \
	src/agm.cpp \
	src/bbp_algorithm.cpp \
	src/binary_splitting.cpp \
	src/bbp.cpp \
	src/benchmark.cpp \
	src/borwein.cpp \
	src/borwein_cubic.cpp \
	src/candidate.cpp \
	src/checkpoint.cpp \
	src/chudnovsky.cpp \
	src/chudnovsky_baselines.cpp \
	src/chudnovsky_common.cpp \
	src/chudnovsky_crown.cpp \
	src/crown.cpp \
	src/external_baselines.cpp \
	src/formula_spec.cpp \
	src/format.cpp \
	src/machin.cpp \
	src/memory_estimate.cpp \
	src/ramanujan.cpp \
	src/residue_verify.cpp \
	src/resource_monitor.cpp \
	src/run_config.cpp \
	src/tuner.cpp \
	src/verification.cpp

OBJ = $(SRC:src/%.cpp=build/%.o)

.PHONY: all test smoke smoke-engineering figures ablation clean

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

smoke-engineering: bin/satox-bench | results
	./bin/satox-bench --digits 1000 --guard 25 --trials 1 --out results \
		--algorithms chudnovsky_naive,chudnovsky_recurrence,chudnovsky_bs,bbp_hex_extract \
		--ablation no_residues --skip-memory-guard

ablation: bin/satox-bench | results
	./bin/satox-bench --digits 100000 --guard 25 --trials 1 --out results --merge \
		--algorithms chudnovsky_bs,chudnovsky_bs_crown --skip-memory-guard
	./bin/satox-bench --digits 100000 --guard 25 --trials 1 --out results --merge \
		--algorithms chudnovsky_bs --ablation no_gcd --skip-memory-guard
	./bin/satox-bench --digits 100000 --guard 25 --trials 1 --out results --merge \
		--algorithms chudnovsky_bs --ablation no_binary_split --skip-memory-guard

figures: bin/satox-bench | results
	python3 tools/make_figures.py --input results/benchmark.csv --output docs/figures
	python3 tools/make_efficiency_table.py --input results/benchmark.csv --output results/efficiency.md

clean:
	rm -rf build bin

-include $(OBJ:.o=.d) build/bench_main.d build/test_main.d
