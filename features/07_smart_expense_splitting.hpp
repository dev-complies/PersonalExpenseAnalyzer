#pragma once

/**
 * ============================================================================
 * FEATURE 7: SMART EXPENSE SPLITTING
 * ============================================================================
 * Description:
 *   Handles equal and custom division of group expenses among participants,
 *   with exact paise remainder preservation (no fractional currency lost).
 * ============================================================================
 */

#include <string>
#include <vector>
#include <numeric>
#include <sstream>
#include <iomanip>

enum class SplitStrategy {
    EQUAL,
    SELECTED_MEMBERS,
    EXACT_AMOUNT
};

struct SplitShare {
    std::string userId;
    long long sharePaise = 0;
};

struct SharedGroupExpense {
    std::string id;
    std::string groupId;
    std::string description;
    std::string category;
    std::string paidByUserId;
    long long totalAmountPaise = 0;
    std::string date;
    SplitStrategy strategy = SplitStrategy::EQUAL;
    std::vector<SplitShare> splits;
};

class SmartSplitEngine {
private:
    std::vector<SharedGroupExpense> groupExpenses;

    std::string generateNextId() const {
        int maxId = 0;
        for (const auto& ge : groupExpenses) {
            if (ge.id.rfind("GEXP", 0) == 0) {
                try {
                    int num = std::stoi(ge.id.substr(4));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
        std::ostringstream oss;
        oss << "GEXP" << std::setw(3) << std::setfill('0') << (maxId + 1);
        return oss.str();
    }

public:
    SmartSplitEngine() = default;

    // 1. Equal Split Calculation with Paise Remainder Distribution
    static bool calculateEqualSplit(long long totalPaise, std::vector<SplitShare>& participants) {
        if (participants.empty() || totalPaise <= 0) return false;

        size_t count = participants.size();
        long long baseShare = totalPaise / count;
        long long remainder = totalPaise % count; // Extra paise to distribute evenly

        for (size_t i = 0; i < count; ++i) {
            participants[i].sharePaise = baseShare + (i < static_cast<size_t>(remainder) ? 1 : 0);
        }
        return true;
    }

    // 2. Validate Exact Shares Sum Up Exactly to Total
    static bool validateExactSplit(long long totalPaise, const std::vector<SplitShare>& splits) {
        long long sum = 0;
        for (const auto& s : splits) {
            if (s.sharePaise < 0) return false;
            sum += s.sharePaise;
        }
        return sum == totalPaise;
    }

    // 3. Add and Automatically Split Group Expense
    std::string recordGroupExpense(const std::string& groupId, const std::string& description,
                                   const std::string& category, const std::string& paidByUserId,
                                   long long totalAmountPaise, const std::string& date,
                                   const std::vector<std::string>& participantUserIds) {
        if (groupId.empty() || description.empty() || paidByUserId.empty() ||
            totalAmountPaise <= 0 || participantUserIds.empty()) {
            return "";
        }

        SharedGroupExpense expense;
        expense.id = generateNextId();
        expense.groupId = groupId;
        expense.description = description;
        expense.category = category;
        expense.paidByUserId = paidByUserId;
        expense.totalAmountPaise = totalAmountPaise;
        expense.date = date;
        expense.strategy = SplitStrategy::EQUAL;

        // Build splits for all participants
        for (const auto& uId : participantUserIds) {
            expense.splits.push_back({uId, 0});
        }

        // Apply equal split algorithm
        if (!calculateEqualSplit(totalAmountPaise, expense.splits)) {
            return "";
        }

        groupExpenses.push_back(expense);
        return expense.id;
    }

    std::vector<SharedGroupExpense> getExpensesForGroup(const std::string& groupId) const {
        std::vector<SharedGroupExpense> res;
        for (const auto& ge : groupExpenses) {
            if (ge.groupId == groupId) res.push_back(ge);
        }
        return res;
    }

    const std::vector<SharedGroupExpense>& getAllGroupExpenses() const {
        return groupExpenses;
    }
};
