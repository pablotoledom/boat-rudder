#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "data_fetcher.h"
#include "../include/config_loader.h"
#include "../include/log.h"
#include "../utils/cJSON/cJSON.h"
#include "google_sheets_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to parse JSON data and return a 2D array
char ***getData(const char *sheetName, const char *startCell,
                const char *endCell, int *rowCount) {
  // printf("Idioma seleccionado: %s\n", lang);
  // Build query
  int size =
      snprintf(NULL, 0, "%s%s!%s:%s", sheetName, lang, startCell, endCell) + 1;

  // Allocate memory for the resulting string
  char *range = (char *)malloc(size * sizeof(char));
  if (range == NULL) {
    perror("Failed to allocate memory");
    // exit(1); // Exiting immediately is not ideal.
    return NULL; // Exit gracefully without crashing the entire program
  }

  // Format the string
  snprintf(range, size, "%s%s!%s:%s", sheetName, lang, startCell, endCell);

  // Fetch data from Google Sheets
  char *json_data = fetch_google_sheets_data(range);
  if (json_data == NULL) {
    printf("Failed to fetch data from Google Sheets.\n");
    free(range); // Free the allocated memory for 'range'
    return NULL;
  }

  LOG_DEBUG("JSON Response API: %s", json_data);

  // Parse the JSON data
  cJSON *root = cJSON_Parse(json_data);
  if (root == NULL) {
    perror("Failed to parse JSON");
    free(json_data);
    free(range);
    return NULL;
  }

  // Get the "values" array
  cJSON *valuesArray = cJSON_GetObjectItem(root, "values");
  if (!cJSON_IsArray(valuesArray)) {
    cJSON_Delete(root);
    free(json_data);
    free(range);
    perror("Values array is not found or is invalid");
    return NULL;
  }

  // Set rowCount
  *rowCount = cJSON_GetArraySize(valuesArray);

  // Allocate memory for the 2D array (+1 for NULL terminator)
  char ***result = (char ***)malloc((*rowCount + 1) * sizeof(char **));
  if (result == NULL) {
    cJSON_Delete(root);
    free(json_data);
    free(range);
    perror("Failed to allocate memory for result");
    return NULL;
  }

  // Build result 2D Array
  for (int i = 0; i < *rowCount; i++) {
    cJSON *row = cJSON_GetArrayItem(valuesArray, i);
    if (!cJSON_IsArray(row)) {
      result[i] = NULL;
      continue;
    }

    int columnCount = cJSON_GetArraySize(row); // Number of columns in this row

    // Allocate memory for each row with a dynamic number of columns (+1 for
    // NULL terminator)
    result[i] = (char **)malloc((columnCount + 1) * sizeof(char *));
    if (result[i] == NULL) {
      perror("Failed to allocate memory for row");
      // Free allocated memory for the previous rows and the result array
      for (int j = 0; j < i; j++) {
        if (result[j] != NULL) {
          for (int k = 0; result[j][k] != NULL; k++) {
            free(result[j][k]);
          }
          free(result[j]);
        }
      }
      free(result);
      cJSON_Delete(root);
      free(json_data);
      free(range);
      return NULL;
    }

    // Copy each column value into result and add NULL terminator
    for (int j = 0; j < columnCount; j++) {
      cJSON *cell = cJSON_GetArrayItem(row, j);
      if (cell != NULL && cell->valuestring != NULL) {
        result[i][j] = strdup(cell->valuestring);
      } else {
        result[i][j] = strdup("");
      }
      if (result[i][j] == NULL) {
        perror("Failed to allocate memory for cell");
        // Free allocated memory
        for (int k = 0; k <= j; k++) {
          free(result[i][k]);
        }
        free(result[i]);
        // Free previous rows
        for (int m = 0; m < i; m++) {
          if (result[m] != NULL) {
            for (int n = 0; result[m][n] != NULL; n++) {
              free(result[m][n]);
            }
            free(result[m]);
          }
        }
        free(result);
        cJSON_Delete(root);
        free(json_data);
        free(range);
        return NULL;
      }
    }
    result[i][columnCount] = NULL; // Add NULL terminator for the row
  }

  // Add NULL terminator for the entire table
  result[*rowCount] = NULL;

  // Clean up
  cJSON_Delete(root);
  free(json_data);
  free(range);

  return result;
}

// Function to free the dynamically allocated data
void freeData(char ***data, int rows) {
  if (data == NULL)
    return;

  for (int i = 0; i < rows; i++) {
    if (data[i] != NULL) {
      for (int j = 0; data[i][j] != NULL; j++) {
        free(data[i][j]);
      }
      free(data[i]);
    }
  }
  free(data);
}
