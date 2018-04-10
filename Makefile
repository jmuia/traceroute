BUILD_DIR = ./build/
BIN_DIR = ./bin/

$(BUILD_DIR)/%.o: %.c dirs
	@$(CC) -c $< -o $@

.PHONY: build_dir bin_dir dirs clean test_utils

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

