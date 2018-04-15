BUILD_DIR = ./build/
BIN_DIR = ./bin/

all: $(BIN_DIR)/traceroute

$(BIN_DIR)/traceroute: $(BUILD_DIR)/utils.o $(BUILD_DIR)/traceroute.o
	@$(CC) $^ -o $@

$(BUILD_DIR)/%.o: %.c dirs
	@$(CC) -c $< -o $@

.PHONY: build_dir bin_dir dirs clean test_utils test all

clean:
	rm -rf ${BUILD_DIR}
	rm -rf ${BIN_DIR}

build_dir:
	@mkdir -p $(BUILD_DIR)

bin_dir:
	@mkdir -p $(BIN_DIR)

dirs: build_dir bin_dir

test_utils: $(BUILD_DIR)/test_utils.o $(BUILD_DIR)/utils.o
	@$(CC) $^ -o $(BIN_DIR)/$@
	$(BIN_DIR)/$@

test_traceroute: $(BUILD_DIR)/test_traceroute.o
	@$(CC) $^ -o $(BIN_DIR)/$@
	$(BIN_DIR)/$@

test: test_utils test_traceroute

