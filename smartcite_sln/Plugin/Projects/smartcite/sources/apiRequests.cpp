#include "apiRequests.h"

// Helper function to trim whitespace
std::string trim(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\n\r");
    size_t last = str.find_last_not_of(" \t\n\r");
    return (first == std::string::npos || last == std::string::npos) ? "" : str.substr(first, last - first + 1);
}

// Helper function to find a key and return its value as a string
std::string findValue(const std::string& json, const std::string& key) {
    size_t keyPos = json.find("\"" + key + "\"");
    if (keyPos == std::string::npos) return "";

    size_t colonPos = json.find(":", keyPos);
    if (colonPos == std::string::npos) return "";

    size_t start = json.find_first_not_of(" \t\n\r", colonPos + 1);
    if (start == std::string::npos) return "";

    size_t end;
    if (json[start] == '"') {  // String value
        start++;
        end = json.find('"', start);
    }
    else {  // Numeric or boolean value
        end = json.find_first_of(",}\n", start);
    }

    return trim(json.substr(start, end - start));
}

// Helper function to parse the items array and save mediaUrl with its id
std::vector<std::pair<std::string, std::string>> extractMediaUrlsWithIds(const std::string& json) {
    std::vector<std::pair<std::string, std::string>> mediaUrlsWithIds;

    size_t itemsPos = json.find("\"items\"");
    if (itemsPos == std::string::npos) return mediaUrlsWithIds;

    size_t arrayStart = json.find("[", itemsPos);
    size_t arrayEnd = json.find("]", arrayStart);
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) return mediaUrlsWithIds;

    std::string itemsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    size_t objStart = 0;
    while ((objStart = itemsArray.find("{", objStart)) != std::string::npos) {
        size_t objEnd = itemsArray.find("}", objStart);
        if (objEnd == std::string::npos) break;

        std::string item = itemsArray.substr(objStart + 1, objEnd - objStart - 1);

        // Extract id, citationsCount, and mediaUrl
        std::string id = findValue(item, "id");
        std::string citationsCountStr = findValue(item, "citationsCount");
        std::string mediaUrl = findValue(item, "mediaUrl");

        if (!citationsCountStr.empty() && std::stoi(citationsCountStr) > 0 && !mediaUrl.empty() && !id.empty()) {
            mediaUrlsWithIds.emplace_back(mediaUrl, id);
        }

        objStart = objEnd + 1;
    }

    return mediaUrlsWithIds;
}

// Function to parse the new request and extract id and sourceText
std::vector<std::pair<std::string, std::string>> parseIdsAndSourceText(const std::string& json) {
    std::vector<std::pair<std::string, std::string>> idSourceTextPairs;

    size_t itemsPos = json.find("\"items\"");
    if (itemsPos == std::string::npos) return idSourceTextPairs;

    size_t arrayStart = json.find("[", itemsPos);
    size_t arrayEnd = json.find("]", arrayStart);
    if (arrayStart == std::string::npos || arrayEnd == std::string::npos) return idSourceTextPairs;

    std::string itemsArray = json.substr(arrayStart + 1, arrayEnd - arrayStart - 1);

    size_t objStart = 0;
    while ((objStart = itemsArray.find("{", objStart)) != std::string::npos) {
        size_t objEnd = itemsArray.find("}", objStart);
        if (objEnd == std::string::npos) break;

        std::string item = itemsArray.substr(objStart + 1, objEnd - objStart - 1);

        // Extract id and sourceText
        std::string id = findValue(item, "id");
        std::string sourceText = findValue(item, "sourceText");

        if (!id.empty() && !sourceText.empty()) {
            idSourceTextPairs.emplace_back(id, sourceText);
        }

        objStart = objEnd + 1;
    }

    return idSourceTextPairs;
}

// Helper function to write cURL response to a buffer
size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t realsize = size * nmemb;
    char** response = (char**)userp;

    char* temp = (char*)realloc(*response, strlen(*response) + realsize + 1);
    if (!temp) {
        fprintf(stderr, "Failed to reallocate memory for response buffer\n");
        return 0;
    }

    *response = temp;
    strncat(*response, (char*)contents, realsize);
    return realsize;
}

// Function to perform a cURL request with an API key and return the response
std::string performCurlRequest(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return "";
    }

    char* responseBuffer = (char*)malloc(1);
    if (!responseBuffer) {
        fprintf(stderr, "Failed to allocate memory for response buffer\n");
        curl_easy_cleanup(curl);
        return "";
    }
    responseBuffer[0] = '\0';

    // Define the API key
    const std::string apiKey = "43166c69-c74a-4eb7-9176-86f79b9ff315";
    struct curl_slist* headers = nullptr;
    std::string apiKeyHeader = "x-api-key: " + apiKey;

    // Set the custom headers
    headers = curl_slist_append(headers, apiKeyHeader.c_str());

    // Set cURL options
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseBuffer);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL request failed: %s\n", curl_easy_strerror(res));
        free(responseBuffer);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        return "";
    }

    std::string response(responseBuffer);
    free(responseBuffer);
    curl_slist_free_all(headers);
    curl_easy_cleanup(curl);
    return response;
}

// Function to process citation data
std::vector<std::string> processCitationData(const std::string& documentId) {
    std::string citationsUrl = "https://api.smartcite.povio.dev/api/documents/" + documentId + "/citations";
    std::string citationsResponse = performCurlRequest(citationsUrl);
    std::vector<std::string> output;

    if (citationsResponse.empty()) {
        fprintf(stderr, "Failed to retrieve citations for document ID: %s\n", documentId.c_str());
        return output;
    }
    
    // Parse and process the citations response
    std::vector<std::pair<std::string, std::string>> idSourceTextPairs = parseIdsAndSourceText(citationsResponse);

    for (const auto& pair : idSourceTextPairs) {
        printf("Citation ID: %s, Source Text: %s\n", pair.first.c_str(), pair.second.c_str());
        output.push_back(pair.second);
    }

    return output;
}

// Function to get documents and process them
std::vector<std::pair<std::string, std::vector<std::string>>> getDocumentData() {
    std::string documentsUrl = "https://api.smartcite.povio.dev/api/documents/";
    std::string documentsResponse = performCurlRequest(documentsUrl);
    std::vector<std::pair<std::string, std::vector<std::string>>> output;
    if (documentsResponse.empty()) {
        fprintf(stderr, "Failed to retrieve documents\n");
        return output;
    }

    // Parse documents and extract media URLs with IDs
    std::vector<std::pair<std::string, std::string>> mediaUrlsWithIds = extractMediaUrlsWithIds(documentsResponse);
    // Process each document's citations
    for (const auto& pair : mediaUrlsWithIds) {
        const std::string& documentId = pair.second; // The second element is the document ID
        std::vector<std::string> sourceText = processCitationData(documentId);
        output.push_back(std::make_pair(pair.first, sourceText));
    }

    return output;
}