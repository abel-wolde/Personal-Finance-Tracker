#pragma once

#include <string>
#include <vector>

#include "transaction.h"

// In-memory tracker. Same behaviour as the original single-file version,
// plus sorting, category search and per-month totals. Persistence is kept
// separate in Storage; call save()/load() to round-trip through SQLite.
class FinanceTracker {
public:
    explicit FinanceTracker(double budget = 0.0) : monthlyBudget_(budget) {}

    void setBudget(double budget) { monthlyBudget_ = budget; }
    double monthlyBudget() const { return monthlyBudget_; }

    void addTransaction(const std::string& date, const std::string& category,
                        const std::string& description, double amount);
    const std::vector<Transaction>& transactions() const { return transactions_; }

    double totalSpending() const;
    double remainingBudget() const { return monthlyBudget_ - totalSpending(); }

    // All transactions in a category (exact, case-sensitive match).
    std::vector<Transaction> searchByCategory(const std::string& category) const;
    // Total spent in a month, e.g. "2026-09" (matches the date prefix).
    double totalForMonth(const std::string& yearMonth) const;
    // Total spent in one category.
    double totalForCategory(const std::string& category) const;

    // Sort the stored list in place.
    void sortByDate();
    void sortByAmount();     // ascending
    void sortByCategory();   // then date

    // Persistence. dbPath defaults to finance.db in the working directory.
    bool save(const std::string& dbPath = "finance.db");
    bool load(const std::string& dbPath = "finance.db");
    std::string lastError() const { return lastError_; }

    void displayTransactions() const;
    void displaySummary() const;

private:
    std::vector<Transaction> transactions_;
    double monthlyBudget_;
    std::string lastError_;
};
