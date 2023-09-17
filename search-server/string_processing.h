#pragma once

#include <set>
#include <string>
#include <vector>

std::vector<std::string> SplitIntoWords(const std::string& text);

std::set<std::string> MakeUniqueNonEmptyStrings(const std::vector<std::string>& strings);

std::set<std::string> MakeUniqueNonEmptyStrings(const std::set<std::string>& strings);