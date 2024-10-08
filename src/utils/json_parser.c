#include <stdio.h>
#include "cJSON/cJSON.h"  // Include cJSON for JSON parsing
#include "../include/json_parser.h"

void parse_json(const char *json_data) {
    // Parse JSON using cJSON
    cJSON *root = cJSON_Parse(json_data);
    if (root == NULL) {
        fprintf(stderr, "Error parsing JSON\n");
        return;
    }

    // Get the values array
    cJSON *values = cJSON_GetObjectItem(root, "values");
    if (values) {
        int rows = cJSON_GetArraySize(values);
        for (int i = 0; i < rows; i++) {
            cJSON *row = cJSON_GetArrayItem(values, i);
            int columns = cJSON_GetArraySize(row);
            for (int j = 0; j < columns; j++) {
                cJSON *cell = cJSON_GetArrayItem(row, j);
                printf("%s\t", cell->valuestring);
            }
            printf("\n");
        }
    }

    cJSON_Delete(root);  // Free the JSON object
}
