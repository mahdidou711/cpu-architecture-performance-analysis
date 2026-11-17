# Processor Performance Benchmarks
#
# Build, run, and test the C benchmark programs.
#
# Build products live in a directory whose name encodes the full build
# configuration (compiler, flags, N, TYPE, BL, M). A change of any of these
# values therefore selects a different set of binaries; stale executables
# built with other parameters are never reused.
#
# Examples:
#   make                 build all programs (N=1000, TYPE=float, BL=32, M=16)
#   make test            run the functional tests (66 checks, a few seconds)
#   make test-sanitize   run the same tests with ASan/UBSan (separate build)
#   make check           tests + smoke runs of the timing probes and matrix harness (reduced sizes)
#   make bench           quick benchmark of kernels (N=32, BL=8, M=4)
#   make bench-full      full benchmark of kernels (N=1000, BL=32, M=16)
#   make run N=256       run kernels with chosen parameters
#   make clean           remove build artefacts
#   make help            show this help

ifeq ($(origin CC),default)
CC := gcc
endif
CFLAGS ?= -O2 -std=gnu11 -Wall -Wextra -Werror
CPPFLAGS ?=
LDFLAGS ?=
LDLIBS ?= -lm

BUILD_DIR ?= build
PYTHON ?= python3

# Dimensions of the benchmark programs (kernels, matrix_extras).
N ?= 1000
TYPE ?= float
BL ?= 32
M ?= 16

# Block size and repetitions of the functional tests. The matrix dimension
# and the element type of each test binary are part of its name
# (test_kernels_<type>_<N>).
TEST_BL ?= 8
TEST_M ?= 4

# Reduced dimensions of the smoke run performed by `make check`.
CHECK_N ?= 64
CHECK_BL ?= 8
CHECK_M ?= 4

SRC := src
TESTS := tests

# --- Build directories ------------------------------------------------------
# The directory name carries a checksum of the whole configuration.
cfg_id = $(shell printf '%s' '$(subst ','\'',$(1))' | cksum | cut -d' ' -f1)

TOOLCHAIN := $(CC)|$(CFLAGS)|$(CPPFLAGS)|$(LDFLAGS)|$(LDLIBS)
PROG_CFG := $(TOOLCHAIN)|N=$(N)|TYPE=$(TYPE)|BL=$(BL)|M=$(M)
TEST_CFG := $(TOOLCHAIN)|BL=$(TEST_BL)|M=$(TEST_M)

BIN_DIR := $(BUILD_DIR)/N$(N)_$(TYPE)_B$(BL)_M$(M)_$(call cfg_id,$(PROG_CFG))/bin
TEST_DIR := $(BUILD_DIR)/tests_$(call cfg_id,$(TEST_CFG))/bin

PROGRAMS := $(BIN_DIR)/kernels $(BIN_DIR)/matrix_extras $(BIN_DIR)/latency \
            $(BIN_DIR)/tsc_probe $(BIN_DIR)/overhead_probe $(BIN_DIR)/cpu_probe

TEST_BINS := $(TEST_DIR)/test_kernels_float_32 $(TEST_DIR)/test_kernels_float_33 \
             $(TEST_DIR)/test_kernels_int_32 $(TEST_DIR)/test_kernels_int_33 \
             $(TEST_DIR)/test_kernels_double_32 $(TEST_DIR)/test_kernels_double_33 \
             $(TEST_DIR)/test_matrix_float_32 $(TEST_DIR)/test_matrix_float_33 \
             $(TEST_DIR)/test_matrix_int_32 $(TEST_DIR)/test_matrix_int_33 \
             $(TEST_DIR)/test_matrix_double_32 $(TEST_DIR)/test_matrix_double_33

SAN_CFLAGS := -O1 -g -std=gnu11 -Wall -Wextra -Werror \
              -fsanitize=address,undefined -fno-sanitize-recover=all \
              -fno-omit-frame-pointer

.PHONY: all clean test test-sanitize sanitize-run check check-run run bench \
        bench-full print-bin-dir figures help

all: $(PROGRAMS)

# --- Programs ---------------------------------------------------------------
$(BIN_DIR)/kernels: $(SRC)/kernels.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(N) -DTYPE=$(TYPE) -DBL=$(BL) -DM=$(M) -o $@ $(SRC)/kernels.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/matrix_extras: $(SRC)/matrix_extras.c $(SRC)/kernels.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(N) -DTYPE=$(TYPE) -DBL=$(BL) -DM=$(M) -o $@ $(SRC)/matrix_extras.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/latency: $(SRC)/latency.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/latency.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/tsc_probe: $(SRC)/tsc_probe.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/tsc_probe.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/overhead_probe: $(SRC)/overhead_probe.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/overhead_probe.c $(LDFLAGS) $(LDLIBS)

$(BIN_DIR)/cpu_probe: $(SRC)/cpu_probe.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -o $@ $(SRC)/cpu_probe.c $(LDFLAGS) $(LDLIBS)

# --- Functional tests -------------------------------------------------------
# The 12 variants are built by two pattern rules; the suffix encodes the type
# and the matrix dimension.
$(TEST_DIR)/test_kernels_%: $(TESTS)/test_kernels.c $(SRC)/kernels.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(word 2,$(subst _, ,$*)) -DTYPE=$(word 1,$(subst _, ,$*)) -DBL=$(TEST_BL) -DM=$(TEST_M) -o $@ $(TESTS)/test_kernels.c $(LDFLAGS) $(LDLIBS)

