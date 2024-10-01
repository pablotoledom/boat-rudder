#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to load configurations from googlesheets.txt
void load_config(char *spreadsheet_id, char *api_key) {
    FILE *file = fopen("./configs/googlesheets.txt", "r");
    if (file == NULL) {
        perror("Failed to open config file");
        exit(1);
    }

    char line[256];
    while (fgets(line, sizeof(line), file)) {
        if (strncmp(line, "spreadsheet_id=", 15) == 0) {
            strcpy(spreadsheet_id, line + 15);
            spreadsheet_id[strcspn(spreadsheet_id, "\n")] = 0;  // Remove newline
        } else if (strncmp(line, "api_key=", 8) == 0) {
            strcpy(api_key, line + 8);
            api_key[strcspn(api_key, "\n")] = 0;  // Remove newline
        }
    }

    fclose(file);
}
