#include "storage.h"

#include <sqlite3.h>

namespace {

bool execOrFail(sqlite3* db, const char* sql, std::string& err) {
    char* msg = nullptr;
    if (sqlite3_exec(db, sql, nullptr, nullptr, &msg) != SQLITE_OK) {
        err = msg ? msg : "sqlite error";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

}  // namespace

Storage::Storage(const std::string& dbPath) {
    if (sqlite3_open(dbPath.c_str(), &db_) != SQLITE_OK) {
        lastError_ = db_ ? sqlite3_errmsg(db_) : "could not open database";
        if (db_) {
            sqlite3_close(db_);
            db_ = nullptr;
        }
        return;
    }
    if (!execOrFail(db_,
                    "CREATE TABLE IF NOT EXISTS transactions ("
                    "  id INTEGER PRIMARY KEY AUTOINCREMENT,"
                    "  date TEXT NOT NULL, category TEXT NOT NULL,"
                    "  description TEXT NOT NULL, amount REAL NOT NULL);"
                    "CREATE TABLE IF NOT EXISTS settings ("
                    "  key TEXT PRIMARY KEY, value REAL NOT NULL);",
                    lastError_)) {
        sqlite3_close(db_);
        db_ = nullptr;
    }
}

Storage::~Storage() {
    if (db_) sqlite3_close(db_);
}

std::string Storage::lastError() const { return lastError_; }

bool Storage::saveAll(const std::vector<Transaction>& transactions, double monthlyBudget) {
    if (!db_) return false;
    char* msg = nullptr;
    if (sqlite3_exec(db_, "BEGIN TRANSACTION;", nullptr, nullptr, &msg) != SQLITE_OK) {
        lastError_ = msg ? msg : "begin failed";
        sqlite3_free(msg);
        return false;
    }
    auto fail = [&](const std::string& e) {
        lastError_ = e;
        sqlite3_exec(db_, "ROLLBACK;", nullptr, nullptr, nullptr);
        return false;
    };
    if (!execOrFail(db_, "DELETE FROM transactions;", lastError_)) return fail(lastError_);

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_,
                           "INSERT INTO transactions (date, category, description, amount) "
                           "VALUES (?, ?, ?, ?);",
                           -1, &stmt, nullptr) != SQLITE_OK) {
        return fail(sqlite3_errmsg(db_));
    }
    for (const auto& t : transactions) {
        sqlite3_bind_text(stmt, 1, t.date.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 2, t.category.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_text(stmt, 3, t.description.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_bind_double(stmt, 4, t.amount);
        if (sqlite3_step(stmt) != SQLITE_DONE) {
            std::string e = sqlite3_errmsg(db_);
            sqlite3_finalize(stmt);
            return fail(e);
        }
        sqlite3_reset(stmt);
        sqlite3_clear_bindings(stmt);
    }
    sqlite3_finalize(stmt);

    if (sqlite3_prepare_v2(db_,
                           "INSERT INTO settings (key, value) VALUES ('monthly_budget', ?) "
                           "ON CONFLICT(key) DO UPDATE SET value = excluded.value;",
                           -1, &stmt, nullptr) != SQLITE_OK) {
        return fail(sqlite3_errmsg(db_));
    }
    sqlite3_bind_double(stmt, 1, monthlyBudget);
    if (sqlite3_step(stmt) != SQLITE_DONE) {
        std::string e = sqlite3_errmsg(db_);
        sqlite3_finalize(stmt);
        return fail(e);
    }
    sqlite3_finalize(stmt);

    if (sqlite3_exec(db_, "COMMIT;", nullptr, nullptr, &msg) != SQLITE_OK) {
        lastError_ = msg ? msg : "commit failed";
        sqlite3_free(msg);
        return false;
    }
    return true;
}

bool Storage::loadAll(std::vector<Transaction>& transactions, double& monthlyBudget) {
    if (!db_) return false;
    transactions.clear();

    sqlite3_stmt* stmt = nullptr;
    if (sqlite3_prepare_v2(db_,
                           "SELECT date, category, description, amount FROM transactions "
                           "ORDER BY date, id;",
                           -1, &stmt, nullptr) != SQLITE_OK) {
        lastError_ = sqlite3_errmsg(db_);
        return false;
    }
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        Transaction t;
        auto text = [&](int col) {
            const unsigned char* s = sqlite3_column_text(stmt, col);
            return s ? reinterpret_cast<const char*>(s) : "";
        };
        t.date = text(0);
        t.category = text(1);
        t.description = text(2);
        t.amount = sqlite3_column_double(stmt, 3);
        transactions.push_back(t);
    }
    sqlite3_finalize(stmt);

    monthlyBudget = 0.0;
    if (sqlite3_prepare_v2(db_, "SELECT value FROM settings WHERE key = 'monthly_budget';",
                           -1, &stmt, nullptr) == SQLITE_OK) {
        if (sqlite3_step(stmt) == SQLITE_ROW) monthlyBudget = sqlite3_column_double(stmt, 0);
        sqlite3_finalize(stmt);
    }
    return true;
}
