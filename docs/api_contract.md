# Personal Expense Analyzer — API Contract

## 1. Overview

This document describes the HTTP API provided by the C++ backend of Personal Expense Analyzer.

The API allows a frontend application to create, retrieve, update, and delete personal expenses using HTTP requests and JSON.

## 2. Server Configuration

**Base URL:**

```text
http://localhost:8080
```

**API Base Path:**

```text
http://localhost:8080/api
```

The backend listens on `127.0.0.1` at port `8080`.

## 3. Common Response Structure

All API responses use a JSON response envelope.

### Success response

```json
{
  "success": true,
  "message": "Request successful",
  "data": {}
}
```

### Error response

```json
{
  "success": false,
  "message": "Expense not found",
  "data": null
}
```

The frontend should check the `success` field to determine whether the operation succeeded.

## 4. Expense Object

An expense contains the following fields:

| Field | Data Type | Description |
|---|---|---|
| id | String | Unique expense identifier |
| userId | String | Associated user identifier |
| title | String | Expense title |
| category | String | Expense category |
| amountPaise | Integer | Expense amount in paise |
| date | String | Expense date |
| note | String | Additional expense information |

**Currency convention:**

Amounts are stored in paise rather than rupees.

Example:

```text
₹250.50 = 25050 paise
```

The frontend should divide `amountPaise` by 100 to display the amount in rupees.

## 5. API Endpoints

### 5.1 Personal Expenses Endpoints

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/expenses` | Fetch all expenses |
| GET | `/api/expenses/:id` | Fetch an expense by ID |
| POST | `/api/expenses` | Create a new expense |
| PUT | `/api/expenses/:id` | Update an existing expense |
| DELETE | `/api/expenses/:id` | Delete an expense |

### 5.2 Analytics Endpoint

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/analytics` | Overall spending stats, averages, extremes, category & monthly breakdown, and peak 7-day spending window |

### 5.3 Budgets Endpoints

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/budgets` | Fetch active budgets with live spending status, percentage used, and alert status |
| POST | `/api/budgets` | Create or update a budget (`category`, `limitPaise`, `period`) |
| DELETE | `/api/budgets/:category` | Remove a budget |

### 5.4 Groups & Smart Splitter Endpoints

| Method | Endpoint | Description |
|---|---|---|
| GET | `/api/groups` | Fetch all groups with member counts |
| GET | `/api/groups/:id` | Fetch group details, members, expenses, net balances, and minimal settlement plan |
| POST | `/api/groups` | Create a new group with member names |
| POST | `/api/groups/:id/expenses` | Add a group expense with automatic equal split calculation |
| GET | `/api/users` | List all registered users |


**Method:** GET

**Endpoint:**

```http
/api/expenses
```

**Description:** Retrieves all available expenses.

**Example response:**

```json
{
  "success": true,
  "message": "Request successful",
  "data": [
    {
      "id": "EXP0001",
      "userId": "USR001",
      "title": "Lunch",
      "category": "Food",
      "amountPaise": 25050,
      "date": "2026-11-09",
      "note": "Lunch at restaurant"
    }
  ]
}
```

### 5.2 Get Expense by ID

**Method:** GET

**Endpoint:**

```http
/api/expenses/{id}
```

**Example request:**

```http
GET /api/expenses/EXP0001
```

**Description:** Retrieves a particular expense using its unique ID.

**Example response:**

```json
{
  "success": true,
  "message": "Request successful",
  "data": {
    "id": "EXP0001",
    "userId": "USR001",
    "title": "Lunch",
    "category": "Food",
    "amountPaise": 25050,
    "date": "2026-11-09",
    "note": "Lunch at restaurant"
  }
}
```

### 5.3 Create Expense

**Method:** POST

**Endpoint:**

```http
/api/expenses
```

**Headers:**

```http
Content-Type: application/json
```

**Request body:**

```json
{
  "title": "Lunch",
  "category": "Food",
  "amountPaise": 25000,
  "date": "2026-11-12",
  "note": "Lunch with friends"
}
```

**Required fields:**
- `title`
- `category`
- `amountPaise`
- `date`

The `note` field is optional.

**Example response:**

```json
{
  "success": true,
  "message": "Request successful",
  "data": {
    "id": "EXP0007"
  }
}
```

The backend generates the expense ID and associates the expense with the current user.

### 5.4 Update Expense

**Method:** PUT

**Endpoint:**

```http
/api/expenses/{id}
```

**Headers:**

```http
Content-Type: application/json
```

**Example request:**

```http
PUT /api/expenses/EXP0007
```

**Request body:**

```json
{
  "title": "Updated Lunch",
  "category": "Food",
  "amountPaise": 30000,
  "date": "2026-11-12",
  "note": "Updated expense details"
}
```

**Example response:**

```json
{
  "success": true,
  "message": "Request successful",
  "data": {
    "id": "EXP0007"
  }
}
```

### 5.5 Delete Expense

**Method:** DELETE

**Endpoint:**

```http
/api/expenses/{id}
```

**Example request:**

```http
DELETE /api/expenses/EXP0007
```

**Example response:**

```json
{
  "success": true,
  "message": "Request successful",
  "data": {
    "id": "EXP0007"
  }
}
```

If the requested expense does not exist, the API returns:

```json
{
  "success": false,
  "message": "Expense not found",
  "data": null
}
```

## 6. Frontend Integration

The frontend can communicate with the backend using JavaScript Fetch API.

### Fetch all expenses

```javascript
const API_BASE_URL = "http://localhost:8080/api";

async function getExpenses() {
    const response = await fetch(`${API_BASE_URL}/expenses`);
    const result = await response.json();

    if (!result.success) {
        throw new Error(result.message);
    }

    return result.data;
}
```

### Create an expense

```javascript
async function createExpense(expense) {
    const response = await fetch(`${API_BASE_URL}/expenses`, {
        method: "POST",
        headers: {
            "Content-Type": "application/json"
        },
        body: JSON.stringify(expense)
    });

    return await response.json();
}
```

## 7. CORS Configuration

The backend enables Cross-Origin Resource Sharing (CORS) to allow frontend applications running on a different local development port to communicate with the API.

The current configuration allows:
- Methods: GET, POST, PUT, DELETE, OPTIONS
- Headers: Content-Type
- Origins: `*`

## 8. Testing and Verification

The following operations were manually tested using `curl`:

| Operation | Result |
|---|---|
| Retrieve all expenses | Passed |
| Retrieve expense by ID | Passed |
| Create a new expense | Passed |
| Update an existing expense | Passed |
| Delete an expense | Passed |
| Verify deleted expense | Passed |

## 9. Current API Scope

The currently implemented and tested HTTP API supports expense CRUD operations.

Other project modules, such as group expense management, budgets, and user management, are not included in this API contract because their HTTP endpoints have not been verified.

---

**Project:** Personal Expense Analyzer & Smart Expense Splitter  
**Backend:** C++17  
**Communication:** HTTP + JSON