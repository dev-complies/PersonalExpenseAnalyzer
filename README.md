# Personal Expense Analyzer & Smart Expense Splitter

A C++-based expense management application designed to record, manage, and organize personal expenses. The project includes a REST API backend that allows a frontend application to interact with the expense management system.

## Features

- Create, view, update, and delete expenses.
- Organize expenses by title, category, amount, and date.
- Store expense amounts in paise for precise currency handling.
- File-based data persistence.
- REST API built using C++.
- JSON-based communication between frontend and backend.
- CORS enabled for frontend integration.

## Technology Stack

| Component | Technology |
|---|---|
| Programming Language | C++ |
| Standard | C++17 |
| Backend API | cpp-httplib |
| JSON Handling | nlohmann/json |
| Compiler | GCC |
| Data Storage | Local files |
| API Communication | HTTP + JSON |

## Project Structure

```text
PersonalExpenseAnalyzer/
│
├── api/
│   ├── server_main.cpp
│   ├── ExpenseApi.cpp
│   └── ExpenseApi.hpp
│
├── application/
│   ├── ApplicationState.cpp
│   ├── ApplicationState.hpp
│   ├── ExpenseService.cpp
│   └── ExpenseService.hpp
│
├── core/
├── models/
├── storage/
├── dsa/
├── docs/
│   └── api_contract.md
│
├── main.cpp
├── README.md
└── .gitignore
```

## Prerequisites

- Windows
- MSYS2 UCRT64 environment
- GCC C++ compiler
- nlohmann/json library
- cpp-httplib header available in the project

Install the JSON library in the UCRT64 terminal:

```bash
pacman -S mingw-w64-ucrt-x86_64-nlohmann-json
```

## Build and Run

### Quick Start with Make (macOS / Linux / Windows)

```bash
# Build the server
make build

# Start the server
make run

# Run test suite
make test
```

### Manual Compilation

**On macOS / Linux:**
```bash
clang++ -std=c++17 -O2 -Wall -Wextra -I. -Iinclude \
  api/server_main.cpp \
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
  storage/storage_manager.cpp \
  -o expense_server \
  -pthread
```

**On Windows (MSYS2 UCRT64 / MinGW):**
```bash
g++ -std=c++17 -O2 -Wall -Wextra -I. -Iinclude \
  api/server_main.cpp \
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
  storage/storage_manager.cpp \
  -o expense_server.exe \
  -lws2_32
```

### Start the Server

```bash
./expense_server
```

Once running:
- **Frontend Website:** Open [`http://localhost:8080`](http://localhost:8080) directly in your browser.
- **REST API:** Available at [`http://localhost:8080/api`](http://localhost:8080/api).


## API Endpoints

Base URL:

```text
http://localhost:8080/api
```

| Method | Endpoint | Description |
|---|---|---|
| GET | `/expenses` | Fetch all expenses |
| GET | `/expenses/:id` | Fetch an expense by ID |
| POST | `/expenses` | Create a new expense |
| PUT | `/expenses/:id` | Update an existing expense |
| DELETE | `/expenses/:id` | Delete an expense |

For request and response formats, refer to [`docs/api_contract.md`](docs/api_contract.md).

## Frontend Integration

The backend communicates with the frontend using HTTP requests and JSON responses.

Example:

```javascript
fetch("http://localhost:8080/api/expenses")
  .then(response => response.json())
  .then(result => console.log(result));
```

CORS is enabled on the backend to support frontend requests from a different local development port.

## Important Notes

- The backend must be running before the frontend can communicate with it.
- The frontend should use `http://localhost:8080/api` as its API base URL during local development.
- Expense amounts are represented in paise. Divide by 100 to display rupees.
- Local user data and personal records should not be committed to the public repository.
- The currently implemented HTTP API exposes expense operations. Other project modules should not be assumed to have API endpoints.

## Testing

The following operations have been tested successfully using `curl`:

- GET all expenses
- GET expense by ID
- POST a new expense
- PUT an existing expense
- DELETE an expense

## Future Scope

- Frontend integration
- Expense analytics and visual dashboards
- Budget management integration
- Group expense splitting interface
- Improved authentication and user management

---

**Personal Expense Analyzer & Smart Expense Splitter**

C++ Backend Development Project