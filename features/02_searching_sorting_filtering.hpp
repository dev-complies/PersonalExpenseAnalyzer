#pragma once

/**
 * ============================================================================
 * FEATURE 2: SEARCHING, SORTING & FILTERING
 * ============================================================================
 * Description:
 *   High-performance search, filtering, and sorting algorithms for
 *   exploring personal financial transactions.
 * 
 * Algorithms Included:
 *   - Search: Case-Insensitive Substring Matcher - O(N * M)
 *   - Merge Sort: Divide-and-Conquer Sorting by Amount - O(N log N)
 *   - Filter by Category / Date Range: Single-Pass Linear Filter - O(N)
 * ============================================================================
 */

#include "01_personal_expense_management.hpp"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>

class ExpenseSearchFilterSort {
private:
    static std::string toLower(std::string str) {
        for (char& c : str) c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
        return str;
    }

    // Merge Sort Helper for sorting by amount
    static void merge(std::vector<ExpenseRecord>& arr, int left, int mid, int right, bool ascending) {
        int n1 = mid - left + 1;
        int n2 = right - mid;

        std::vector<ExpenseRecord> L(arr.begin() + left, arr.begin() + mid + 1);
        std::vector<ExpenseRecord> R(arr.begin() + mid + 1, arr.begin() + right + 1);

        int i = 0, j = 0, k = left;
        while (i < n1 && j < n2) {
            bool condition = ascending ? (L[i].amountPaise <= R[j].amountPaise)
                                       : (L[i].amountPaise >= R[j].amountPaise);
            if (condition) {
                arr[k++] = L[i++];
            } else {
                arr[k++] = R[j++];
            }
        }
        while (i < n1) arr[k++] = L[i++];
        while (j < n2) arr[k++] = R[j++];
    }

    static void mergeSort(std::vector<ExpenseRecord>& arr, int left, int right, bool ascending) {
        if (left < right) {
            int mid = left + (right - left) / 2;
            mergeSort(arr, left, mid, ascending);
            mergeSort(arr, mid + 1, right, ascending);
            merge(arr, left, mid, right, ascending);
        }
    }

public:
    // 1. SEARCH: By Title or Note (Case-Insensitive)
    static std::vector<ExpenseRecord> search(const std::vector<ExpenseRecord>& list, const std::string& query) {
        if (query.empty()) return list;
        std::string q = toLower(query);
        std::vector<ExpenseRecord> results;
        for (const auto& e : list) {
            if (toLower(e.title).find(q) != std::string::npos ||
                toLower(e.note).find(q) != std::string::npos) {
                results.push_back(e);
            }
        }
        return results;
    }

    // 2. FILTER: By Category
    static std::vector<ExpenseRecord> filterByCategory(const std::vector<ExpenseRecord>& list, const std::string& category) {
        if (category == "ALL" || category.empty()) return list;
        std::string target = toLower(category);
        std::vector<ExpenseRecord> results;
        for (const auto& e : list) {
            if (toLower(e.category) == target) {
                results.push_back(e);
            }
        }
        return results;
    }

    // 3. FILTER: By Date Range (Inclusive)
    static std::vector<ExpenseRecord> filterByDateRange(const std::vector<ExpenseRecord>& list, const std::string& startDate, const std::string& endDate) {
        std::vector<ExpenseRecord> results;
        for (const auto& e : list) {
            if (e.date >= startDate && e.date <= endDate) {
                results.push_back(e);
            }
        }
        return results;
    }

    // 4. SORT: By Amount using Divide-and-Conquer Merge Sort - O(N log N)
    static std::vector<ExpenseRecord> sortByAmount(std::vector<ExpenseRecord> list, bool ascending = false) {
        if (!list.empty()) {
            mergeSort(list, 0, static_cast<int>(list.size()) - 1, ascending);
        }
        return list;
    }

    // 5. SORT: By Date (Chronological)
    static std::vector<ExpenseRecord> sortByDate(std::vector<ExpenseRecord> list, bool newestFirst = true) {
        std::sort(list.begin(), list.end(), [newestFirst](const ExpenseRecord& a, const ExpenseRecord& b) {
            return newestFirst ? (a.date > b.date) : (a.date < b.date);
        });
        return list;
    }

    // 6. SORT: By Title Alphabetically
    static std::vector<ExpenseRecord> sortByTitle(std::vector<ExpenseRecord> list, bool ascending = true) {
        std::sort(list.begin(), list.end(), [ascending](const ExpenseRecord& a, const ExpenseRecord& b) {
            std::string tA = toLower(a.title);
            std::string tB = toLower(b.title);
            return ascending ? (tA < tB) : (tA > tB);
        });
        return list;
    }
};
