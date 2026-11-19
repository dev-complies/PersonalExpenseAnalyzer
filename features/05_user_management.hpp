#pragma once

/**
 * ============================================================================
 * FEATURE 5: USER MANAGEMENT
 * ============================================================================
 * Description:
 *   Manages system users, authentication identities, and profile directories
 *   used in personal expenses and group sharing.
 * ============================================================================
 */

#include <string>
#include <vector>
#include <sstream>
#include <iomanip>

struct UserProfile {
    std::string id;
    std::string name;
};

class UserManager {
private:
    std::vector<UserProfile> users;

    std::string generateNextUserId() const {
        int maxId = 0;
        for (const auto& u : users) {
            if (u.id.rfind("USR", 0) == 0) {
                try {
                    int num = std::stoi(u.id.substr(3));
                    if (num > maxId) maxId = num;
                } catch (...) {}
            }
        }
        std::ostringstream oss;
        oss << "USR" << std::setw(3) << std::setfill('0') << (maxId + 1);
        return oss.str();
    }

public:
    UserManager() = default;

    // 1. Register or Add User
    std::string addUser(const std::string& name) {
        if (name.empty()) return "";
        UserProfile user;
        user.id = generateNextUserId();
        user.name = name;
        users.push_back(user);
        return user.id;
    }

    void addExistingUser(const std::string& id, const std::string& name) {
        if (!userExists(id)) {
            users.push_back({id, name});
        }
    }

    // 2. Check if User Exists
    bool userExists(const std::string& id) const {
        for (const auto& u : users) {
            if (u.id == id) return true;
        }
        return false;
    }

    // 3. Get User By ID
    const UserProfile* getUserById(const std::string& id) const {
        for (const auto& u : users) {
            if (u.id == id) return &u;
        }
        return nullptr;
    }

    // 4. Get User Name
    std::string getUserName(const std::string& id) const {
        const auto* u = getUserById(id);
        return u ? u->name : id;
    }

    const std::vector<UserProfile>& getAllUsers() const {
        return users;
    }

    size_t count() const { return users.size(); }
};
