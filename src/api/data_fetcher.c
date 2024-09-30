#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "data_fetcher.h"
#include "../include/json_parser.h"
#include "cjson/cJSON.h"
#include "google_sheets_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *spreadsheet_id = "1kOHas0XqFkiHdaE4RpWgB4dvNL731EnuGEIBkKpaRGw";
const char *range = "Routes!A3:C20"; // Example range
const char *api_key =
    "AIzaSyBMp9XaDQ8V4Rn6hU6TjI_mUSTqZg_RA0Y"; // Replace with your actual API
                                               // key

// Function to parse JSON data and return a 2D array
char ***getData() {
  // Fetch data from Google Sheets
  char *json_data = fetch_google_sheets_data(spreadsheet_id, range, api_key);
  if (json_data != NULL) {
    printf("Data fetched from Google Sheets:\n%s\n", json_data);

    // Parse the JSON data
    parse_json(json_data);

    // Free the fetched data
    free(json_data);
  } else {
    printf("Failed to fetch data from Google Sheets.\n");
  }

  // Parse the JSON data
  cJSON *root = cJSON_Parse(json_data);
  free(json_data); // Free the raw JSON string after parsing
  if (!root) {
    perror("Failed to parse JSON");
    return NULL;
  }

  // Get the array of routes
  cJSON *routesArray = cJSON_GetObjectItem(root, "routes");
  if (!cJSON_IsArray(routesArray)) {
    cJSON_Delete(root);
    perror("Routes array is not found or is invalid");
    return NULL;
  }

  // Get the number of items in the array
  int routeCount = cJSON_GetArraySize(routesArray);

  // Allocate memory for the 2D array
  char ***result = (char ***)malloc(routeCount * sizeof(char **));
  if (!result) {
    cJSON_Delete(root);
    perror("Failed to allocate memory for routes");
    return NULL;
  }

  // Iterate over the array and extract each route's details
  for (int i = 0; i < routeCount; i++) {
    cJSON *route = cJSON_GetArrayItem(routesArray, i);
    if (!cJSON_IsArray(route)) {
      continue;
    }

    // Allocate memory for each row (id, name, link)
    result[i] = (char **)malloc(3 * sizeof(char *));
    result[i][0] = strdup(cJSON_GetArrayItem(route, 0)->valuestring); // id
    result[i][1] = strdup(cJSON_GetArrayItem(route, 1)->valuestring); // name
    result[i][2] = strdup(cJSON_GetArrayItem(route, 2)->valuestring); // link
  }

  // Clean up the JSON object
  cJSON_Delete(root);

  return result;
}

// Function to free the dynamically allocated data
void freeData(char ***data, int rows) {
  for (int i = 0; i < rows; i++) {
    for (int j = 0; j < 3; j++) {
      free(data[i][j]);
    }
    free(data[i]);
  }
  free(data);
}
