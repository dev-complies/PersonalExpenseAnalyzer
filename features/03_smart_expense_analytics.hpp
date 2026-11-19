#pragma once

/**
 * ============================================================================
 * FEATURE 3: SMART EXPENSE ANALYTICS & SLIDING WINDOW
 * ============================================================================
 * Description:
 *   Computes macro-level financial metrics, multi-dimensional aggregations,
 *   and algorithmic pattern insights.
 * 
 * Algorithms Included:
 *   - Aggregations: Hash Map Bucketing for Categories & Months - O(N)
 *   - Peak Spending Window: Sliding Window Algorithm over Time - O(N)
 * ============================================================================
 */

#include "01_personal_expense_management.hpp"
#include <string>
#include <vector>
#include <map>
#include <algorithm>

struct OverallAnalytics {
    int totalCount = 0;
    long long totalSpendingPaise = 0;
    long long averageExpensePaise = 0;
    ExpenseRecord highestExpense;
    ExpenseRecord lowestExpense;
    bool hasData = false;
};

struct PeakSpendingWindowResult {
    bool found = false;
    std::string startDate;
    std::string endDate;
    long long totalPaise = 0;
};

class SmartExpenseAnalytics {
public:
    // 1. Overall Statistics (Count, Total, Average, Min, Max) - O(N)
    static OverallAnalytics calculateOverall(const std::vector<ExpenseRecord>& list) {
        OverallAnalytics res;
        res.totalCount = static_cast<int>(list.size());
        if (list.empty()) return res;

        res.hasData = true;
        res.highestExpense = list[0];
        res.lowestExpense = list[0];

        for (const auto& e : list) {
            res.totalSpendingPaise += e.amountPaise;
            if (e.amountPaise > res.highestExpense.amountPaise) res.highestExpense = e;
            if (e.amountPaise < res.lowestExpense.amountPaise) res.lowestExpense = e;
        }

        res.averageExpensePaise = res.totalSpendingPaise / res.totalCount;
        return res;
    }

    // 2. Category Breakdown using Hash Map - O(N)
    static std::map<std::string, long long> calculateCategorySpending(const std::vector<ExpenseRecord>& list) {
        std::map<std::string, long long> categoryMap;
        for (const auto& e : list) {
            categoryMap[e.category] += e.amountPaise;
        }
        return categoryMap;
    }

    // 3. Monthly Spending Trend (Groups by YYYY-MM) - O(N)
    static std::map<std::string, long long> calculateMonthlySpending(const std::vector<ExpenseRecord>& list) {
        std::map<std::string, long long> monthlyMap;
        for (const auto& e : list) {
            if (e.date.length() >= 7) {
                std::string month = e.date.substr(0, 7); // e.g., "2026-11"
                monthlyMap[month] += e.amountPaise;
            }
        }
        return monthlyMap;
    }

    // 4. Daily Spending Timeline - O(N)
    static std::map<std::string, long long> calculateDailySpending(const std::vector<ExpenseRecord>& list) {
        std::map<std::string, long long> dailyMap;
        for (const auto& e : list) {
            dailyMap[e.date] += e.amountPaise;
        }
        return dailyMap;
    }

    // 5. SLIDING WINDOW ALGORITHM: Finds Peak Spending Window - O(N)
    // Computes the maximum spending occurred within any consecutive N days
    static PeakSpendingWindowResult findPeakSpendingWindow(const std::vector<ExpenseRecord>& list, int windowDays = 7) {
        PeakSpendingWindowResult result;
        if (list.empty() || windowDays <= 0) return result;

        // Step 1: Bucket expenses by date into sorted map
        auto daily = calculateDailySpending(list);
        std::vector<std::pair<std::string, long long>> sortedDays(daily.begin(), daily.end());

        if (sortedDays.empty()) return result;

        // Step 2: Sliding Window over chronological daily data
        size_t left = 0;
        long long currentWindowSum = 0;
        long long maxWindowSum = -1;

        for (size_t right = 0; right < sortedDays.size(); ++right) {
            currentWindowSum += sortedDays[right].second;

            // In a fixed-index or date-span sliding window:
            if (right >= static_cast<size_t>(windowDays)) {
                currentWindowSum -= sortedDays[left].second;
                left++;
            }

            if (currentWindowSum > maxWindowSum) {
                maxWindowSum = currentWindowSum;
                result.found = true;
                result.startDate = sortedDays[left].first;
                result.endDate = sortedDays[right].first;
                result.totalPaise = maxWindowSum;
            }
        }

        return result;
    }
};
