
#include "ApplicationState.hpp"

#include "../storage/ExpenseStorage.h"
#include "../storage/storage_manager.hpp"

#include <algorithm>

using namespace std;

namespace
{
    const string EXPENSE_FILE = "storage/expenses.csv";
}

ApplicationState::ApplicationState()
{
    currentUserId = "USR001";

    // Create the default user.
    User currentUser;
    currentUser.id = currentUserId;
    currentUser.name = "Arsh";

    users.push_back(currentUser);
}

void ApplicationState::load()
{
    // Personal expenses
    loadExpensesFromFile(expenses, EXPENSE_FILE);
    if (expenses.empty())
    {
        loadExpensesFromFile(expenses, "storage/expenses_test.csv");
        if (!expenses.empty())
        {
            saveExpensesToFile(expenses, EXPENSE_FILE);
        }
    }

    // Users
    loadUsers(users, "storage/users.txt");
    if (users.empty())
    {
        loadUsers(users, "storage/users_test.txt");
        if (!users.empty())
        {
            saveUsers(users, "storage/users.txt");
        }
    }

    // Ensure current user exists
    bool userExists = false;

    for (const User& user : users)
    {
        if (user.id == currentUserId)
        {
            userExists = true;
            break;
        }
    }

    if (!userExists)
    {
        User currentUser;
        currentUser.id = currentUserId;
        currentUser.name = "Arsh (You)";

        users.push_back(currentUser);
    }

    // Groups
    loadGroups(groups, "storage/groups.txt");
    if (groups.empty())
    {
        loadGroups(groups, "storage/groups_test.txt");
        if (!groups.empty())
        {
            saveGroups(groups, "storage/groups.txt");
        }
    }

    // Group expenses
    loadGroupExpenses(
        groupExpenses,
        "storage/group_expenses.txt"
    );
    if (groupExpenses.empty())
    {
        loadGroupExpenses(
            groupExpenses,
            "storage/group_expenses_test.txt"
        );
        if (!groupExpenses.empty())
        {
            saveGroupExpenses(groupExpenses, "storage/group_expenses.txt");
        }
    }

    // Settlements
    loadSettlements(
        settlements,
        "storage/settlements.txt"
    );

    // Budgets
    loadBudgets(budgets, "storage/budgets.txt");
    if (budgets.empty())
    {
        budgets = {
            {currentUserId, "Food", 500000, BudgetPeriod::MONTHLY},
            {currentUserId, "Travel", 280000, BudgetPeriod::MONTHLY},
            {currentUserId, "Entertainment", 80000, BudgetPeriod::MONTHLY},
            {currentUserId, "Groceries", 400000, BudgetPeriod::MONTHLY},
            {currentUserId, "ALL", 2500000, BudgetPeriod::MONTHLY}
        };
        saveBudgets(budgets, "storage/budgets.txt");
    }
}

bool ApplicationState::save() const
{
    bool success = true;

    if (!saveExpensesToFile(expenses, EXPENSE_FILE))
        success = false;

    if (!saveUsers(users, "storage/users.txt"))
        success = false;

    if (!saveGroups(groups, "storage/groups.txt"))
        success = false;

    if (!saveGroupExpenses(
            groupExpenses,
            "storage/group_expenses.txt"))
        success = false;

    if (!saveSettlements(
            settlements,
            "storage/settlements.txt"))
        success = false;

    if (!saveBudgets(budgets, "storage/budgets.txt"))
        success = false;

    return success;
}
