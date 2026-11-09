CXX      ?= g++
CXXFLAGS ?= -std=c++17 -Wall -Wextra -I src
LDLIBS   ?= -lsqlite3

APP_SRCS  = src/main.cpp src/finance_tracker.cpp src/storage.cpp
LIB_SRCS  = src/finance_tracker.cpp src/storage.cpp
TEST_SRCS = tests/test_finance_tracker.cpp

all: finance_tracker

finance_tracker: $(APP_SRCS) src/*.h
	$(CXX) $(CXXFLAGS) -o $@ $(APP_SRCS) $(LDLIBS)

test: test_finance_tracker
	./test_finance_tracker

test_finance_tracker: $(LIB_SRCS) $(TEST_SRCS) src/*.h
	$(CXX) $(CXXFLAGS) -o $@ $(LIB_SRCS) $(TEST_SRCS) $(LDLIBS)

clean:
	rm -f finance_tracker test_finance_tracker finance.db test_finance.db

.PHONY: all test clean
