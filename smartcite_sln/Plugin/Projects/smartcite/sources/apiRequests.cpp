#include "apiRequests.h"

#include "PIHeaders.h"
#include <iostream>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <string>

#pragma comment(lib, "Ws2_32.lib")

size_t write_callback_test(void* contents, size_t size, size_t nmemb, void* userp) {
    size_t total_size = size * nmemb;
    std::string* response = static_cast<std::string*>(userp);
    response->append(static_cast<char*>(contents), total_size);
    return total_size;
}

void sendTestRequest()
{
    CURL* curl;
    CURLcode res;

    std::string url = "https://google.com";
    std::string response;

    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback_test);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

        // Perform the HTTP GET request
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << "\n";
        }
        else {
            std::cout << "Server response: " << response << "\n";
        }

        curl_easy_cleanup(curl);
    }
    else {
        std::cerr << "Failed to initialize libcurl.\n";
    }

    curl_global_cleanup();
    return ;
}

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
std::vector<docInfo> extractMediaUrlsWithIds(const std::string& json) {
    std::vector<docInfo> mediaUrlsWithIds;

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
        std::string filename = findValue(item, "title");

        if (!citationsCountStr.empty() && std::stoi(citationsCountStr) > 0 && !mediaUrl.empty() && !id.empty()) {
            mediaUrlsWithIds.emplace_back(docInfo({ id, mediaUrl, filename }));
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

static int debug_callback(CURL* handle, curl_infotype type, char* data, size_t size, void* userptr) {
    if (type == CURLINFO_TEXT)
        fprintf(stderr, "== Info: %s", data);
    return 0;
}

std::string performCurlRequest_simple(const std::string& url) {
    CURL* curl = curl_easy_init();
    if (!curl) {
        fprintf(stderr, "Failed to initialize CURL\n");
        return "";
    }

    std::string response;
    curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
    curl_easy_setopt(curl, CURLOPT_SSL_OPTIONS, CURLSSLOPT_NO_REVOKE);

    curl_easy_setopt(curl, CURLOPT_SSLCERT, NULL);
    curl_easy_setopt(curl, CURLOPT_SSLKEY, NULL);

    // SSL/TLS options
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 1L); // Verify hostname
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Verify CA

    curl_easy_setopt(curl, CURLOPT_DEBUGFUNCTION, debug_callback);
    curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);

    CURLcode res = curl_easy_perform(curl);
    if (res != CURLE_OK) {
        fprintf(stderr, "CURL request failed: %s\n", curl_easy_strerror(res));
    }

    curl_easy_cleanup(curl);
    return response;
}

