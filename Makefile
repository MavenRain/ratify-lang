# Builds the ratify-lang kernel (a port of permit-lang 3d47120 and 18deea2) on the
# tcc-wasm host with TinyCC. See README.md. src/front/lower.c, src/wasm.c and
# src/ir.h are in the build from S3.
TCC ?= tcc
CC ?= cc
CFLAGS = -std=c99 -Wall -Werror -Isrc
CLANG_FLAGS = -std=c99 -Wall -Wextra -Wswitch-enum -Werror -fsyntax-only -Isrc
FRONT = src/front/base.c src/front/lexer.c src/front/parser.c src/front/front.c src/front/eval.c src/front/check.c src/front/lower.c
TARGET = src/wasm.c
SRC = src/main.c $(FRONT) $(TARGET)
FRONT_TOOL = test/fronttool.c $(filter-out src/front/front.c,$(FRONT))
PATH_TOOL = test/pathtool.c $(filter-out src/front/front.c src/front/lower.c,$(FRONT))
NOTRAP_TOOL = test/notraptool.c
# The crypto layer (S7). langc does not link it; only build/cryptotool does.
CRYPTO = src/crypto/keccak.c src/crypto/u256.c src/crypto/bn254.c src/crypto/elgamal.c src/crypto/threshold.c src/crypto/sigma.c src/crypto/dkg.c src/crypto/secp256k1.c src/crypto/xmd.c src/crypto/sha256.c src/crypto/schnorr.c
# The six import bodies are in host/imports.c (S10 U2, Q-S10-U2-3 (a)).
CRYPTO_TOOL = test/cryptotool.c host/imports.c $(CRYPTO)
# The engine (S10 U1, Q-S10-3 (a)): a port of permit-lang host/.
# build/enginetool and build/ratifyd link it.
ENGINE_TOOL = test/enginetool.c host/engine.c host/wasmread.c
ENGINE_API_TOOL = test/engineapitool.c host/engine.c host/wasmread.c
# The T1 host (S10 U2, Q-S10-U2-4 (a)): the engine and the six import bodies.
RATIFYD = host/ratifyd.c host/imports.c host/engine.c host/wasmread.c $(CRYPTO)
# The S10 U3 runner (Part C, Form (ii)): the front end and the domain (as build/langc)
# and the engine and the six import bodies (as build/ratifyd).
DEMO_TOOL = test/demotool.c $(FRONT) build/domain.c host/engine.c host/wasmread.c host/imports.c $(CRYPTO)
HOST_HEADERS = $(wildcard host/*.h)
HEADERS = $(wildcard src/*.h src/front/*.h src/crypto/*.h)

.PHONY: build check check-clang clean

build: build/langc build/fronttool build/pathtool build/notraptool build/cryptotool build/enginetool build/engineapitool build/ratifyd build/demotool

build/domain.c: domain/domain.lang gen/embed.c
	mkdir -p build
	$(TCC) -run gen/embed.c domain/domain.lang build/domain.c

build/langc: $(SRC) $(HEADERS) build/domain.c
	$(TCC) $(CFLAGS) -o build/langc $(SRC) build/domain.c

build/fronttool: $(FRONT_TOOL) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/fronttool $(FRONT_TOOL)

build/pathtool: $(PATH_TOOL) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/pathtool $(PATH_TOOL)

build/notraptool: $(NOTRAP_TOOL) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/notraptool $(NOTRAP_TOOL)

build/cryptotool: $(CRYPTO_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/cryptotool $(CRYPTO_TOOL)

build/enginetool: $(ENGINE_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/enginetool $(ENGINE_TOOL)

build/engineapitool: $(ENGINE_API_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/engineapitool $(ENGINE_API_TOOL)

build/ratifyd: $(RATIFYD) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/ratifyd $(RATIFYD)

build/demotool: $(DEMO_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/demotool $(DEMO_TOOL)

check-clang: build/domain.c
	$(CC) $(CLANG_FLAGS) $(SRC) build/domain.c
	$(CC) $(CLANG_FLAGS) $(FRONT_TOOL)
	$(CC) $(CLANG_FLAGS) $(PATH_TOOL)
	$(CC) $(CLANG_FLAGS) $(NOTRAP_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(CRYPTO_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(ENGINE_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(ENGINE_API_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(RATIFYD)
	$(CC) $(CLANG_FLAGS) -Ihost $(DEMO_TOOL)

check: build/langc build/fronttool build/pathtool build/notraptool build/cryptotool build/enginetool build/engineapitool build/ratifyd build/demotool check-clang
	sh test/gate.sh
	sh test/review.sh
	build/engineapitool
	build/fronttool
	build/pathtool
	build/cryptotool

clean:
	rm -rf build
