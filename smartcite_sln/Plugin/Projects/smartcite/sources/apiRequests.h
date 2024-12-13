#pragma once
#include <curl/curl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <vector>
#include <string>
#include <utility>

struct docInfo
{
	std::string id;
	std::string mediaUrl;
	std::string filename;
};

struct docProcessedData
{
	docInfo docInfo;
	std::vector<std::string> citations; 
};

std::string trim(const std::string& str);
std::string findValue(const std::string& json, const std::string& key);
std::vector<docInfo> extractMediaUrlsWithIds(const std::string& json);
std::vector<std::pair<std::string, std::string>> parseIdsAndSourceText(const std::string& json);

std::vector<std::string> processCitationData(const std::string& documentId);
std::vector<docProcessedData> getDocumentData();
int downloadUrl(std::string url, std::string filename);
void openFileUrl(std::string filename);