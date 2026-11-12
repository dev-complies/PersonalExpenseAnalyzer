#include "ExpenseApi.hpp"

#include <nlohmann/json.hpp>
#include <sstream>
#include <iomanip>

using namespace std;
using json = nlohmann::json;

namespace
{
    json expenseToJson(const Expense& expense)
    {
        return json{
            {"id", expense.id},
            {"userId", expense.userId},
            {"title", expense.title},
            {"category", expense.category},
            {"amountPaise", expense.amountPaise},
            {"date", expense.date},
            {"note", expense.note}
        };
    }

    string successResponse(const json& data, const string& message = "Request successful")
    {
        json res = {
            {"success", true},
            {"message", message},
            {"data", data}
        };
        return res.dump();
    }

    string errorResponse(const string& message)
    {
        json res = {
            {"success", false},
            {"message", message},
            {"data", nullptr}
        };
        return res.dump();
    }
}

ExpenseApi::ExpenseApi(ExpenseService& service)
    : expenseService(service)
{
}

string ExpenseApi::getAllExpenses() const
{
    vector<Expense> expenses = expenseService.getAllExpenses();
    json arr = json::array();

    for (const Expense& expense : expenses)
    {
        arr.push_back(expenseToJson(expense));
    }

    return successResponse(arr);
}

string ExpenseApi::getExpenseById(const string& expenseId) const
{
    const Expense* expense = expenseService.getExpenseById(expenseId);

    if (expense == nullptr)
    {
        return errorResponse("Expense not found");
    }

    return successResponse(expenseToJson(*expense));
}

string ExpenseApi::createExpense(Expense expense)
{
    string newId = expenseService.createExpense(expense);

    if (newId.empty())
    {
        return errorResponse("Failed to create expense");
    }

    json data = {
        {"id", newId}
    };

    return successResponse(data);
}

string ExpenseApi::updateExpense(const string& expenseId, const Expense& updatedData)
{
    bool updated = expenseService.updateExpense(expenseId, updatedData);

    if (!updated)
    {
        return errorResponse("Expense not found");
    }

    json data = {
        {"id", expenseId}
    };

    return successResponse(data);
}

string ExpenseApi::deleteExpense(const string& expenseId) const
{
    bool deleted = expenseService.removeExpense(expenseId);

    if (!deleted)
    {
        return errorResponse("Expense not found");
    }

    json data = {
        {"id", expenseId}
    };

    return successResponse(data);
}

string ExpenseApi::getAnalytics() const
{
    ExpenseAnalytics analytics = expenseService.getExpenseAnalytics();
    auto catSpending = expenseService.getCategorySpending();
    auto monthlySpending = expenseService.getMonthlySpending();
    auto dailySpending = expenseService.getDailySpending();
    auto topWindow = expenseService.getHighestSpendingWindow(7);

    json highestObj = nullptr;
    if (analytics.hasExpenses) {
        highestObj = expenseToJson(analytics.highestExpense);
    }

    json lowestObj = nullptr;
    if (analytics.hasExpenses) {
        lowestObj = expenseToJson(analytics.lowestExpense);
    }

    json categoryJson = json::object();
    for (const auto& pair : catSpending) {
        categoryJson[pair.first] = pair.second;
    }

    json monthlyJson = json::object();
    for (const auto& pair : monthlySpending) {
        monthlyJson[pair.first] = pair.second;
    }

    json dailyJson = json::object();
    for (const auto& pair : dailySpending) {
        dailyJson[pair.first] = pair.second;
    }

    json windowJson = {
        {"found", topWindow.found},
        {"startDate", topWindow.startDate},
        {"endDate", topWindow.endDate},
        {"totalPaise", topWindow.totalPaise}
    };

    json data = {
        {"totalExpenseCount", analytics.totalExpenseCount},
        {"totalSpendingPaise", analytics.totalSpendingPaise},
        {"averageExpensePaise", analytics.averageExpensePaise},
        {"hasExpenses", analytics.hasExpenses},
        {"highestExpense", highestObj},
        {"lowestExpense", lowestObj},
        {"categorySpending", categoryJson},
        {"monthlySpending", monthlyJson},
        {"dailySpending", dailyJson},
        {"highestSpendingWindow7Days", windowJson}
    };

    return successResponse(data);
}

string ExpenseApi::getBudgets() const
{
    auto budgetViews = expenseService.getBudgets();
    json arr = json::array();

    for (const auto& bv : budgetViews) {
        string alertStr = "WITHIN_BUDGET";
        if (bv.status.alert == BudgetAlert::WARNING) alertStr = "WARNING";
        else if (bv.status.alert == BudgetAlert::EXCEEDED) alertStr = "EXCEEDED";

        string periodStr = "MONTHLY";
        if (bv.budget.period == BudgetPeriod::DAILY) periodStr = "DAILY";
        else if (bv.budget.period == BudgetPeriod::WEEKLY) periodStr = "WEEKLY";

        json b = {
            {"userId", bv.budget.userId},
            {"category", bv.budget.category},
            {"limitPaise", bv.budget.limitPaise},
            {"period", periodStr},
            {"actualSpendingPaise", bv.status.actualSpendingPaise},
            {"remainingPaise", bv.status.remainingPaise},
            {"percentageUsed", bv.status.percentageUsed},
            {"alert", alertStr}
        };
        arr.push_back(b);
    }

    return successResponse(arr);
}

