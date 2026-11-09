# Personal Finance Tracker — Upgraded

C++17 console app by Abel Wolde. Tracks expenses against a monthly budget:
expense logging, monthly budgets, spending summaries, sorting, and category
search. Upgraded from the original single-file `main.cpp`
([abel-wolde/Personal-Finance-Tracker](https://github.com/abel-wolde/Personal-Finance-Tracker)).

## What changed in this upgrade

- **SQLite persistence** — transactions and the monthly budget are stored in
  `finance.db`, so data survives between runs. The original kept everything
  in memory and lost it on exit.
- **Split into modules** — `src/transaction.h` (data model),
  `src/finance_tracker.h/.cpp` (tracker logic), `src/storage.h/.cpp`
  (SQLite layer), `src/main.cpp` (interactive CLI).
- **New features** — sort by date / amount / category, search by category,
  monthly totals (`totalForMonth("2026-09")`), per-category totals.
- **Unit tests** — `tests/test_finance_tracker.cpp`, 27 checks, no external
  framework.
- **Interactive CLI** — the original `main()` only ran a fixed demo with
  three hard-coded transactions. The demo data is NOT seeded into the app;
  it lives on as test data only. The app now starts with an empty database.

## Build & test

Requires g++ (C++17) and SQLite dev headers (`libsqlite3-dev` on Ubuntu).

```bash
make           # builds ./finance_tracker
make test      # builds and runs the tests (27 checks, 0 failures)
./finance_tracker   # interactive menu; data saved to finance.db
make clean
```

## Schema

```sql
CREATE TABLE transactions (
  id INTEGER PRIMARY KEY AUTOINCREMENT,
  date TEXT, category TEXT, description TEXT, amount REAL);
CREATE TABLE settings (key TEXT PRIMARY KEY, value REAL); -- monthly_budget
```

## What's missing before pushing to GitHub

- Push these files into the existing `Personal-Finance-Tracker` repo
  (replace `main.cpp` with this layout, or add it alongside and update the
  repo README). The original `main.cpp` is preserved in git history.
- Not yet built: CSV export/import, deleting or editing a transaction,
  multi-month budget tracking.
