#pragma once

#include <string>

// One expense entry. Same fields as the original single-file version:
// date (YYYY-MM-DD), category, description, amount in dollars.
struct Transaction {
    std::string date;
    std::string category;
    std::string description;
    double amount = 0.0;
};