string ExpenseApi::addOrUpdateBudget(const string& category, long long limitPaise, int periodInt)
{
    BudgetPeriod period = BudgetPeriod::MONTHLY;
    if (periodInt == 0) period = BudgetPeriod::DAILY;
    else if (periodInt == 1) period = BudgetPeriod::WEEKLY;

    bool ok = expenseService.addOrUpdateBudget(category, limitPaise, period);
    if (!ok) {
        return errorResponse("Failed to set budget");
    }

    return successResponse(json{{"category", category}});
}

string ExpenseApi::deleteBudget(const string& category)
{
    bool ok = expenseService.removeBudget(category);
    if (!ok) {
        return errorResponse("Budget not found");
    }
    return successResponse(json{{"category", category}});
}

string ExpenseApi::getAllGroups() const
{
    auto groups = expenseService.getAllGroups();
    json arr = json::array();

    for (const auto& g : groups) {
        arr.push_back({
            {"id", g.id},
            {"name", g.name},
            {"createdBy", g.createdBy},
            {"creatorName", expenseService.getUserNameById(g.createdBy)},
            {"groupCode", g.groupCode},
            {"memberCount", g.memberIds.size()}
        });
    }

    return successResponse(arr);
}

string ExpenseApi::getGroupDetails(const string& groupId) const
{
    GroupView gv;
    if (!expenseService.getGroupDetails(groupId, gv)) {
        return errorResponse("Group not found");
    }

    json membersArr = json::array();
    for (const auto& m : gv.members) {
        membersArr.push_back({
            {"id", m.id},
            {"name", m.name}
        });
    }

    json expensesArr = json::array();
    for (const auto& ge : gv.expenses) {
        json splitsArr = json::array();
        for (const auto& sp : ge.splits) {
            splitsArr.push_back({
                {"userId", sp.userId},
                {"userName", expenseService.getUserNameById(sp.userId)},
                {"sharePaise", sp.sharePaise}
            });
        }

        expensesArr.push_back({
            {"id", ge.id},
            {"groupId", ge.groupId},
            {"description", ge.description},
            {"category", ge.category},
            {"paidByUserId", ge.paidByUserId},
            {"paidByName", expenseService.getUserNameById(ge.paidByUserId)},
            {"amountPaise", ge.amountPaise},
            {"date", ge.date},
            {"splits", splitsArr}
        });
    }

    json balancesArr = json::array();
    for (const auto& pair : gv.balances) {
        balancesArr.push_back({
            {"userId", pair.first},
            {"userName", expenseService.getUserNameById(pair.first)},
            {"netBalancePaise", pair.second}
        });
    }

    json settlementsArr = json::array();
    for (const auto& st : gv.settlements) {
        settlementsArr.push_back({
            {"fromUserId", st.fromUserId},
            {"fromUserName", expenseService.getUserNameById(st.fromUserId)},
            {"toUserId", st.toUserId},
            {"toUserName", expenseService.getUserNameById(st.toUserId)},
            {"amountPaise", st.amountPaise}
        });
    }

    json data = {
        {"id", gv.group.id},
        {"name", gv.group.name},
        {"groupCode", gv.group.groupCode},
        {"createdBy", gv.group.createdBy},
        {"creatorName", expenseService.getUserNameById(gv.group.createdBy)},
        {"members", membersArr},
        {"expenses", expensesArr},
        {"balances", balancesArr},
        {"settlements", settlementsArr}
    };

    return successResponse(data);
}

string ExpenseApi::createGroup(const string& name, const vector<string>& memberNames)
{
    string groupId = expenseService.createGroup(name, memberNames);
    if (groupId.empty()) {
        return errorResponse("Failed to create group");
    }

    return successResponse(json{{"id", groupId}});
}

string ExpenseApi::addGroupExpense(
    const string& groupId,
    const string& title,
    const string& category,
    const string& paidByUserId,
    long long amountPaise,
    const string& date
)
{
    string geId = expenseService.addGroupExpense(groupId, title, category, paidByUserId, amountPaise, date);
    if (geId.empty()) {
        return errorResponse("Failed to add group expense");
    }

    return successResponse(json{{"id", geId}});
}

string ExpenseApi::getAllUsers() const
{
    auto users = expenseService.getAllUsers();
    json arr = json::array();
    for (const auto& u : users) {
        arr.push_back({
            {"id", u.id},
            {"name", u.name}
        });
    }
    return successResponse(arr);
}
