#include "finance_tracker.h"

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <numeric>

#include "storage.h"

void FinanceTracker::addTransaction(const std::string& date, const std::string& category,
                                    const std::string& description, double amount) {
    transactions_.push_back({date, category, description, amount});
}

double FinanceTracker::totalSpending() const {
    double total = 0.0;
    for (const auto& t : transactions_) total += t.amount;
    return total;
}

std::vector<Transaction> FinanceTracker::searchByCategory(const std::string& category) const {
    std::vector<Transaction> out;
    for (const auto& t : transactions_)
        if (t.category == category) out.push_back(t);
    return out;
}

double FinanceTracker::totalForMonth(const std::string& yearMonth) const {
    double total = 0.0;
    for (const auto& t : transactions_)
        if (t.date.rfind(yearMonth, 0) == 0) total += t.amount;
    return total;
}

double FinanceTracker::totalForCategory(const std::string& category) const {
    double total = 0.0;
    for (const auto& t : transactions_)
        if (t.category == category) total += t.amount;
    return total;
}

void FinanceTracker::sortByDate() {
    std::sort(transactions_.begin(), transactions_.end(),
              [](const Transaction& a, const Transaction& b) { return a.date < b.date; });
}

void FinanceTracker::sortByAmount() {
    std::sort(transactions_.begin(), transactions_.end(),
              [](const Transaction& a, const Transaction& b) { return a.amount < b.amount; });
}

void FinanceTracker::sortByCategory() {
    std::sort(transactions_.begin(), transactions_.end(), [](const Transaction& a, const Transaction& b) {
        if (a.category != b.category) return a.category < b.category;
        return a.date < b.date;
    });
}

bool FinanceTracker::save(const std::string& dbPath) {
    Storage store(dbPath);
    if (!store.ok()) {
        lastError_ = store.lastError();
        return false;
    }
    if (!store.saveAll(transactions_, monthlyBudget_)) {
        lastError_ = store.lastError();
        return false;
    }
    return true;
}

bool FinanceTracker::load(const std::string& dbPath) {
    Storage store(dbPath);
    if (!store.ok()) {
        lastError_ = store.lastError();
        return false;
    }
    if (!store.loadAll(transactions_, monthlyBudget_)) {
        lastError_ = store.lastError();
        return false;
    }
    return true;
}

void FinanceTracker::displayTransactions() const {
    if (transactions_.empty()) {
        std::cout << "No transactions recorded yet.\n";
        return;
    }
    std::cout << "\n--- All Transactions ---\n";
    std::cout << std::left << std::setw(12) << "Date" << std::setw(18) << "Category"
              << std::setw(25) << "Description" << std::right << std::setw(10) << "Amount ($)"
              << "\n";
    std::cout << std::string(65, '-') << "\n";
    for (const auto& t : transactions_) {
        std::cout << std::left << std::setw(12) << t.date << std::setw(18) << t.category
                  << std::setw(25) << t.description << std::right << std::setw(10) << std::fixed
                  << std::setprecision(2) << t.amount << "\n";
    }
}

void FinanceTracker::displaySummary() const {
    double total = totalSpending();
    std::cout << "\n=== Financial Summary ===\n";
    std::cout << "Monthly Budget : $" << std::fixed << std::setprecision(2) << monthlyBudget_
              << "\n";
    std::cout << "Total Spent    : $" << total << "\n";
    if (monthlyBudget_ > 0) {
        double remaining = monthlyBudget_ - total;
        if (remaining >= 0) {
            std::cout << "Remaining      : $" << remaining << " (Within Budget)\n";
        } else {
            std::cout << "Remaining      : -$" << -remaining << " (OVER BUDGET!)\n";
        }
    }
}
