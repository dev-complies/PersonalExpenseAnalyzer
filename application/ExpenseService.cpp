#include "ExpenseService.hpp"

#include "../core/expense_manager.hpp"
#include "../core/budget_manager.hpp"
#include "../core/GroupManager.h"
#include "../core/GroupExpenseManager.h"
#include "../core/BalanceEngine.h"
#include "../core/SettlementEngine.h"
#include "../core/SplitEngine.h"
#include "../core/ExpenseAlgorithms.h"

#include <ctime>
#include <iomanip>
#include <sstream>
#include <algorithm>

using namespace std;

namespace {
    string getCurrentMonthPrefix() {
        time_t t = time(nullptr);
        tm* now = localtime(&t);
        ostringstream oss;
        oss << (now->tm_year + 1900) << "-"
            << setw(2) << setfill('0') << (now->tm_mon + 1);
        return oss.str();
    }

    string generateUserId(const vector<User>& users) {
        int maxId = 0;
        for (const auto& u : users) {
            if (u.id.substr(0, 3) == "USR") {
                try {
                    int num = stoi(u.id.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
        ostringstream oss;
        oss << "USR" << setw(3) << setfill('0') << (maxId + 1);
        return oss.str();
    }
}

ExpenseService::ExpenseService(ApplicationState& appState)
    : state(appState)
{
}

vector<Expense> ExpenseService::getAllExpenses() const
{
    return state.expenses;
}

const Expense* ExpenseService::getExpenseById(
    const string& expenseId
) const
{
    return findExpenseById(state.expenses, expenseId);
}

string ExpenseService::createExpense(Expense expense)
{
    // Attach current user ID if not provided
    if (expense.userId.empty()) {
        expense.userId = state.currentUserId;
    }

    string newId = addExpense(state.expenses, expense);

    if (newId.empty())
        return "";

    if (!state.save())
    {
        deleteExpense(state.expenses, newId);
        return "";
    }

    return newId;
}

bool ExpenseService::updateExpense(
    const string& expenseId,
    const Expense& updatedData
)
{
    const Expense* existing =
        findExpenseById(state.expenses, expenseId);

    if (existing == nullptr)
        return false;

    Expense previousExpense = *existing;

    if (!::updateExpense(state.expenses, expenseId, updatedData))
        return false;

    if (!state.save())
    {
        ::updateExpense(
            state.expenses,
            expenseId,
            previousExpense
        );
        return false;
    }

    return true;
}

bool ExpenseService::removeExpense(const string& expenseId)
{
    const Expense* existing =
        findExpenseById(state.expenses, expenseId);

    if (existing == nullptr)
        return false;

    Expense deletedExpense = *existing;

    if (!deleteExpense(state.expenses, expenseId))
        return false;

    if (!state.save())
    {
        state.expenses.push_back(deletedExpense);
        return false;
    }

    return true;
}

ExpenseAnalytics ExpenseService::getExpenseAnalytics() const
{
    return calculateExpenseAnalytics(state.expenses);
}

map<string, long long> ExpenseService::getCategorySpending() const
{
    return calculateCategorySpending(state.expenses);
}

map<string, long long> ExpenseService::getMonthlySpending() const
{
    return calculateMonthlySpending(state.expenses);
}

map<string, long long> ExpenseService::getDailySpending() const
{
    return calculateDailySpending(state.expenses);
}

SpendingWindowResult ExpenseService::getHighestSpendingWindow(int days) const
{
    if (state.expenses.empty()) {
        SpendingWindowResult res;
        res.found = false;
        res.totalPaise = 0;
        return res;
    }
    return findHighestSpendingWindow(state.expenses, days);
}

vector<BudgetView> ExpenseService::getBudgets() const
{
    vector<BudgetView> result;
    string curMonth = getCurrentMonthPrefix();
    string startDate = curMonth + "-01";
    string endDate = curMonth + "-31";

    for (const auto& b : state.budgets) {
        BudgetView bv;
        bv.budget = b;
        bv.status = evaluateBudget(state.expenses, b, startDate, endDate);
        result.push_back(bv);
    }
    return result;
}

bool ExpenseService::addOrUpdateBudget(const string& category, long long limitPaise, BudgetPeriod period)
{
    if (category.empty() || limitPaise <= 0) return false;

    // Remove existing budget for this category if present
    for (auto it = state.budgets.begin(); it != state.budgets.end(); ++it) {
        if (it->category == category) {
            state.budgets.erase(it);
            break;
        }
    }

    Budget b;
    b.userId = state.currentUserId;
    b.category = category;
    b.limitPaise = limitPaise;
    b.period = period;

    state.budgets.push_back(b);
    return state.save();
}

bool ExpenseService::removeBudget(const string& category)
{
    bool removed = false;
    for (auto it = state.budgets.begin(); it != state.budgets.end(); ) {
        if (it->category == category) {
            it = state.budgets.erase(it);
            removed = true;
        } else {
            ++it;
        }
    }
    if (removed) {
        state.save();
    }
    return removed;
}

vector<Group> ExpenseService::getAllGroups() const
{
    return state.groups;
}

bool ExpenseService::getGroupDetails(const string& groupId, GroupView& outGroupView) const
{
    const Group* grp = findGroupById(state.groups, groupId);
    if (!grp) return false;

    outGroupView.group = *grp;

    // Collect members
    for (const auto& mId : grp->memberIds) {
        for (const auto& u : state.users) {
            if (u.id == mId) {
                outGroupView.members.push_back(u);
                break;
            }
        }
    }

    // Collect group expenses
    for (const auto& ge : state.groupExpenses) {
        if (ge.groupId == groupId) {
            outGroupView.expenses.push_back(ge);
        }
    }

    // Calculate balances
    outGroupView.balances = calculateGroupBalances(state.groups, state.groupExpenses, groupId);

    // Calculate settlements
    generateSettlementPlan(outGroupView.balances, outGroupView.settlements);

    return true;
}

string ExpenseService::createGroup(const string& name, const vector<string>& memberNames)
{
    if (name.empty()) return "";

    Group newGroup;
    if (!::createGroup(state.groups, state.users, state.currentUserId, name, newGroup)) {
        return "";
    }

    // Add additional members
    for (const auto& mName : memberNames) {
        if (mName.empty() || mName == "Arsh") continue;
        // Check if user already exists with this name
        string targetUserId;
        for (const auto& u : state.users) {
            if (u.name == mName) {
                targetUserId = u.id;
                break;
            }
        }
        if (targetUserId.empty()) {
            User nu;
            nu.id = generateUserId(state.users);
            nu.name = mName;
            state.users.push_back(nu);
            targetUserId = nu.id;
        }
        ::addMemberToGroup(state.groups, state.users, newGroup.id, targetUserId);
    }

    state.save();
    return newGroup.id;
}

string ExpenseService::addGroupExpense(
    const string& groupId,
    const string& title,
    const string& category,
    const string& paidByUserId,
    long long amountPaise,
    const string& date
)
{
    const Group* grp = findGroupById(state.groups, groupId);
    if (!grp) return "";

    GroupExpense ge;
    ge.id = generateGroupExpenseId(state.groupExpenses);
    ge.groupId = groupId;
    ge.description = title;
    ge.category = category;
    ge.paidByUserId = paidByUserId;
    ge.amountPaise = amountPaise;
    ge.date = date;
    ge.splitType = SplitType::EQUAL;

    for (const auto& mId : grp->memberIds) {
        Split s;
        s.userId = mId;
        s.sharePaise = 0;
        ge.splits.push_back(s);
    }

    if (!calculateExpenseSplits(ge)) {
        return "";
    }

    if (!::addGroupExpense(state.groupExpenses, state.groups, ge)) {
        return "";
    }

    state.save();
    return ge.id;
}

vector<User> ExpenseService::getAllUsers() const
{
    return state.users;
}

string ExpenseService::getCurrentUserId() const
{
    return state.currentUserId;
}

string ExpenseService::getUserNameById(const string& userId) const
{
    for (const auto& u : state.users) {
        if (u.id == userId) return u.name;
    }
    return userId;
}
