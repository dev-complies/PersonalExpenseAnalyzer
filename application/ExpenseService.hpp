#pragma once

#include <string>
#include <vector>
#include <map>

#include "ApplicationState.hpp"
#include "../core/expense_manager.hpp"
#include "../core/budget_manager.hpp"
#include "../core/GroupManager.h"
#include "../core/GroupExpenseManager.h"
#include "../core/BalanceEngine.h"
#include "../core/SettlementEngine.h"
#include "../core/ExpenseAlgorithms.h"

struct BudgetView {
    Budget budget;
    BudgetStatus status;
};

struct GroupView {
    Group group;
    std::vector<User> members;
    std::vector<GroupExpense> expenses;
    std::map<std::string, long long> balances;
    std::vector<SettlementTransfer> settlements;
};

class ExpenseService
{
private:
    ApplicationState& state;

public:
    explicit ExpenseService(ApplicationState& appState);

    // Personal Expenses CRUD
    std::vector<Expense> getAllExpenses() const;

    const Expense* getExpenseById(
        const std::string& expenseId
    ) const;

    std::string createExpense(Expense expense);

    bool updateExpense(
        const std::string& expenseId,
        const Expense& updatedData
    );

    bool removeExpense(const std::string& expenseId);

    // Analytics
    ExpenseAnalytics getExpenseAnalytics() const;
    std::map<std::string, long long> getCategorySpending() const;
    std::map<std::string, long long> getMonthlySpending() const;
    std::map<std::string, long long> getDailySpending() const;
    SpendingWindowResult getHighestSpendingWindow(int days = 7) const;

    // Budgets
    std::vector<BudgetView> getBudgets() const;
    bool addOrUpdateBudget(const std::string& category, long long limitPaise, BudgetPeriod period);
    bool removeBudget(const std::string& category);

    // Groups & Expense Splitting
    std::vector<Group> getAllGroups() const;
    bool getGroupDetails(const std::string& groupId, GroupView& outGroupView) const;
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
    std::vector<User> getAllUsers() const;
    std::string getCurrentUserId() const;
    std::string getUserNameById(const std::string& userId) const;
};
