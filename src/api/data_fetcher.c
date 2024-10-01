#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "data_fetcher.h"
#include "../utils/cJSON/cJSON.h"
#include "../include/config_loader.h"
#include "google_sheets_api.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to parse JSON data and return a 2D array
char ***getData(const char *sheetName, const char *startCell,
                const char *endCell, int *rowCount) {
  // Build query
  int size = snprintf(NULL, 0, "%s!%s:%s", sheetName, startCell, endCell) + 1;

  // Asignar memoria para la cadena resultante
  char *range = (char *)malloc(size * sizeof(char));
  if (range == NULL) {
    perror("Failed to allocate memory");
    exit(1); // Terminar el programa si no hay suficiente memoria
  }

  // Formatear la cadena
  snprintf(range, size, "%s!%s:%s", sheetName, startCell, endCell);

  char spreadsheet_id[128];
    char api_key[128];

    // Cargar las configuraciones desde config.txt
    load_config(spreadsheet_id, api_key);

  // Fetch data from Google Sheets
  char *json_data = fetch_google_sheets_data(spreadsheet_id, range, api_key);
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
