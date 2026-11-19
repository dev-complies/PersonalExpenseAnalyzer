#pragma once

/**
 * ============================================================================
 * FEATURE 1: PERSONAL EXPENSE MANAGEMENT (CRUD)
 * ============================================================================
 * Description:
 *   Handles the core lifecycle of personal expenses: Creating, Reading,
 *   Updating, and Deleting (CRUD).
 * 
 * Key Concepts:
 *   - Auto ID Generation: Formats unique sequential IDs (EXP0001, EXP0002).
 *   - Integer Paise Storage: Eliminates floating-point currency rounding errors.
 *   - Input Validation: Enforces mandatory fields and positive amounts.
 * 
 * Time Complexities:
 *   - Add Expense:    O(1) amortized
 *   - Find by ID:     O(N) linear search
 *   - Update Expense: O(N)
 *   - Delete Expense: O(N)
 * ============================================================================
 */

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct ExpenseRecord {
    std::string id;
    std::string userId;
    std::string title;
    std::string category;
    long long amountPaise; // 1 Rupee = 100 Paise
    std::string date;      // Format: YYYY-MM-DD
    std::string note;
};

class PersonalExpenseManager {
private:
    std::vector<ExpenseRecord> expenses;

    // Helper: Generates unique sequential ID like "EXP0001"
    std::string generateNextId() const {
        int maxId = 0;
        for (const auto& exp : expenses) {
            if (exp.id.rfind("EXP", 0) == 0) {
                try {
                    int num = std::stoi(exp.id.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
        std::ostringstream oss;
        oss << "EXP" << std::setw(4) << std::setfill('0') << (maxId + 1);
        return oss.str();
    }

public:
    PersonalExpenseManager() = default;

    // 1. CREATE
    std::string addExpense(ExpenseRecord record) {
        if (record.title.empty() || record.category.empty() ||
            record.amountPaise <= 0 || record.date.empty()) {
            return ""; // Validation failed
        }
        record.id = generateNextId();
        expenses.push_back(record);
        return record.id;
    }

    // 2. READ ALL
    const std::vector<ExpenseRecord>& getAllExpenses() const {
        return expenses;
    }

    // 3. READ ONE (Find by ID)
    const ExpenseRecord* getExpenseById(const std::string& id) const {
        for (const auto& exp : expenses) {
            if (exp.id == id) return &exp;
        }
        return nullptr;
    }

    // 4. UPDATE
    bool updateExpense(const std::string& id, const ExpenseRecord& updatedData) {
        if (updatedData.title.empty() || updatedData.category.empty() ||
            updatedData.amountPaise <= 0 || updatedData.date.empty()) {
            return false;
        }
        for (auto& exp : expenses) {
            if (exp.id == id) {
                exp.title = updatedData.title;
                exp.category = updatedData.category;
                exp.amountPaise = updatedData.amountPaise;
                exp.date = updatedData.date;
                exp.note = updatedData.note;
                return true;
            }
        }
        return false;
    }

    // 5. DELETE
    bool deleteExpense(const std::string& id) {
        for (auto it = expenses.begin(); it != expenses.end(); ++it) {
            if (it->id == id) {
                expenses.erase(it);
                return true;
            }
        }
        return false;
    }

    size_t count() const { return expenses.size(); }
};
