#pragma once

/**
 * ============================================================================
 * FEATURE 6: GROUP MANAGEMENT
 * ============================================================================
 * Description:
 *   Handles multi-user group creation, unique joining invite codes,
 *   membership management, and group lookups.
 * ============================================================================
 */

#include "05_user_management.hpp"
#include <string>
#include <vector>
#include <sstream>
#include <iomanip>
#include <algorithm>

struct GroupEntity {
    std::string id;
    std::string name;
    std::string createdBy;
    std::string groupCode; // e.g., "GOA2026", "FLAT302"
    std::vector<std::string> memberIds;
};

class GroupManager {
private:
    std::vector<GroupEntity> groups;

    std::string generateGroupId() const {
        int maxId = 0;
        for (const auto& g : groups) {
            if (g.id.rfind("GRP", 0) == 0) {
                try {
                    int num = std::stoi(g.id.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
        std::ostringstream oss;
        oss << "GRP" << std::setw(3) << std::setfill('0') << (maxId + 1);
        return oss.str();
    }

    std::string generateGroupCode(const std::string& name) const {
        std::string prefix = "";
        for (char c : name) {
            if (std::isalnum(static_cast<unsigned char>(c))) {
                prefix += static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
                if (prefix.length() >= 4) break;
            }
        }
        if (prefix.empty()) prefix = "GRP";
        return prefix + std::to_string(100 + (groups.size() * 11));
    }

public:
    GroupManager() = default;

    // 1. CREATE GROUP
    std::string createGroup(const std::string& name, const std::string& creatorUserId) {
        if (name.empty() || creatorUserId.empty()) return "";

        GroupEntity group;
        group.id = generateGroupId();
        group.name = name;
        group.createdBy = creatorUserId;
        group.groupCode = generateGroupCode(name);
        group.memberIds.push_back(creatorUserId);

        groups.push_back(group);
        return group.id;
    }

    // 2. ADD MEMBER TO GROUP
    bool addMember(const std::string& groupId, const std::string& userId) {
        for (auto& g : groups) {
            if (g.id == groupId) {
                // Prevent duplicate members
                if (std::find(g.memberIds.begin(), g.memberIds.end(), userId) == g.memberIds.end()) {
                    g.memberIds.push_back(userId);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    // 3. REMOVE MEMBER
    bool removeMember(const std::string& groupId, const std::string& userId) {
        for (auto& g : groups) {
            if (g.id == groupId) {
                auto it = std::find(g.memberIds.begin(), g.memberIds.end(), userId);
                if (it != g.memberIds.end()) {
                    g.memberIds.erase(it);
                    return true;
                }
                return false;
            }
        }
        return false;
    }

    // 4. FIND BY ID
    const GroupEntity* getGroupById(const std::string& groupId) const {
        for (const auto& g : groups) {
            if (g.id == groupId) return &g;
        }
        return nullptr;
    }

    // 5. FIND BY INVITE CODE
    const GroupEntity* getGroupByCode(const std::string& code) const {
        for (const auto& g : groups) {
            if (g.groupCode == code) return &g;
        }
        return nullptr;
    }

    // 6. CHECK MEMBERSHIP
    bool isMember(const std::string& groupId, const std::string& userId) const {
        const auto* g = getGroupById(groupId);
        if (!g) return false;
        return std::find(g->memberIds.begin(), g->memberIds.end(), userId) != g->memberIds.end();
    }

    const std::vector<GroupEntity>& getAllGroups() const {
        return groups;
    }
};
