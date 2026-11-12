#include <iostream>
#include <string>
#include <vector>
#include "httplib.h"

#include "../application/ApplicationState.hpp"
#include "../application/ExpenseService.hpp"
#include "ExpenseApi.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;

int main()
{
    // 1. Load application data
    ApplicationState state;
    state.load();

    // 2. Create service and API objects
    ExpenseService expenseService(state);
    ExpenseApi expenseApi(expenseService);

    // 3. Create HTTP server
    httplib::Server server;

    // Enable CORS for frontend integration
    server.set_default_headers({
        {"Access-Control-Allow-Origin", "*"},
        {"Access-Control-Allow-Methods", "GET, POST, PUT, DELETE, OPTIONS"},
        {"Access-Control-Allow-Headers", "Content-Type, Authorization"}
    });

    // Handle browser preflight requests
    server.Options(R"(/api/.*)",
        [](const httplib::Request&, httplib::Response& res)
        {
            res.status = 204;
        }
    );

    // -------------------------------------------------------------
    // Personal Expenses Endpoints
    // -------------------------------------------------------------

    // GET all expenses
    server.Get("/api/expenses",
        [&](const httplib::Request&, httplib::Response& res)
        {
            std::string result = expenseApi.getAllExpenses();
            res.set_content(result, "application/json");
        }
    );

    // GET expense by ID
    server.Get("/api/expenses/:id",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            std::string expenseId = req.path_params.at("id");
            std::string result = expenseApi.getExpenseById(expenseId);
            res.set_content(result, "application/json");
        }
    );

    // POST create expense
    server.Post("/api/expenses",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);

                Expense expense;
                expense.title = body.value("title", std::string(""));
                expense.category = body.value("category", std::string(""));
                expense.amountPaise = body.value("amountPaise", 0LL);
                expense.date = body.value("date", std::string(""));
                expense.note = body.value("note", std::string(""));

                if (expense.title.empty() ||
                    expense.category.empty() ||
                    expense.amountPaise <= 0 ||
                    expense.date.empty())
                {
                    res.status = 400;
                    res.set_content(
                        R"({"success":false,"message":"Invalid expense data","data":null})",
                        "application/json"
                    );
                    return;
                }

                std::string result = expenseApi.createExpense(expense);
                res.set_content(result, "application/json");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid JSON request","data":null})",
                    "application/json"
                );
            }
        }
    );

    // PUT update expense
    server.Put("/api/expenses/:id",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);

                Expense updatedExpense;
                updatedExpense.title = body.value("title", std::string(""));
                updatedExpense.category = body.value("category", std::string(""));
                updatedExpense.amountPaise = body.value("amountPaise", 0LL);
                updatedExpense.date = body.value("date", std::string(""));
                updatedExpense.note = body.value("note", std::string(""));

                if (updatedExpense.title.empty() ||
                    updatedExpense.category.empty() ||
                    updatedExpense.amountPaise <= 0 ||
                    updatedExpense.date.empty())
                {
                    res.status = 400;
                    res.set_content(
                        R"({"success":false,"message":"Invalid expense data","data":null})",
                        "application/json"
                    );
                    return;
                }

                std::string id = req.path_params.at("id");
                std::string result = expenseApi.updateExpense(id, updatedExpense);
                res.set_content(result, "application/json");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid JSON request","data":null})",
                    "application/json"
                );
            }
        }
    );

    // DELETE expense
    server.Delete("/api/expenses/:id",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            std::string id = req.path_params.at("id");
            std::string result = expenseApi.deleteExpense(id);
            res.set_content(result, "application/json");
        }
    );

    // -------------------------------------------------------------
    // Analytics Endpoint
    // -------------------------------------------------------------
    server.Get("/api/analytics",
        [&](const httplib::Request&, httplib::Response& res)
        {
            std::string result = expenseApi.getAnalytics();
            res.set_content(result, "application/json");
        }
    );

    // -------------------------------------------------------------
    // Budgets Endpoints
    // -------------------------------------------------------------
    server.Get("/api/budgets",
        [&](const httplib::Request&, httplib::Response& res)
        {
            std::string result = expenseApi.getBudgets();
            res.set_content(result, "application/json");
        }
    );

    server.Post("/api/budgets",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                std::string category = body.value("category", std::string(""));
                long long limitPaise = body.value("limitPaise", 0LL);
                int period = body.value("period", 2); // default: 2 (MONTHLY)

                if (category.empty() || limitPaise <= 0)
                {
                    res.status = 400;
                    res.set_content(
                        R"({"success":false,"message":"Category and positive limitPaise are required","data":null})",
                        "application/json"
                    );
                    return;
                }

                std::string result = expenseApi.addOrUpdateBudget(category, limitPaise, period);
                res.set_content(result, "application/json");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid JSON request","data":null})",
                    "application/json"
                );
            }
        }
    );

    server.Delete("/api/budgets/:category",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            std::string category = req.path_params.at("category");
            std::string result = expenseApi.deleteBudget(category);
            res.set_content(result, "application/json");
        }
    );

    // -------------------------------------------------------------
    // Groups & Expense Splitting Endpoints
    // -------------------------------------------------------------
    server.Get("/api/groups",
        [&](const httplib::Request&, httplib::Response& res)
        {
            std::string result = expenseApi.getAllGroups();
            res.set_content(result, "application/json");
        }
    );

    server.Get("/api/groups/:id",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            std::string groupId = req.path_params.at("id");
            std::string result = expenseApi.getGroupDetails(groupId);
            res.set_content(result, "application/json");
        }
    );

    server.Post("/api/groups",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                auto body = json::parse(req.body);
                std::string name = body.value("name", std::string(""));
                std::vector<std::string> members;
                if (body.contains("members") && body["members"].is_array())
                {
                    for (const auto& m : body["members"])
                    {
                        if (m.is_string()) members.push_back(m.get<std::string>());
                    }
                }

                if (name.empty())
                {
                    res.status = 400;
                    res.set_content(
                        R"({"success":false,"message":"Group name is required","data":null})",
                        "application/json"
                    );
                    return;
                }

                std::string result = expenseApi.createGroup(name, members);
                res.set_content(result, "application/json");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid JSON request","data":null})",
                    "application/json"
                );
            }
        }
    );

    server.Post("/api/groups/:id/expenses",
        [&](const httplib::Request& req, httplib::Response& res)
        {
            try
            {
                std::string groupId = req.path_params.at("id");
                auto body = json::parse(req.body);

                std::string title = body.value("title", std::string(""));
                std::string category = body.value("category", std::string("Other"));
                std::string paidBy = body.value("paidByUserId", std::string("USR001"));
                long long amountPaise = body.value("amountPaise", 0LL);
                std::string date = body.value("date", std::string(""));

                if (title.empty() || amountPaise <= 0 || date.empty())
                {
                    res.status = 400;
                    res.set_content(
                        R"({"success":false,"message":"Invalid group expense data","data":null})",
                        "application/json"
                    );
                    return;
                }

                std::string result = expenseApi.addGroupExpense(groupId, title, category, paidBy, amountPaise, date);
                res.set_content(result, "application/json");
            }
            catch (const std::exception&)
            {
                res.status = 400;
                res.set_content(
                    R"({"success":false,"message":"Invalid JSON request","data":null})",
                    "application/json"
                );
            }
        }
    );

    // -------------------------------------------------------------
    // Users Endpoint
    // -------------------------------------------------------------
    server.Get("/api/users",
        [&](const httplib::Request&, httplib::Response& res)
        {
            std::string result = expenseApi.getAllUsers();
            res.set_content(result, "application/json");
        }
    );

    // -------------------------------------------------------------
    // Static Frontend File Serving
    // -------------------------------------------------------------
    server.set_mount_point("/", "./frontend");

    // -------------------------------------------------------------
    // Start Server
    // -------------------------------------------------------------
    std::cout << "====================================================\n";
    std::cout << " PERSONAL EXPENSE ANALYZER & SMART SPLITTER SERVER  \n";
    std::cout << "====================================================\n";
    std::cout << " API Base URL:       http://localhost:8080/api\n";
    std::cout << " Frontend Website:   http://localhost:8080\n";
    std::cout << " Press Ctrl + C to stop the server.\n";

    if (!server.listen("0.0.0.0", 8080))
    {
        std::cerr << "Failed to start server on port 8080!\n";
        return 1;
    }

    return 0;
}
