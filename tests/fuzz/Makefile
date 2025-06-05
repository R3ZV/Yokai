CXX_FLAGS := -fsanitize=address,fuzzer -std=c++23
CXX := clang++
TARGET := fuzzers/fuzzer
CXX_FILES := ../../daemon/command.cpp \
			 fuzz_target.cpp

fuzz:
	$(CXX) $(CXX_FLAGS) $(CXX_FILES) -o $(TARGET)
	./$(TARGET) -artifact_prefix=crash-reports/ corpus/
