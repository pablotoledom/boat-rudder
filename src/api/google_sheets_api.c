#define _XOPEN_SOURCE 700  // Define POSIX.1-2008 compliance level

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../utils/curl/curl.h"
#include "google_sheets_api.h"
#include "../include/config_loader.h"

// Struct to hold response data from Google Sheets API
struct memory {
    char *response;
    size_t size;
};

// Callback function for curl to write received data
static size_t write_callback(void *data, size_t size, size_t nmemb, void *userp) {
    size_t real_size = size * nmemb;
    struct memory *mem = (struct memory *)userp;

    char *ptr = realloc(mem->response, mem->size + real_size + 1);
    if (ptr == NULL) {
        fprintf(stderr, "Not enough memory to allocate\n");
        return 0;
    }

    mem->response = ptr;
    memcpy(&(mem->response[mem->size]), data, real_size);
    mem->size += real_size;
    mem->response[mem->size] = '\0';

    return real_size;
}

// Function to fetch data from Google Sheets
char* fetch_google_sheets_data(const char* range) {
    CURL *curl;
    CURLcode res;
    struct memory chunk = { .response = NULL, .size = 0 };

    // Construct the URL for the Google Sheets API
    char url[1024];
    snprintf(url, sizeof(url),
             "https://sheets.googleapis.com/v4/spreadsheets/%s/values/%s?key=%s",
             spreadsheet_id, range, api_key);

    // Initialize curl
    curl = curl_easy_init();
    if (curl) {
        // Set the URL
        curl_easy_setopt(curl, CURLOPT_URL, url);

        // Set callback function to handle the response
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_callback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, (void *)&chunk);

        // Perform the request
        res = curl_easy_perform(curl);

        // Check for errors
        if (res != CURLE_OK) {
            fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(res));
        }

        // Cleanup curl
        curl_easy_cleanup(curl);
    }

    // Return the fetched data (JSON string)
    return chunk.response;  // The caller must free this memory later
}