$(TEST_DIR)/test_matrix_%: $(TESTS)/test_matrix.c $(SRC)/matrix_extras.c $(SRC)/kernels.c $(SRC)/tsc.h
	@mkdir -p $(@D)
	$(CC) $(CFLAGS) $(CPPFLAGS) -DN=$(word 2,$(subst _, ,$*)) -DTYPE=$(word 1,$(subst _, ,$*)) -DBL=$(TEST_BL) -DM=$(TEST_M) -o $@ $(TESTS)/test_matrix.c $(LDFLAGS) $(LDLIBS)

define run_tests
set -e; \
for type in float int double; do \
  for n in 32 33; do \
    echo "== test_kernels type=$$type N=$$n =="; \
    $(TEST_DIR)/test_kernels_$${type}_$${n} >/dev/null; \
    echo "== test_matrix type=$$type N=$$n =="; \
    $(TEST_DIR)/test_matrix_$${type}_$${n} >/dev/null; \
  done; \
done
endef

test: $(TEST_BINS)
	@$(run_tests)

# Sanitizer tests are built in $(BUILD_DIR)/sanitize, never in the directory
# of the ordinary tests. The recipe verifies that every executable really
# links the ASan and UBSan runtimes, then runs it; with
# -fno-sanitize-recover=all any UBSan report aborts the process (non-zero
# exit status), and ASan errors abort by default.
test-sanitize:
	@$(MAKE) --no-print-directory sanitize-run BUILD_DIR=$(BUILD_DIR)/sanitize CFLAGS="$(SAN_CFLAGS)"

sanitize-run: $(TEST_BINS)
	@set -e; \
	for b in $(TEST_BINS); do \
	  nm $$b | grep -q '__asan_init' || { echo "FAIL: $$b is not ASan-instrumented" >&2; exit 1; }; \
	  nm $$b | grep -q '__ubsan_handle' || { echo "FAIL: $$b is not UBSan-instrumented" >&2; exit 1; }; \
	done; \
	echo "instrumentation verified (ASan + UBSan symbols) in $(words $(TEST_BINS)) test binaries"
	@export ASAN_OPTIONS=detect_leaks=1:halt_on_error=1 UBSAN_OPTIONS=halt_on_error=1:print_stacktrace=1; \
	$(run_tests)

# --- Smoke run of every program ---------------------------------------------
# `check` runs the functional tests, then delegates to `check-run`, which
# lists every executable it launches as a prerequisite. The matrix programs
# use reduced dimensions so that the run stays short.
check: test
	@$(MAKE) --no-print-directory check-run N=$(CHECK_N) TYPE=float BL=$(CHECK_BL) M=$(CHECK_M)

check-run: $(BIN_DIR)/tsc_probe $(BIN_DIR)/overhead_probe $(BIN_DIR)/cpu_probe \
           $(BIN_DIR)/latency $(BIN_DIR)/matrix_extras
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
	@echo "== matrix_extras smoke (N=$(N) M=$(M) BL=$(BL)) =="
	@set -e; \
	for op in transpose trans_mul trans_total ps4 mm_ijk mm_ikj mm_b_ijk ps; do \
	  echo "== matrix_extras $$op =="; \
	  $(BIN_DIR)/matrix_extras $$op; \
	done

# --- Benchmarks -------------------------------------------------------------
# `run` builds and executes kernels with the current N, TYPE, BL and M. `bench`
# and `bench-full` only select parameters and delegate to `run`, so the
# executable always matches the announced configuration.
run: $(BIN_DIR)/kernels
	@set -e; \
	for op in zero copy_ij copy_ji add_ij add_ji ps mm_ijk mm_ikj mm_b_ijk; do \
	  echo "== kernels $$op =="; \
	  $(BIN_DIR)/kernels $$op; \
	done

bench:
	@$(MAKE) --no-print-directory run N=32 TYPE=float BL=8 M=4

bench-full:
	@$(MAKE) --no-print-directory run N=1000 TYPE=float BL=32 M=16

print-bin-dir:
	@echo $(BIN_DIR)

figures:
	MPLCONFIGDIR="$(CURDIR)/.mplconfig" $(PYTHON) scripts/plot_results.py

clean:
	rm -rf $(BUILD_DIR)

help:
	@echo "Processor Performance Benchmarks"
	@echo "  make                 build all programs (N, TYPE, BL, M)"
	@echo "  make test            run the functional tests"
	@echo "  make test-sanitize   run the functional tests with ASan/UBSan (separate build)"
	@echo "  make check           tests + smoke runs of the timing probes and matrix harness (reduced sizes)"
	@echo "  make run             run kernels with the current N, TYPE, BL, M"
	@echo "  make bench           quick kernels benchmark (N=32, BL=8, M=4)"
	@echo "  make bench-full      full kernels benchmark (N=1000, BL=32, M=16)"
	@echo "  make print-bin-dir   print the directory of the current configuration"
	@echo "  make figures          regenerate the showcase figures from archived CSV files"
	@echo "  make clean           remove build artefacts"
	@echo ""
	@echo "Variables: CC, CFLAGS, N, TYPE, BL, M, TEST_BL, TEST_M, CHECK_N, CHECK_BL, CHECK_M"