// Function to perform a cURL request with an API key and return the response
std::string performCurlRequest(const std::string& url) {
    //sendTestRequest();
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
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);

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
std::vector<docProcessedData> getDocumentData() {
    std::string documentsUrl = "http://google.com";
    std::string documentsResponse = performCurlRequest(documentsUrl);
    std::vector<docProcessedData> output;
    if (documentsResponse.empty()) {
        fprintf(stderr, "Failed to retrieve documents\n");
        return output;
    }

    // Parse documents and extract media URLs with IDs
    std::vector<docInfo> mediaUrlsWithIds = extractMediaUrlsWithIds(documentsResponse);
    // Process each document's citations
    for (const auto& pair : mediaUrlsWithIds) {
        const std::string& documentId = pair.id; // The second element is the document ID
        std::vector<std::string> sourceText = processCitationData(documentId);
        output.push_back(docProcessedData({pair, sourceText}));
    }

    return output;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <curl/curl.h>
#include <windows.h>
#include <shlobj.h> // For SHGetFolderPath

// Structure for file handling during download
struct FileHandle {
    FILE* file;
    const char* filename;
};

// Callback function to write data to a file
size_t write_data(void* ptr, size_t size, size_t nmemb, void* stream) {
    struct FileHandle* out = (struct FileHandle*)stream;
    if (!out->file) {
        return -1; // Failure
    }
    return fwrite(ptr, size, nmemb, out->file);
}

// Function to get the AppData directory path
char* get_appdata_path() {
    static char appdata_path[MAX_PATH];
    if (SHGetFolderPath(NULL, CSIDL_LOCAL_APPDATA, NULL, 0, appdata_path) != S_OK) {
        fprintf(stderr, "Error: Unable to retrieve AppData path.\n");
        return NULL;
    }
    return appdata_path;
}

// Function to download a file using libcurl
int download_file(const char* url, const char* destination_path) {
    CURL* curl;
    CURLcode res;
    struct FileHandle file_handle;

    // Open file for writing
    file_handle.file = fopen(destination_path, "wb");
    if (!file_handle.file) {
        fprintf(stderr, "Error: Unable to open file %s for writing.\n", destination_path);
        return -1;
    }
    file_handle.filename = destination_path;

    // Initialize curl
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file_handle);

        // Perform the file download
        res = curl_easy_perform(curl);
        if (res != CURLE_OK) {
            fprintf(stderr, "Error: curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
            fclose(file_handle.file);
            curl_easy_cleanup(curl);
            return -1;
        }

        // Clean up
        curl_easy_cleanup(curl);
    }
    else {
        fprintf(stderr, "Error: Unable to initialize curl.\n");
        fclose(file_handle.file);
        return -1;
    }

    fclose(file_handle.file);
    return 0; // Success
}

int downloadUrl(std::string url, std::string filename) {
    char* appdata_path = get_appdata_path();
    if (!appdata_path) {
        return -1;
    }

    // Construct the target directory path
    char target_dir[MAX_PATH];
    snprintf(target_dir, sizeof(target_dir), "%s\\smartcite", appdata_path);

    // Create the directory if it doesn't exist
    if (CreateDirectory(target_dir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        // Construct the full destination path
        char destination_path[MAX_PATH];
        snprintf(destination_path, sizeof(destination_path), "%s\\%s", target_dir, filename.c_str());

        // Download the file
        if (download_file(url.c_str(), destination_path) == 0) {
            printf("File successfully downloaded to: %s\n", destination_path);
        }
        else {
            fprintf(stderr, "Error: Failed to download the file.\n");
        }
    }
    else {
        fprintf(stderr, "Error: Unable to create directory %s.\n", target_dir);
    }

    return 0;
}

void openPDFDocument(std::string filePath) {
    ASPathName path = ASFileSysCreatePathName(NULL, ASAtomFromString("Cstring"), filePath.c_str(), NULL);
    ASFile file;
    if (path != NULL) {
        ASFileSysOpenFile(NULL, path, ASFILE_READ, &file);
        AVDoc avDoc = AVDocOpenFromASFileWithParams(file, NULL, NULL);
        if (avDoc) {
            printf("Document opened successfully.\n");
        }
        else {
            printf("Failed to open document.\n");
        }
        ASFileSysReleasePath(NULL, path);
    }
    else {
        printf("Invalid file path.\n");
    }
}

void openFileUrl(std::string filename) {
    char* appdata_path = get_appdata_path();
    if (!appdata_path) {
        return;
    }

    // Construct the target directory path
    char target_dir[MAX_PATH];
    snprintf(target_dir, sizeof(target_dir), "%s\\smartcite", appdata_path);

    // Create the directory if it doesn't exist
    if (CreateDirectory(target_dir, NULL) || GetLastError() == ERROR_ALREADY_EXISTS) {
        // Construct the full destination path
        char destination_path[MAX_PATH];
        snprintf(destination_path, sizeof(destination_path), "%s\\%s", target_dir, filename.c_str());

        // Download the file
        openPDFDocument(std::string(destination_path));
    }
    else {
        fprintf(stderr, "Error: Unable to create directory %s.\n", target_dir);
    }

    return;
}