// Personal Finance Tracker — interactive CLI.
// Data persists in finance.db (SQLite) in the working directory, so
// transactions survive between runs. The original version kept everything
// in memory and lost it on exit; the demo transactions from the original
// main() are NOT seeded here — the database starts empty.

#include <iostream>
#include <limits>
#include <string>

#include "finance_tracker.h"

namespace {

std::string readLine(const std::string& prompt) {
    std::cout << prompt;
    std::string line;
    std::getline(std::cin, line);
    return line;
}

double readDouble(const std::string& prompt) {
    while (true) {
        std::cout << prompt;
        double value;
        if (std::cin >> value) {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            return value;
        }
        std::cin.clear();
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
        std::cout << "Please enter a number.\n";
    }
}

}  // namespace

int main() {
    FinanceTracker tracker;
    if (!tracker.load()) {
        std::cerr << "Warning: could not load finance.db (" << tracker.lastError()
                  << "). Starting empty.\n";
    }

    while (true) {
        std::cout << "\n=== Personal Finance Tracker ===\n"
                  << "1. Add transaction\n"
                  << "2. List transactions\n"
                  << "3. Show summary\n"
                  << "4. Search by category\n"
                  << "5. Monthly total\n"
                  << "6. Sort transactions\n"
                  << "7. Set monthly budget\n"
                  << "8. Save and exit\n"
                  << "Choice: ";
        int choice = 0;
        if (!(std::cin >> choice)) break;
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');

        if (choice == 1) {
            std::string date = readLine("Date (YYYY-MM-DD): ");
            std::string category = readLine("Category: ");
            std::string description = readLine("Description: ");
            double amount = readDouble("Amount ($): ");
            tracker.addTransaction(date, category, description, amount);
            std::cout << "Transaction added successfully!\n";
        } else if (choice == 2) {
            tracker.displayTransactions();
        } else if (choice == 3) {
            tracker.displaySummary();
        } else if (choice == 4) {
            std::string category = readLine("Category to search: ");
            auto found = tracker.searchByCategory(category);
            std::cout << found.size() << " transaction(s) in '" << category << "' totalling $"
                      << tracker.totalForCategory(category) << "\n";
            for (const auto& t : found)
                std::cout << "  " << t.date << "  " << t.description << "  $" << t.amount << "\n";
        } else if (choice == 5) {
            std::string month = readLine("Month (YYYY-MM): ");
            std::cout << "Total spent in " << month << ": $" << tracker.totalForMonth(month)
                      << "\n";
        } else if (choice == 6) {
            std::cout << "Sort by: 1=date  2=amount  3=category\nChoice: ";
            int by = 0;
            std::cin >> by;
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            if (by == 1) tracker.sortByDate();
            if (by == 2) tracker.sortByAmount();
            if (by == 3) tracker.sortByCategory();
            tracker.displayTransactions();
        } else if (choice == 7) {
            tracker.setBudget(readDouble("New monthly budget ($): "));
            std::cout << "Monthly budget set.\n";
        } else if (choice == 8) {
            if (!tracker.save()) {
                std::cerr << "Error saving: " << tracker.lastError() << "\n";
                return 1;
            }
            std::cout << "Saved to finance.db. Goodbye!\n";
            return 0;
        } else {
            std::cout << "Unknown choice.\n";
        }
    }
    return 0;
}
