#pragma once

/**
 * ============================================================================
 * FEATURE 4: BUDGET MANAGEMENT & THRESHOLD ALERTS
 * ============================================================================
 * Description:
 *   Monitors category and overall spending limits against real expenditures,
 *   dynamically evaluating remaining allowances and triggering alerts.
 * 
 * Alert Logic:
 *   - WITHIN_BUDGET: Spending < 80% of limit
 *   - WARNING:       80% <= Spending < 100% of limit
 *   - EXCEEDED:      Spending >= 100% of limit
 * ============================================================================
 */

#include "01_personal_expense_management.hpp"
#include <string>
#include <vector>
#include <cctype>

enum class BudgetPeriodType {
    DAILY,
    WEEKLY,
    MONTHLY
};

enum class BudgetAlertLevel {
    WITHIN_BUDGET, // < 80%
    WARNING,       // 80% to 99.9%
    EXCEEDED       // >= 100%
};

struct BudgetPlan {
    std::string userId;
    std::string category; // "ALL" for global limit, or "Food", "Travel", etc.
    long long limitPaise = 0;
    BudgetPeriodType period = BudgetPeriodType::MONTHLY;
};

struct BudgetEvaluationResult {
    std::string category;
    long long limitPaise = 0;
    long long actualSpendingPaise = 0;
    long long remainingPaise = 0;
    double percentageUsed = 0.0;
    BudgetAlertLevel alert = BudgetAlertLevel::WITHIN_BUDGET;
};

class BudgetManager {
private:
    std::vector<BudgetPlan> budgets;

    static std::string toLower(std::string str) {
        for (char& c : str) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return str;
    }

public:
    BudgetManager() = default;

    // 1. Set or Update Budget
    bool setBudget(const std::string& userId, const std::string& category, long long limitPaise, BudgetPeriodType period = BudgetPeriodType::MONTHLY) {
        if (category.empty() || limitPaise <= 0) return false;

        // Update if already exists
        for (auto& b : budgets) {
            if (toLower(b.category) == toLower(category) && b.userId == userId) {
                b.limitPaise = limitPaise;
                b.period = period;
                return true;
            }
        }

        // Otherwise insert new
        budgets.push_back({userId, category, limitPaise, period});
        return true;
    }

    // 2. Remove Budget
    bool removeBudget(const std::string& category) {
        std::string target = toLower(category);
        for (auto it = budgets.begin(); it != budgets.end(); ++it) {
            if (toLower(it->category) == target) {
                budgets.erase(it);
                return true;
            }
        }
        return false;
    }

    const std::vector<BudgetPlan>& getAllBudgets() const {
        return budgets;
    }

    // 3. Evaluate Expenses Against Budget for a Given Date Range
    static BudgetEvaluationResult evaluate(const BudgetPlan& budget, const std::vector<ExpenseRecord>& expenses,
                                           const std::string& startDate = "", const std::string& endDate = "") {
        BudgetEvaluationResult result;
        result.category = budget.category;
        result.limitPaise = budget.limitPaise;

        std::string targetCat = toLower(budget.category);

        for (const auto& e : expenses) {
            // Match date window if provided
            if (!startDate.empty() && e.date < startDate) continue;
            if (!endDate.empty() && e.date > endDate) continue;

            // Match category (unless "ALL")
            if (targetCat != "all" && toLower(e.category) != targetCat) continue;

            result.actualSpendingPaise += e.amountPaise;
        }

        result.remainingPaise = result.limitPaise - result.actualSpendingPaise;

        if (result.limitPaise > 0) {
            result.percentageUsed = (static_cast<double>(result.actualSpendingPaise) / result.limitPaise) * 100.0;
        }

        // Determine Alert State
        if (result.actualSpendingPaise >= result.limitPaise) {
            result.alert = BudgetAlertLevel::EXCEEDED;
        } else if (result.actualSpendingPaise * 100 >= result.limitPaise * 80) {
            result.alert = BudgetAlertLevel::WARNING;
        } else {
            result.alert = BudgetAlertLevel::WITHIN_BUDGET;
        }

        return result;
    }
};
