#pragma once

/**
 * ============================================================================
 * FEATURE 9: ACTIVITY HISTORY & AUXILIARY DATA STRUCTURES (DSA)
 * ============================================================================
 * Description:
 *   Demonstrates core Data Structures & Algorithms (DSA) implemented from
 *   scratch for internal application state:
 * 
 * Data Structures Implemented:
 *   1. Custom Linked List: Chronological Activity Log (O(1) append)
 *   2. Custom Stack (LIFO): Undo History Engine (O(1) push, pop)
 *   3. Custom Queue (FIFO): Bill Reminder Notification Queue (O(1) enqueue, dequeue)
 * ============================================================================
 */

#include "01_personal_expense_management.hpp"
#include <string>
#include <vector>
#include <iostream>

// ============================================================================
// 1. LINKED LIST: CHRONOLOGICAL ACTIVITY AUDIT LOG
// ============================================================================
struct ActivityNode {
    std::string description;
    std::string timestamp;
    ActivityNode* next = nullptr;
    ActivityNode* prev = nullptr;

    ActivityNode(std::string desc, std::string ts)
        : description(std::move(desc)), timestamp(std::move(ts)), next(nullptr), prev(nullptr) {}
};

class ActivityHistoryList {
private:
    ActivityNode* head = nullptr;
    ActivityNode* tail = nullptr;
    int sizeCount = 0;

public:
    ActivityHistoryList() = default;

    ~ActivityHistoryList() {
        ActivityNode* curr = head;
        while (curr) {
            ActivityNode* next = curr->next;
            delete curr;
            curr = next;
        }
    }

    // Append to end of linked list - O(1)
    void logActivity(const std::string& desc, const std::string& timestamp) {
        auto* newNode = new ActivityNode(desc, timestamp);
        if (!tail) {
            head = tail = newNode;
        } else {
            tail->next = newNode;
            newNode->prev = tail;
            tail = newNode;
        }
        sizeCount++;
    }

    std::vector<std::pair<std::string, std::string>> getAllActivities() const {
        std::vector<std::pair<std::string, std::string>> list;
        ActivityNode* curr = head;
        while (curr) {
            list.push_back({curr->description, curr->timestamp});
            curr = curr->next;
        }
        return list;
    }

    int size() const { return sizeCount; }
};


// ============================================================================
// 2. STACK (LIFO): UNDO TRANSACTION ENGINE
// ============================================================================
enum class ActionType {
    ACTION_ADD,
    ACTION_UPDATE,
    ACTION_DELETE
};

struct UndoCommand {
    ActionType type;
    ExpenseRecord expenseSnapshot;
};

class UndoActionStack {
private:
    std::vector<UndoCommand> stack;

public:
    UndoActionStack() = default;

    // Push action onto stack - O(1)
    void pushAction(ActionType type, const ExpenseRecord& record) {
        stack.push_back({type, record});
    }

    // Pop most recent action - O(1)
    bool popAction(UndoCommand& outCommand) {
        if (stack.empty()) return false;
        outCommand = stack.back();
        stack.pop_back();
        return true;
    }

    bool isEmpty() const { return stack.empty(); }
    size_t size() const { return stack.size(); }
};


// ============================================================================
// 3. QUEUE (FIFO): BILL REMINDER DISPATCH ENGINE
// ============================================================================
struct BillReminderItem {
    std::string title;
    long long dueAmountPaise = 0;
    std::string dueDate;
};

class BillReminderQueue {
private:
    std::vector<BillReminderItem> queue;
    size_t frontIdx = 0;

public:
    BillReminderQueue() = default;

    // Enqueue new reminder - O(1)
    void enqueue(const BillReminderItem& item) {
        queue.push_back(item);
    }

    // Dequeue next upcoming reminder - O(1) amortized
    bool dequeue(BillReminderItem& outItem) {
        if (isEmpty()) return false;
        outItem = queue[frontIdx++];
        // Clean up memory if read half
        if (frontIdx > 50 && frontIdx * 2 >= queue.size()) {
            queue.erase(queue.begin(), queue.begin() + frontIdx);
            frontIdx = 0;
        }
        return true;
    }

    bool isEmpty() const { return frontIdx >= queue.size(); }
    size_t size() const { return queue.size() - frontIdx; }
};
