#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "../include/config_loader.h"

// Define global variables
int verbose_level = 0;  // Default is disabled
int server_port = 0;  // Default value for the port
char spreadsheet_id[128] = {0};  // Initialize as an empty string
char api_key[128] = {0};  // Initialize as an empty string

// Implementation of load_config
int load_config(const char *filename) {
    FILE *file = fopen(filename, "r");
    if (file == NULL) {
        perror("Failed to open config file");
        return -1;  // Error opening the file
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        // Ignore empty lines or comments
        if (line[0] == '\n' || line[0] == '#') {
            continue;
        }

        // Look for '=' to split key and value
        char *delimiter = strchr(line, '=');
        if (delimiter == NULL) {
            continue;  // If there is no '=', it's not a valid line
        }

        *delimiter = '\0';  // Replace '=' with the null terminator
        char *key = line;
        char *value = delimiter + 1;

        // Remove newline characters from the value
        value[strcspn(value, "\n")] = '\0';

        // Assign values based on the key
        if (strcmp(key, "verbose_level") == 0) {
            verbose_level = atoi(value); // Convert to integer
        } else if (strcmp(key, "server_port") == 0) {
            server_port = atoi(value);  // Convert to integer
        } else if (strcmp(key, "spreadsheet_id") == 0) {
            strncpy(spreadsheet_id, value, sizeof(spreadsheet_id) - 1);
            spreadsheet_id[sizeof(spreadsheet_id) - 1] = '\0';  // Ensure null terminator
        } else if (strcmp(key, "api_key") == 0) {
            strncpy(api_key, value, sizeof(api_key) - 1);
            api_key[sizeof(api_key) - 1] = '\0';  // Ensure null terminator
        }
    }

    fclose(file);
    return 0;
}
