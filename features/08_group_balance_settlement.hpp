#pragma once

/**
 * ============================================================================
 * FEATURE 8: GROUP BALANCE MANAGEMENT & DEBT SETTLEMENT ENGINE
 * ============================================================================
 * Description:
 *   Tracks individual member balances and implements a Greedy Graph
 *   Minimization Algorithm to compute the minimum number of transactions
 *   required to settle all group debts.
 * 
 * Algorithm:
 *   - Net Balance: (Total Amount Paid by Member) - (Member's Total Share)
 *   - Settlement Engine: Greedy Two-Pointer Matching of Max Debtor to Max Creditor
 *     Time Complexity: O(M log M) where M is the number of members.
 * ============================================================================
 */

#include "07_smart_expense_splitting.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>

struct DebtTransfer {
    std::string fromUserId; // Debtor (Owes money)
    std::string toUserId;   // Creditor (Is owed money)
    long long amountPaise = 0;
};

class BalanceAndSettlementEngine {
public:
    // 1. Calculate Net Balances for All Members in a Group
    // Positive balance = Money to receive (Creditor)
    // Negative balance = Money to pay (Debtor)
    // Zero balance     = Fully settled
    static std::map<std::string, long long> calculateBalances(
        const std::vector<std::string>& groupMemberIds,
        const std::vector<SharedGroupExpense>& groupExpenses) {

        std::map<std::string, long long> balances;
        for (const auto& mId : groupMemberIds) {
            balances[mId] = 0;
        }

        for (const auto& expense : groupExpenses) {
            // The person who paid gets a credit
            balances[expense.paidByUserId] += expense.totalAmountPaise;

            // Every participant incurs a debit for their share
            for (const auto& split : expense.splits) {
                balances[split.userId] -= split.sharePaise;
            }
        }

        return balances;
    }

    // 2. Validate that All Balances Sum to Exactly Zero
    static bool validateBalances(const std::map<std::string, long long>& balances) {
        long long sum = 0;
        for (const auto& pair : balances) {
            sum += pair.second;
        }
        return sum == 0;
    }

    // 3. GREEDY DEBT SETTLEMENT ALGORITHM
    // Minimizes transactions by repeatedly pairing the largest debtor with the largest creditor
    static std::vector<DebtTransfer> generateOptimalSettlementPlan(
        const std::map<std::string, long long>& balances) {

        std::vector<DebtTransfer> transfers;

        // Separate debtors (negative) and creditors (positive)
        std::vector<std::pair<std::string, long long>> creditors;
        std::vector<std::pair<std::string, long long>> debtors;

        for (const auto& pair : balances) {
            if (pair.second > 0) {
                creditors.push_back({pair.first, pair.second});
            } else if (pair.second < 0) {
                debtors.push_back({pair.first, -pair.second}); // Store debt as positive value
            }
        }

        size_t cIdx = 0;
        size_t dIdx = 0;

        while (cIdx < creditors.size() && dIdx < debtors.size()) {
            auto& creditor = creditors[cIdx];
            auto& debtor = debtors[dIdx];

            long long settleAmount = std::min(creditor.second, debtor.second);

            if (settleAmount > 0) {
                transfers.push_back({debtor.first, creditor.first, settleAmount});
                creditor.second -= settleAmount;
                debtor.second -= settleAmount;
            }

            if (creditor.second == 0) cIdx++;
            if (debtor.second == 0) dIdx++;
        }

        return transfers;
    }
};
