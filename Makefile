CXX ?= g++
CXXFLAGS = -std=c++17 -O2 -Wall -Wextra -I. -Iinclude

SRCS = api/server_main.cpp \
       api/ExpenseApi.cpp \
       application/ApplicationState.cpp \
       application/ExpenseService.cpp \
       core/expense_manager.cpp \
       core/budget_manager.cpp \
       core/GroupManager.cpp \
       core/GroupExpenseManager.cpp \
       core/BalanceEngine.cpp \
       core/SettlementEngine.cpp \
       core/SplitEngine.cpp \
       core/ExpenseAlgorithms.cpp \
       storage/ExpenseStorage.cpp \
       storage/storage_manager.cpp

# Platform-specific settings
ifeq ($(OS),Windows_NT)
    TARGET = expense_server.exe
    LDFLAGS = -lws2_32
    RM = del /Q /F
else
    TARGET = expense_server
    LDFLAGS = -pthread
    RM = rm -f
endif

.PHONY: all build run clean test

all: $(TARGET)

build: $(TARGET)

$(TARGET): $(SRCS)
	$(CXX) $(CXXFLAGS) $(SRCS) -o $(TARGET) $(LDFLAGS)

run: $(TARGET)
	./$(TARGET)

test:
	$(CXX) $(CXXFLAGS) api_test.cpp api/ExpenseApi.cpp application/ExpenseService.cpp application/ApplicationState.cpp core/expense_manager.cpp core/budget_manager.cpp core/GroupManager.cpp core/GroupExpenseManager.cpp core/BalanceEngine.cpp core/SettlementEngine.cpp core/SplitEngine.cpp core/ExpenseAlgorithms.cpp storage/ExpenseStorage.cpp storage/storage_manager.cpp -o api_test $(LDFLAGS)
	./api_test
	$(RM) api_test

clean:
	$(RM) $(TARGET) api_test *.o
