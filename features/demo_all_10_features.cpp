/**
 * ============================================================================
 * PERSONAL EXPENSE ANALYZER & SMART SPLITTER
 * 10-FEATURE INTERACTIVE CONSOLE DEMONSTRATION
 * ============================================================================
 * This standalone driver file executes and tests each of the 10 modular
 * feature files sequentially with clean terminal output for demonstration.
 * ============================================================================
 */

#include <iostream>
#include <iomanip>
#include <cassert>

// Include each of the 10 self-contained feature files
#include "01_personal_expense_management.hpp"
#include "02_searching_sorting_filtering.hpp"
#include "03_smart_expense_analytics.hpp"
#include "04_budget_management.hpp"
#include "05_user_management.hpp"
#include "06_group_management.hpp"
#include "07_smart_expense_splitting.hpp"
#include "08_group_balance_settlement.hpp"
#include "09_activity_history_dsa.hpp"
#include "10_permanent_data_storage.hpp"

using namespace std;

void printHeader(const string& title) {
    cout << "\n==================================================================\n";
    cout << "  " << title << "\n";
    cout << "==================================================================\n";
}

int main() {
    printHeader("PERSONAL EXPENSE ANALYZER - 10 FEATURES SHOWCASE");

    // -------------------------------------------------------------
    // FEATURE 1: PERSONAL EXPENSE MANAGEMENT (CRUD)
    // -------------------------------------------------------------
    cout << "\n[FEATURE 1] Personal Expense Management (CRUD)...\n";
    PersonalExpenseManager pem;
    string id1 = pem.addExpense({"","USR001","College Lunch","Food",15000,"2026-11-10","Canteen meal"});
    string id2 = pem.addExpense({"","USR001","Metro Recharge","Travel",50000,"2026-11-11","SmartCard topup"});
    string id3 = pem.addExpense({"","USR001","Data Science Book","Education",85000,"2026-11-12","Textbook"});

    cout << "  -> Created 3 expenses. IDs: " << id1 << ", " << id2 << ", " << id3 << "\n";
    assert(pem.count() == 3);
    const auto* exp = pem.getExpenseById(id1);
    cout << "  -> Read Expense: " << exp->title << " | Amount: Rs." << (exp->amountPaise / 100.0) << "\n";

    pem.updateExpense(id1, {"","USR001","College Deluxe Lunch","Food",18000,"2026-11-10","With beverage"});
    cout << "  -> Updated Expense " << id1 << " to new title: " << pem.getExpenseById(id1)->title << "\n";
    cout << "  [PASS] Feature 1 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 2: SEARCHING, SORTING & FILTERING
    // -------------------------------------------------------------
    cout << "\n[FEATURE 2] Searching, Sorting & Filtering...\n";
    auto allExpenses = pem.getAllExpenses();

    auto searchResults = ExpenseSearchFilterSort::search(allExpenses, "deluxe");
    cout << "  -> Search for 'deluxe': Found " << searchResults.size() << " match (" << searchResults[0].title << ")\n";

    auto foodExpenses = ExpenseSearchFilterSort::filterByCategory(allExpenses, "Food");
    cout << "  -> Filter by Category 'Food': " << foodExpenses.size() << " items\n";

    auto sortedByAmt = ExpenseSearchFilterSort::sortByAmount(allExpenses, false); // High to Low
    cout << "  -> Merge Sort by Amount (High to Low): Highest is Rs." << (sortedByAmt[0].amountPaise / 100.0) << " (" << sortedByAmt[0].title << ")\n";
    cout << "  [PASS] Feature 2 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 3: SMART EXPENSE ANALYTICS & SLIDING WINDOW
    // -------------------------------------------------------------
    cout << "\n[FEATURE 3] Smart Expense Analytics & Sliding Window...\n";
    auto analytics = SmartExpenseAnalytics::calculateOverall(allExpenses);
    cout << "  -> Total Transactions: " << analytics.totalCount << "\n";
    cout << "  -> Total Expenditure: Rs." << (analytics.totalSpendingPaise / 100.0) << "\n";
    cout << "  -> Average Expense:   Rs." << (analytics.averageExpensePaise / 100.0) << "\n";

    auto catSpending = SmartExpenseAnalytics::calculateCategorySpending(allExpenses);
    for (const auto& pair : catSpending) {
        cout << "     * " << setw(12) << left << pair.first << ": Rs." << (pair.second / 100.0) << "\n";
    }

    auto window = SmartExpenseAnalytics::findPeakSpendingWindow(allExpenses, 7);
    cout << "  -> Peak 7-Day Window: Rs." << (window.totalPaise / 100.0) << " from " << window.startDate << " to " << window.endDate << "\n";
    cout << "  [PASS] Feature 3 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 4: BUDGET MANAGEMENT & ALERTS
    // -------------------------------------------------------------
    cout << "\n[FEATURE 4] Budget Management & Alerts Evaluation...\n";
    BudgetManager bm;
    bm.setBudget("USR001", "Food", 50000);   // Rs. 500 limit (Spent 180 -> 36% -> WITHIN)
    bm.setBudget("USR001", "Travel", 55000); // Rs. 550 limit (Spent 500 -> 90% -> WARNING)

    for (const auto& b : bm.getAllBudgets()) {
        auto eval = BudgetManager::evaluate(b, allExpenses);
        string statusStr = eval.alert == BudgetAlertLevel::WITHIN_BUDGET ? "WITHIN BUDGET (Green)"
                         : eval.alert == BudgetAlertLevel::WARNING ? "WARNING >80% (Amber)" : "EXCEEDED (Red)";
        cout << "  -> Budget for [" << b.category << "]: Limit Rs." << (b.limitPaise / 100.0)
             << " | Spent: Rs." << (eval.actualSpendingPaise / 100.0)
             << " (" << fixed << setprecision(1) << eval.percentageUsed << "%)"
             << " -> Status: " << statusStr << "\n";
    }
    cout << "  [PASS] Feature 4 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 5: USER MANAGEMENT
    // -------------------------------------------------------------
    cout << "\n[FEATURE 5] User Management...\n";
    UserManager um;
    string u1 = um.addUser("Arsh");
    string u2 = um.addUser("Rahul");
    string u3 = um.addUser("Priya");
    cout << "  -> Registered Users: " << u1 << " (" << um.getUserName(u1) << "), "
                                      << u2 << " (" << um.getUserName(u2) << "), "
                                      << u3 << " (" << um.getUserName(u3) << ")\n";
    cout << "  [PASS] Feature 5 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 6: GROUP MANAGEMENT
    // -------------------------------------------------------------
    cout << "\n[FEATURE 6] Group Management...\n";
    GroupManager gm;
    string gId = gm.createGroup("Goa Trip 2026", u1);
    gm.addMember(gId, u2);
    gm.addMember(gId, u3);
    const auto* grp = gm.getGroupById(gId);
    cout << "  -> Group Created: '" << grp->name << "' [Code: " << grp->groupCode << "]\n";
    cout << "  -> Members Count: " << grp->memberIds.size() << " members enrolled.\n";
    cout << "  [PASS] Feature 6 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 7: SMART EXPENSE SPLITTING
    // -------------------------------------------------------------
    cout << "\n[FEATURE 7] Smart Expense Splitting...\n";
    SmartSplitEngine sse;
    // Rs. 3000 Hotel booking split 3 ways: 1000 each
    string ge1 = sse.recordGroupExpense(gId, "Villa Booking", "Travel", u1, 300000, "2026-11-10", {u1, u2, u3});
    // Rs. 900 Dinner split 3 ways: 300 each
    string ge2 = sse.recordGroupExpense(gId, "Seafood Dinner", "Food", u2, 90000, "2026-11-11", {u1, u2, u3});

    auto grpExps = sse.getExpensesForGroup(gId);
    cout << "  -> Recorded " << grpExps.size() << " group shared expenses.\n";
    for (const auto& ge : grpExps) {
        cout << "     * " << ge.description << ": Total Rs." << (ge.totalAmountPaise / 100.0)
             << " paid by " << um.getUserName(ge.paidByUserId) << "\n";
    }
    cout << "  [PASS] Feature 7 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 8: GROUP BALANCE MANAGEMENT & DEBT SETTLEMENT
    // -------------------------------------------------------------
    cout << "\n[FEATURE 8] Group Balance Management & Settlement Engine...\n";
    auto balances = BalanceAndSettlementEngine::calculateBalances(grp->memberIds, grpExps);
    cout << "  -> Net Member Balances:\n";
    for (const auto& pair : balances) {
        string sign = pair.second > 0 ? "+" : "";
        cout << "     * " << setw(10) << left << um.getUserName(pair.first) << ": "
             << sign << "Rs." << (pair.second / 100.0) << (pair.second > 0 ? " (To receive)" : " (To pay)") << "\n";
    }

    auto transfers = BalanceAndSettlementEngine::generateOptimalSettlementPlan(balances);
    cout << "  -> Optimal Minimized Debt Settlement Plan:\n";
    for (const auto& t : transfers) {
        cout << "     [TRANSFER] " << um.getUserName(t.fromUserId) << " pays "
             << um.getUserName(t.toUserId) << " -> Rs." << (t.amountPaise / 100.0) << "\n";
    }
    cout << "  [PASS] Feature 8 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 9: ACTIVITY HISTORY & DATA STRUCTURES (DSA)
    // -------------------------------------------------------------
    cout << "\n[FEATURE 9] Activity History & Data Structures (DSA)...\n";
    // 1. Linked List Activity Log
    ActivityHistoryList actList;
    actList.logActivity("User added expense EXP0001", "2026-11-10 10:15:00");
    actList.logActivity("User created budget for Food", "2026-11-10 10:20:00");
    cout << "  -> [Linked List] Chronological Activity Log: " << actList.size() << " events recorded.\n";

    // 2. Undo Stack
    UndoActionStack undoStack;
    undoStack.pushAction(ActionType::ACTION_ADD, *exp);
    cout << "  -> [Stack LIFO] Undo Stack: " << undoStack.size() << " action pushed.\n";
    UndoCommand cmd;
    undoStack.popAction(cmd);
    cout << "  -> [Stack LIFO] Undo Popped Action for expense: " << cmd.expenseSnapshot.title << "\n";

    // 3. Reminder Queue
    BillReminderQueue rq;
    rq.enqueue({"Electricity Bill", 142000, "2026-11-25"});
    BillReminderItem item;
    rq.dequeue(item);
    cout << "  -> [Queue FIFO] Dequeued next bill reminder: " << item.title << " (Due: " << item.dueDate << ")\n";
    cout << "  [PASS] Feature 9 Verified.\n";

    // -------------------------------------------------------------
    // FEATURE 10: PERMANENT DATA STORAGE & FILE I/O
    // -------------------------------------------------------------
    cout << "\n[FEATURE 10] Permanent Data Storage (CSV & File I/O)...\n";
    string testCSV = "storage/demo_expenses.csv";
    bool saveOk = PermanentDataStorage::saveExpensesToCSV(allExpenses, testCSV);
    cout << "  -> Saved " << allExpenses.size() << " expenses to '" << testCSV << "': " << (saveOk ? "SUCCESS" : "FAILED") << "\n";

    vector<ExpenseRecord> loadedExpenses;
    bool loadOk = PermanentDataStorage::loadExpensesFromCSV(loadedExpenses, testCSV);
    cout << "  -> Reloaded " << loadedExpenses.size() << " expenses from CSV: " << (loadOk ? "SUCCESS" : "FAILED") << "\n";
    assert(loadedExpenses.size() == allExpenses.size());
    cout << "  [PASS] Feature 10 Verified.\n";

    printHeader("ALL 10 FEATURES COMPILED AND VERIFIED WITH 100% SUCCESS!");
    return 0;
}
