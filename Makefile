CC = gcc
CFLAGS = -std=c99 -O3 -Wall -Wextra -Wpedantic -mavx2

VARIANTS = sha2-128s sha2-128f sha2-192s sha2-192f sha2-256s sha2-256f \
           sha2-128-24 sha2-192-24 sha2-256-24

SRC_FILES = address.c context_sha2.c fors.c hash_sha2.c hash_sha2x8.c merkle.c \
            sha256avx.c sha256x8.c sha512x4.c sign.c thash_sha2_simple.c \
            thash_sha2_simplex8.c utils.c utilsx8.c wots.c

COMMON_OBJS = build/common/sha2.o build/common/randombytes.o

BINARIES = $(addprefix bench_,$(VARIANTS))

all: $(BINARIES)

# Common object files
build/common/sha2.o: common/sha2.c | build/common
	$(CC) $(CFLAGS) -I common -c -o $@ $<

build/common/randombytes.o: common/randombytes.c | build/common
	$(CC) $(CFLAGS) -I common -c -o $@ $<

build/common:
	mkdir -p build/common

# Per-variant rules via template
define VARIANT_RULE

build/$(1):
	mkdir -p build/$(1)

build/$(1)/%.o: src/%.c | build/$(1)
	$$(CC) $$(CFLAGS) -I variants/$(1) -I src -I common -c -o $$@ $$<

build/$(1)/bench.o: bench.c | build/$(1)
	$$(CC) $$(CFLAGS) -DVARIANT_NAME='"$(1)"' -I variants/$(1) -I src -I common -c -o $$@ $$<

OBJS_$(1) = $$(patsubst %.c,build/$(1)/%.o,$$(SRC_FILES)) build/$(1)/bench.o

bench_$(1): $$(OBJS_$(1)) $$(COMMON_OBJS)
	$$(CC) $$(CFLAGS) -o $$@ $$^

endef

$(foreach v,$(VARIANTS),$(eval $(call VARIANT_RULE,$(v))))

clean:
	rm -rf build $(BINARIES)

.PHONY: all clean
