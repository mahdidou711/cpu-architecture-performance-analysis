# Processor Performance Benchmarks
#
# Build, run, and test the C benchmark programs. The default build uses
# N=1000, TYPE=float, BL=32 and M=16. The functional tests use smaller
# dimensions (N=32 and N=33, BL=8) so that they can be rerun quickly.
#
# Examples:
#   make                 build all programs
#   make test            run the functional tests
#   make test-sanitize   run the functional tests with ASan/UBSan
#   make check           run tests plus a short smoke benchmark
#   make run             run a short benchmark
#   make bench           run a quick benchmark (N=32)
#   make bench-full      run the full benchmark (N=1000)
#   make clean           remove build artefacts
#   make help            show this help

CC ?= gcc
CFLAGS ?= -O2 -std=gnu11 -Wall -Wextra -Werror
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lm

BUILD_DIR ?= build
BIN_DIR := $(BUILD_DIR)/bin

# Default dimensions for the benchmark programs.
N ?= 1000
TYPE ?= float
BL ?= 32
M ?= 16

# Dimensions for the functional tests.
TEST_N ?= 32
TEST_TYPE ?= float
TEST_BL ?= 8
TEST_M ?= 4

SRC := src
TESTS := tests

BINARIES := $(BIN_DIR)/tp1 $(BIN_DIR)/matrix_extras $(BIN_DIR)/latency \
            $(BIN_DIR)/tsc_probe $(BIN_DIR)/overhead_probe $(BIN_DIR)/cpu_probe \
            $(BIN_DIR)/test_tp1 $(BIN_DIR)/test_matrix

TEST_BINS := $(BIN_DIR)/test_tp1_float_32 $(BIN_DIR)/test_tp1_float_33 \
             $(BIN_DIR)/test_tp1_int_32 $(BIN_DIR)/test_tp1_int_33 \
             $(BIN_DIR)/test_tp1_double_32 $(BIN_DIR)/test_tp1_double_33 \
             $(BIN_DIR)/test_matrix_float_32 $(BIN_DIR)/test_matrix_float_33 \
             $(BIN_DIR)/test_matrix_int_32 $(BIN_DIR)/test_matrix_int_33 \
             $(BIN_DIR)/test_matrix_double_32 $(BIN_DIR)/test_matrix_double_33

.PHONY: all clean test test-sanitize check run bench bench-full help

all: $(BINARIES)

$(BIN_DIR):
	mkdir -p $@

$(BIN_DIR)/tp1: $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(N) -DTYPE=$(TYPE) -DBL=$(BL) -DM=$(M) -o $@ $(SRC)/tp1.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/matrix_extras: $(SRC)/matrix_extras.c $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(N) -DTYPE=$(TYPE) -DBL=$(BL) -DM=$(M) -o $@ $(SRC)/matrix_extras.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/latency: $(SRC)/latency.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/latency.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/tsc_probe: $(SRC)/tsc_probe.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/tsc_probe.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/overhead_probe: $(SRC)/overhead_probe.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/overhead_probe.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/cpu_probe: $(SRC)/cpu_probe.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/cpu_probe.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/test_tp1: $(TESTS)/test_tp1.c $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(TEST_N) -DTYPE=$(TEST_TYPE) -DBL=$(TEST_BL) -DM=$(TEST_M) -o $@ $(TESTS)/test_tp1.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/test_matrix: $(TESTS)/test_matrix.c $(SRC)/matrix_extras.c $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(TEST_N) -DTYPE=$(TEST_TYPE) -DBL=$(TEST_BL) -DM=$(TEST_M) -o $@ $(TESTS)/test_matrix.c $(LDFLAGS) $(LDLIBS)

# The 12 variants are built by the same pattern rule; the suffix encodes the
# type and the matrix dimension.
$(BIN_DIR)/test_tp1_%: $(TESTS)/test_tp1.c $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(word 2,$(subst _, ,$*)) -DTYPE=$(word 1,$(subst _, ,$*)) -DBL=8 -DM=4 -o $@ $(TESTS)/test_tp1.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/test_matrix_%: $(TESTS)/test_matrix.c $(SRC)/matrix_extras.c $(SRC)/tp1.c $(SRC)/tsc.h | $(BIN_DIR)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(word 2,$(subst _, ,$*)) -DTYPE=$(word 1,$(subst _, ,$*)) -DBL=8 -DM=4 -o $@ $(TESTS)/test_matrix.c $(LDFLAGS) $(LDLIBS)

test: $(TEST_BINS)
	@set -e; \
	for type in float int double; do \
	  for n in 32 33; do \
	    echo "== test_tp1 type=$$type N=$$n =="; \
	    $(BIN_DIR)/test_tp1_$${type}_$${n} >/dev/null; \
	    echo "== test_matrix type=$$type N=$$n =="; \
	    $(BIN_DIR)/test_matrix_$${type}_$${n} >/dev/null; \
	  done; \
	done

test-sanitize:
	@$(MAKE) test CFLAGS="-O1 -g -std=gnu11 -Wall -Wextra -Werror -fsanitize=address,undefined -fno-omit-frame-pointer"

check: test
	@echo "== tsc_probe =="
	$(BIN_DIR)/tsc_probe
	@echo "== overhead_probe =="
	$(BIN_DIR)/overhead_probe 3
	@echo "== cpu_probe =="
	$(BIN_DIR)/cpu_probe
	@echo "== latency int_add_1 =="
	$(BIN_DIR)/latency int_add_1 2000
	@echo "== latency float_add_1 =="
	$(BIN_DIR)/latency float_add_1 2000
	@echo "== matrix_extras smoke =="
	@set -e; \
	for op in transpose trans_mul trans_total ps4 mm_ijk mm_ikj mm_b_ijk ps; do \
	  echo "== matrix_extras $$op =="; \
	  $(BIN_DIR)/matrix_extras $$op; \
	done

run: $(BIN_DIR)/tp1
	@set -e; \
	for op in zero copy_ij copy_ji add_ij add_ji ps mm_ijk mm_ikj mm_b_ijk; do \
	  echo "== tp1 $$op =="; \
	  $(BIN_DIR)/tp1 $$op; \
	done

bench: $(BIN_DIR)/tp1
	@$(MAKE) --no-print-directory run N=32 TYPE=float BL=8 M=4

bench-full: $(BIN_DIR)/tp1
	@$(MAKE) --no-print-directory run N=1000 TYPE=float BL=32 M=16

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Processor Performance Benchmarks"
	@echo "  make                 build all programs"
	@echo "  make test            run the functional tests"
	@echo "  make test-sanitize   run the functional tests with ASan/UBSan"
	@echo "  make check           run tests plus a short smoke benchmark"
	@echo "  make run             run the default benchmark set"
	@echo "  make bench           run a quick benchmark (N=32)"
	@echo "  make bench-full      run the full benchmark (N=1000)"
	@echo "  make clean           remove build artefacts"
	@echo ""
	@echo "Variables: N, TYPE, BL, M, TEST_N, TEST_TYPE, TEST_BL, TEST_M"
