#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "data_fetcher.h"
#include "../utils/cJSON/cJSON.h"
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
  if (json_data == NULL) {
    printf("Failed to fetch data from Google Sheets.\n");
    return NULL;
  }

  // printf("Data fetched from Google Sheets:\n%s\n", json_data);

  // Parse the JSON data
  cJSON *root = cJSON_Parse(json_data);
  if (root == NULL) {
    perror("Failed to parse JSON");
    free(json_data); // Free the raw JSON string only once here
    return NULL;
  }

  // Get the "values" array
  cJSON *valuesArray = cJSON_GetObjectItem(root, "values");
  if (!cJSON_IsArray(valuesArray)) {
    cJSON_Delete(root);
    free(json_data); // Free the JSON data
    perror("Values array is not found or is invalid");
    return NULL;
  }

  // Get the number of items in the array
  int rowCount = cJSON_GetArraySize(valuesArray);

  // Allocate memory for the 2D array
  char ***result = (char ***)malloc(rowCount * sizeof(char **));
  if (result == NULL) {
    cJSON_Delete(root);
    free(json_data); // Free the JSON data
    perror("Failed to allocate memory for routes");
    return NULL;
  }

  // Iterate over the "values" array and extract each row's details
  for (int i = 0; i < rowCount; i++) {
    cJSON *row = cJSON_GetArrayItem(valuesArray, i);
    if (!cJSON_IsArray(row)) {
      continue;
    }

    // Allocate memory for each row (id, name, link)
    result[i] = (char **)malloc(3 * sizeof(char *));
    result[i][0] = strdup(cJSON_GetArrayItem(row, 0)->valuestring); // id
    result[i][1] = strdup(cJSON_GetArrayItem(row, 1)->valuestring); // name
    result[i][2] = strdup(cJSON_GetArrayItem(row, 2)->valuestring); // link
  }

  // Clean up the JSON object and the raw JSON string
  cJSON_Delete(root);
  free(json_data); // Free the JSON data after we're done with it

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
