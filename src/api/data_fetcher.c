#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "data_fetcher.h"
#include "../utils/cJSON/cJSON.h"
#include "google_sheets_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to parse JSON data and return a 2D array
char ***getData(const char *sheetName, const char *startCell,
                const char *endCell, int *rowCount) {
  // Build query
  int size = snprintf(NULL, 0, "%s!%s:%s", sheetName, startCell, endCell) + 1;

  // Allocate memory for the resulting string
  char *range = (char *)malloc(size * sizeof(char));
  if (range == NULL) {
    perror("Failed to allocate memory");
    exit(1); // Terminate the program if there is not enough memory
  }

  // Format the string
  snprintf(range, size, "%s!%s:%s", sheetName, startCell, endCell);

  // Fetch data from Google Sheets
  char *json_data = fetch_google_sheets_data(range);
  if (json_data == NULL) {
    printf("Failed to fetch data from Google Sheets.\n");
    return NULL;
  }

  // Parse the JSON data
  cJSON *root = cJSON_Parse(json_data);
  if (root == NULL) {
    perror("Failed to parse JSON");
    free(json_data);
    return NULL;
  }

  // Get the "values" array
  cJSON *valuesArray = cJSON_GetObjectItem(root, "values");
  if (!cJSON_IsArray(valuesArray)) {
    cJSON_Delete(root);
    free(json_data);
    perror("Values array is not found or is invalid");
    return NULL;
  }

  // Set rowCount
  *rowCount = cJSON_GetArraySize(valuesArray);

  // Allocate memory for the 2D array
  char ***result = (char ***)malloc((*rowCount) * sizeof(char **));
  if (result == NULL) {
    cJSON_Delete(root);
    free(json_data);
    perror("Failed to allocate memory for routes");
    return NULL;
  }

  // Build result 2D Array
  for (int i = 0; i < *rowCount; i++) {
    cJSON *row = cJSON_GetArrayItem(valuesArray, i);
    if (!cJSON_IsArray(row)) {
      continue;
    }

    result[i] = (char **)malloc(3 * sizeof(char *));
    result[i][0] = strdup(cJSON_GetArrayItem(row, 0)->valuestring); // id
    result[i][1] = strdup(cJSON_GetArrayItem(row, 1)->valuestring); // name
    result[i][2] = strdup(cJSON_GetArrayItem(row, 2)->valuestring); // link
  }

  cJSON_Delete(root);

  // Freeing memory
  free(json_data);

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
