#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/config_loader.h"
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Define global variables
int verbose_level = 0;          // Default is disabled
int http_port = 0;            // Default value for the port
int https_port = 0;               // Default value for the port
char spreadsheet_id[128] = {0}; // Initialize as an empty string
char api_key[128] = {0};        // Initialize as an empty string
char theme[16] = {0};           // Initialize as an empty string
char lang[16] = {0};            // Initialize as an empty string
bool ssl_enabled = false;       // Default disabled ssl
char ssl_cert[256] = {0};       // Initialize as an empty string
char ssl_key[256] = {0};        // Initialize as an empty string

// Implementation of load_config
int load_config(const char *filename) {
  FILE *file = fopen(filename, "r");
  if (file == NULL) {
    perror("Failed to open config file");
    return -1; // Error opening the file
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
      continue; // If there is no '=', it's not a valid line
    }

    *delimiter = '\0'; // Replace '=' with the null terminator
    char *key = line;
    char *value = delimiter + 1;

    // Remove newline characters from the value
    value[strcspn(value, "\n")] = '\0';

    // Assign values based on the key
    if (strcmp(key, "verbose_level") == 0) {
      verbose_level = atoi(value); // Convert to integer
    } else if (strcmp(key, "http_port") == 0) {
      http_port = atoi(value); // Convert to integer
    } else if (strcmp(key, "spreadsheet_id") == 0) {
      strncpy(spreadsheet_id, value, sizeof(spreadsheet_id) - 1);
      spreadsheet_id[sizeof(spreadsheet_id) - 1] =
          '\0'; // Ensure null terminator
    } else if (strcmp(key, "api_key") == 0) {
      strncpy(api_key, value, sizeof(api_key) - 1);
      api_key[sizeof(api_key) - 1] = '\0'; // Ensure null terminator
    } else if (strcmp(key, "theme") == 0) {
      strncpy(theme, value, sizeof(theme) - 1);
      theme[sizeof(theme) - 1] = '\0'; // Ensure null terminator
    } else if (strcmp(key, "lang") == 0) {
      strncpy(lang, value, sizeof(lang) - 1);
      lang[sizeof(lang) - 1] = '\0'; // Ensure null terminator
    } else if (strcmp(key, "ssl_enabled") == 0) {
      ssl_enabled = atoi(value); // 0 = disabled, 1 = enabled
    } else if (strcmp(key, "ssl_cert") == 0) {
      strncpy(ssl_cert, value, sizeof(ssl_cert) - 1);
      ssl_cert[sizeof(ssl_cert) - 1] = '\0'; // Ensures null termination
    } else if (strcmp(key, "ssl_key") == 0) {
      strncpy(ssl_key, value, sizeof(ssl_key) - 1);
      ssl_key[sizeof(ssl_key) - 1] = '\0'; // Ensures null termination
    } else if (strcmp(key, "https_port") == 0) {
      https_port = atoi(value);
    }
  }

  fclose(file);
  return 0;
}
