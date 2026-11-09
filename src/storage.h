#pragma once

#include <string>
#include <vector>

#include "transaction.h"

// SQLite persistence for the Finance Tracker.
// Schema (created on first open):
//   transactions(id INTEGER PRIMARY KEY, date TEXT, category TEXT,
//                description TEXT, amount REAL)
//   settings(key TEXT PRIMARY KEY, value REAL)   -- key "monthly_budget"
class Storage {
public:
    explicit Storage(const std::string& dbPath);
    ~Storage();

    Storage(const Storage&) = delete;
    Storage& operator=(const Storage&) = delete;

    bool ok() const { return db_ != nullptr; }
    std::string lastError() const;

    bool saveAll(const std::vector<Transaction>& transactions, double monthlyBudget);
    bool loadAll(std::vector<Transaction>& transactions, double& monthlyBudget);

private:
    struct sqlite3* db_ = nullptr;
    std::string lastError_;
};
