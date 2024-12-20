#include <microhttpd.h>
#include <iostream>
#include <cstring>

#define PORT 8080

// Callback to handle incoming HTTP GET requests
MHD_Result handle_request(void* cls,
    struct MHD_Connection* connection,
    const char* url,
    const char* method,
    const char* version,
    const char* upload_data,
    size_t* upload_data_size,
    void** req_cls) {
    // Handle only GET requests
    if (strcmp(method, "GET") != 0) {
        return MHD_NO; // Reject unsupported methods
    }

    std::cout << "Received GET request for URL: " << url << "\n";

    // Create a response
    const char* response_text = "Hello, this is the server responding!";
    struct MHD_Response* response = MHD_create_response_from_buffer(
        strlen(response_text), (void*)response_text, MHD_RESPMEM_PERSISTENT);

    if (!response) {
        return MHD_NO;
    }

    // Queue the response for the client
    MHD_Result ret = MHD_queue_response(connection, MHD_HTTP_OK, response);
    MHD_destroy_response(response);

    return ret;
}

int main() {
    struct MHD_Daemon* daemon;

    daemon = MHD_start_daemon(MHD_USE_INTERNAL_POLLING_THREAD, PORT, NULL, NULL,
        &handle_request, NULL, MHD_OPTION_END);
    if (!daemon) {
        std::cerr << "Failed to start HTTP server.\n";
        return 1;
    }

    std::cout << "HTTP server running on port " << PORT << ". Press Enter to stop.\n";
    getchar();

    MHD_stop_daemon(daemon);
    return 0;
}
