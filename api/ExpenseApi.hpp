#pragma once

#include <string>
#include <vector>
#include "../application/ExpenseService.hpp"

class ExpenseApi
{
private:
    ExpenseService& expenseService;

public:
    explicit ExpenseApi(ExpenseService& service);

    // Personal Expenses CRUD (100% backward compatible with existing contract)
    std::string getAllExpenses() const;

    std::string getExpenseById(
        const std::string& expenseId
    ) const;

    std::string createExpense(Expense expense);

    std::string updateExpense(
        const std::string& expenseId,
        const Expense& updatedData
    );

    std::string deleteExpense(
        const std::string& expenseId
    ) const;

    // Analytics
    std::string getAnalytics() const;

    // Budgets
    std::string getBudgets() const;
    std::string addOrUpdateBudget(const std::string& category, long long limitPaise, int period);
    std::string deleteBudget(const std::string& category);

    // Groups & Splitting
    std::string getAllGroups() const;
    std::string getGroupDetails(const std::string& groupId) const;
    std::string createGroup(const std::string& name, const std::vector<std::string>& memberNames);
    std::string addGroupExpense(
        const std::string& groupId,
        const std::string& title,
        const std::string& category,
        const std::string& paidByUserId,
        long long amountPaise,
        const std::string& date
    );

    // Users
    std::string getAllUsers() const;
};
