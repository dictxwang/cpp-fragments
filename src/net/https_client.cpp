#include <iostream>
#include <curl/curl.h>
#include "config/config.h"

// Callback function to write received data into a string
size_t WriteCallback(void* contents, size_t size, size_t nmemb, std::string* userData) {
    size_t totalSize = size * nmemb;
    userData->append(static_cast<char*>(contents), totalSize);
    return totalSize;
}

int main(int argc, char** argv) {

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " config_file" << std::endl;
        return 0;
    }

    Config config;
    if (!config.LoadConfig(argv[1])) {
        std::cerr << "Load config error : " << argv[1] << std::endl;
        return 1;
    }

    CURL* curl;
    CURLcode res;

    // The URL of the HTTPS backend
    const std::string url = "https://api.binance.com/api/v3/exchangeInfo";
    const std::string host = "api.binance.com";
    const std::string port = "443";
    const std::string localIP = config.binance_reset_local_ip; // Specify the local IP address or network interface here
    const std::string remoteIP = config.binance_reset_remote_ip; // Specify the remote IP address or network interface here

    // The response data will be stored here
    std::string responseData;

    // Initialize libcurl
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();

    if (curl) {
        // Set the URL for the request
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());

        // Bind the request to a specific local IP or network interface
        if (!localIP.empty()) {
            curl_easy_setopt(curl, CURLOPT_INTERFACE, localIP.c_str());
        }

        struct curl_slist* resolve_list = nullptr;
        if (!remoteIP.empty()) {
            // curl_easy_setopt(curl, CURLOPT_CONNECT_TO, (host + ":" + port + "::" + remoteIP).c_str());
            std::string resolve_entry = host + ":" + port + ":" + remoteIP;
            resolve_list = curl_slist_append(resolve_list, resolve_entry.c_str());
            curl_easy_setopt(curl, CURLOPT_RESOLVE, resolve_list);
        }

        // Set the callback function to handle response data
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);

        // Pass the string to the callback function
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &responseData);

        // Enable SSL/TLS verification
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 1L); // Verify the server's certificate
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 2L); // Verify the hostname


        // Perform the request
        res = curl_easy_perform(curl);

        if (res != CURLE_OK) {
            std::cerr << "curl_easy_perform() failed: " << curl_easy_strerror(res) << std::endl;
        } else {
            std::cout << "Response Data: " << responseData << std::endl;
        }

        // Cleanup
        if (resolve_list != nullptr) {
            curl_slist_free_all(resolve_list);
        }
        curl_easy_cleanup(curl);
    }

    // Cleanup libcurl global resources
    curl_global_cleanup();

    return 0;
}