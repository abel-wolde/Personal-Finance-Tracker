// Unit tests for the Finance Tracker upgrade.
// No external framework: a tiny CHECK macro, one binary, non-zero exit on
// any failure. Run with `make test`.
// The sample transactions below are test data only (same shape as the
// original demo entries), not seeded into the app.

#include <cstdio>
#include <string>

#include "finance_tracker.h"

static int g_failures = 0;
static int g_checks = 0;

#define CHECK(cond)                                                                            \
    do {                                                                                       \
        ++g_checks;                                                                            \
        if (!(cond)) {                                                                         \
            ++g_failures;                                                                      \
            std::printf("FAIL %s:%d: %s\n", __FILE__, __LINE__, #cond);                        \
        }                                                                                      \
    } while (0)

static bool approx(double a, double b) { return a > b - 0.005 && a < b + 0.005; }

int main() {
    // Totals, budget, over-budget
    {
        FinanceTracker t(500.0);
        t.addTransaction("2026-09-01", "Groceries", "Campus Store Supplies", 45.50);
        t.addTransaction("2026-09-05", "Books", "C++ Programming Guide", 65.00);
        t.addTransaction("2026-09-10", "Dining", "Sodexo Meal Pass", 25.00);
        CHECK(approx(t.totalSpending(), 135.50));
        CHECK(approx(t.remainingBudget(), 364.50));
        CHECK(approx(t.totalForCategory("Books"), 65.00));
        CHECK(approx(t.totalForCategory("Missing"), 0.0));
        CHECK(approx(t.totalForMonth("2026-09"), 135.50));
        CHECK(approx(t.totalForMonth("2026-10"), 0.0));
        CHECK(t.searchByCategory("Groceries").size() == 1);
        CHECK(t.searchByCategory("Groceries")[0].description == "Campus Store Supplies");
        CHECK(t.searchByCategory("Missing").empty());

        FinanceTracker over(100.0);
        over.addTransaction("2026-09-01", "Books", "Textbook", 150.00);
        CHECK(approx(over.remainingBudget(), -50.00));
    }

    // Sorting
    {
        FinanceTracker t;
        t.addTransaction("2026-09-10", "Dining", "Lunch", 25.00);
        t.addTransaction("2026-09-01", "Groceries", "Store", 45.50);
        t.addTransaction("2026-09-05", "Books", "Guide", 65.00);
        t.sortByDate();
        CHECK(t.transactions()[0].date == "2026-09-01");
        CHECK(t.transactions()[2].date == "2026-09-10");
        t.sortByAmount();
        CHECK(approx(t.transactions()[0].amount, 25.00));
        CHECK(approx(t.transactions()[2].amount, 65.00));
        t.sortByCategory();
        CHECK(t.transactions()[0].category == "Books");
        CHECK(t.transactions()[1].category == "Dining");
        CHECK(t.transactions()[2].category == "Groceries");
    }

    // SQLite persistence round-trip (budget + transactions survive reload)
    {
        const std::string db = "test_finance.db";
        std::remove(db.c_str());
        {
            FinanceTracker t(500.0);
            t.addTransaction("2026-09-01", "Groceries", "Campus Store Supplies", 45.50);
            t.addTransaction("2026-09-05", "Books", "C++ Programming Guide", 65.00);
            CHECK(t.save(db));
        }
        {
            FinanceTracker t;
            CHECK(t.load(db));
            CHECK(approx(t.monthlyBudget(), 500.00));
            CHECK(t.transactions().size() == 2);
            CHECK(approx(t.totalSpending(), 110.50));
            CHECK(t.transactions()[0].category == "Groceries");
        }
        // Overwriting with an empty tracker clears the stored data.
        {
            FinanceTracker t;
            CHECK(t.save(db));
        }
        {
            FinanceTracker t;
            CHECK(t.load(db));
            CHECK(t.transactions().empty());
            CHECK(approx(t.monthlyBudget(), 0.0));
        }
        std::remove(db.c_str());
    }

    std::printf("%d checks, %d failures\n", g_checks, g_failures);
    return g_failures == 0 ? 0 : 1;
}
