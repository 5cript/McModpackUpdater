#pragma once

#include <utility>
#include <string>
#include <tuple>

std::pair <int, std::string> download(std::string const& link);
int download(std::string const& link, std::string const& outFile);
std::tuple <int, int, std::string, std::string> followRedirect(std::string const& link);
