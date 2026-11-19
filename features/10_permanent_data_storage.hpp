#pragma once

/**
 * ============================================================================
 * FEATURE 10: PERMANENT DATA STORAGE & FILE I/O
 * ============================================================================
 * Description:
 *   Handles persistent disk storage:
 *   1. RFC-Compliant CSV serialization with field quote-escaping
 *   2. Relational text-based serialization for multi-entity models
 *   3. File loading and error recovery
 * ============================================================================
 */

#include "01_personal_expense_management.hpp"
#include <string>
#include <vector>
#include <fstream>
#include <sstream>

class PermanentDataStorage {
private:
    // Helper: Escapes quotes for CSV RFC standard
    static std::string escapeCSV(const std::string& field) {
        std::string res = "\"";
        for (char c : field) {
            if (c == '"') res += "\"\"";
            else res += c;
        }
        res += "\"";
        return res;
    }

    // Helper: Parses a single CSV line with support for embedded quotes and commas
    static std::vector<std::string> parseCSVLine(const std::string& line) {
        std::vector<std::string> fields;
        std::string cur;
        bool inQuotes = false;

        for (size_t i = 0; i < line.length(); ++i) {
            char c = line[i];
            if (c == '"') {
                if (inQuotes && i + 1 < line.length() && line[i + 1] == '"') {
                    cur += '"';
                    i++; // Skip escaped quote
                } else {
                    inQuotes = !inQuotes;
                }
            } else if (c == ',' && !inQuotes) {
                fields.push_back(cur);
                cur.clear();
            } else {
                cur += c;
            }
        }
        fields.push_back(cur);
        return fields;
    }

public:
    // 1. Save Expenses to CSV File
    static bool saveExpensesToCSV(const std::vector<ExpenseRecord>& expenses, const std::string& filePath) {
        std::ofstream file(filePath);
        if (!file.is_open()) return false;

        // Write header
        file << "id,userId,title,category,amountPaise,date,note\n";

        for (const auto& e : expenses) {
            file << escapeCSV(e.id) << ","
                 << escapeCSV(e.userId) << ","
                 << escapeCSV(e.title) << ","
                 << escapeCSV(e.category) << ","
                 << e.amountPaise << ","
                 << escapeCSV(e.date) << ","
                 << escapeCSV(e.note) << "\n";
        }

        return file.good();
    }

    // 2. Load Expenses from CSV File
    static bool loadExpensesFromCSV(std::vector<ExpenseRecord>& outExpenses, const std::string& filePath) {
        std::ifstream file(filePath);
        if (!file.is_open()) return false;

        outExpenses.clear();
        std::string line;

        // Skip header
        if (!std::getline(file, line)) return true;

        while (std::getline(file, line)) {
            if (line.empty()) continue;
            auto fields = parseCSVLine(line);
            if (fields.size() < 7) continue;

            ExpenseRecord rec;
            rec.id = fields[0];
            rec.userId = fields[1];
            rec.title = fields[2];
            rec.category = fields[3];
            try {
                rec.amountPaise = std::stoll(fields[4]);
            } catch (...) {
                rec.amountPaise = 0;
            }
            rec.date = fields[5];
            rec.note = fields[6];

            outExpenses.push_back(rec);
        }

        return true;
    }
};
