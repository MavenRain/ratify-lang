# Builds the ratify-lang kernel (a port of permit-lang 3d47120 and 18deea2) on the
# tcc-wasm host with TinyCC. See README.md. src/front/lower.c, src/wasm.c and
# src/ir.h are in the build from S3.
TCC ?= tcc
CC ?= cc
CFLAGS = -std=c99 -Wall -Werror -Isrc
CLANG_FLAGS = -std=c99 -Wall -Wextra -Wswitch-enum -Werror -fsyntax-only -Isrc
FRONT = src/front/base.c src/front/lexer.c src/front/parser.c src/front/front.c src/front/eval.c src/front/check.c src/front/lower.c src/front/fix.c
TARGET = src/wasm.c
# The JSON report (S11, Q-S11-3 (a)): src/jsonout.c, host/jsonin.c and host/jcs.c
# are copies of permit-lang; src/front/report.c writes the report lines.
REPORT = src/front/report.c src/jsonout.c host/jsonin.c host/jcs.c
# S12 (Q-S12-8 (a)): src/front/tfhe.c builds the T2 TFHE tally circuits.
TFHE = src/front/tfhe.c
# S13 (Q-S13-3 (c)): src/front/noir.c writes the T3 Noir text of `langc noir`.
NOIR = src/front/noir.c
# S14 (Q-S14-8 (a)): src/front/fmt.c prints the canonical text of `langc fmt`.
FMT = src/front/fmt.c
SRC = src/main.c src/mcp.c $(FRONT) $(TARGET) $(REPORT) $(TFHE) $(NOIR) $(FMT)
FRONT_TOOL = test/fronttool.c $(filter-out src/front/front.c,$(FRONT))
# The AST dump of the fmt laws (S14 U2).
AST_TOOL = test/asttool.c $(filter-out src/front/front.c src/front/lower.c,$(FRONT))
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
# The write errors of langc report (S11 U3): a port of permit-lang test/reporttool.c.
REPORT_TOOL = test/reporttool.c
# The string escapes of src/jsonout.c (S11 U4, Q-S11-10 (a)).
JSON_TOOL = test/jsontool.c src/jsonout.c host/jsonin.c host/jcs.c src/front/base.c
# The T2 circuit simulator (S12 U3, Q-S12-6 (a)).
TFHE_TOOL = test/tfhetool.c host/jsonin.c src/front/base.c
# The rows of langc check --json and the fix table (S15 U3, Q-S15-6 (a)).
CHECK_JSON_TOOL = test/checkjsontool.c src/front/fix.c host/jsonin.c host/jcs.c src/front/base.c
# The rows of langc mcp (S16 U3, Q-S16-8 (a)).
MCP_TOOL = test/mcptool.c
SKILL_TOOL = test/skilltool.c
HOST_HEADERS = $(wildcard host/*.h)
HEADERS = $(wildcard src/*.h src/front/*.h src/crypto/*.h)

.PHONY: build check check-clang clean

build: build/langc build/fronttool build/asttool build/pathtool build/notraptool build/cryptotool build/enginetool build/engineapitool build/ratifyd build/demotool build/reporttool build/jsontool build/tfhetool build/checkjsontool build/mcptool build/skilltool

build/domain.c: domain/domain.lang gen/embed.c
	mkdir -p build
	$(TCC) -run gen/embed.c domain/domain.lang build/domain.c

build/langc: $(SRC) $(HEADERS) $(HOST_HEADERS) build/domain.c
	$(TCC) $(CFLAGS) -Ihost -o build/langc $(SRC) build/domain.c

build/fronttool: $(FRONT_TOOL) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/fronttool $(FRONT_TOOL)

build/asttool: $(AST_TOOL) $(HEADERS)
	$(TCC) $(CFLAGS) -o build/asttool $(AST_TOOL)

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

build/reporttool: $(REPORT_TOOL)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/reporttool $(REPORT_TOOL)

build/jsontool: $(JSON_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/jsontool $(JSON_TOOL)

build/tfhetool: $(TFHE_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/tfhetool $(TFHE_TOOL)

build/checkjsontool: $(CHECK_JSON_TOOL) $(HOST_HEADERS) $(HEADERS)
	mkdir -p build
	$(TCC) $(CFLAGS) -Ihost -o build/checkjsontool $(CHECK_JSON_TOOL)

build/mcptool: $(MCP_TOOL)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/mcptool $(MCP_TOOL)

build/skilltool: $(SKILL_TOOL)
	mkdir -p build
	$(TCC) $(CFLAGS) -o build/skilltool $(SKILL_TOOL)

check-clang: build/domain.c
	$(CC) $(CLANG_FLAGS) -Ihost $(SRC) build/domain.c
	$(CC) $(CLANG_FLAGS) $(FRONT_TOOL)
	$(CC) $(CLANG_FLAGS) $(AST_TOOL)
	$(CC) $(CLANG_FLAGS) $(PATH_TOOL)
	$(CC) $(CLANG_FLAGS) $(NOTRAP_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(CRYPTO_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(ENGINE_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(ENGINE_API_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(RATIFYD)
	$(CC) $(CLANG_FLAGS) -Ihost $(DEMO_TOOL)
	$(CC) $(CLANG_FLAGS) $(REPORT_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(JSON_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(TFHE_TOOL)
	$(CC) $(CLANG_FLAGS) -Ihost $(CHECK_JSON_TOOL)
	$(CC) $(CLANG_FLAGS) $(MCP_TOOL)
	$(CC) $(CLANG_FLAGS) $(SKILL_TOOL)

check: build/langc build/fronttool build/asttool build/pathtool build/notraptool build/cryptotool build/enginetool build/engineapitool build/ratifyd build/demotool build/reporttool build/jsontool build/tfhetool build/checkjsontool build/mcptool build/skilltool check-clang
	sh test/gate.sh
	sh test/review.sh
	build/engineapitool
	build/fronttool
	build/pathtool
	build/cryptotool
	build/checkjsontool

clean:
	rm -rf build
