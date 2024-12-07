#pragma once
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <utility>

std::string trim(const std::string& str);
std::string findValue(const std::string& json, const std::string& key);
std::vector<std::pair<std::string, std::string>> extractMediaUrlsWithIds(const std::string& json);
std::vector<std::pair<std::string, std::string>> parseIdsAndSourceText(const std::string& json);

std::vector<std::string> processCitationData(const std::string& documentId);
std::vector<std::pair<std::string, std::vector<std::string>>> getDocumentData();