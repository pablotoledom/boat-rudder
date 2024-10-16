#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdbool.h>

int load_config(const char *filename);

// Declare global variables
extern int verbose_level; // 0: no log, 1: Errors, 2: Warnings, 3: Info and data, 4: Debug full verbose
extern int server_port;
extern char spreadsheet_id[128];
extern char api_key[128];
extern char theme[16];
extern char lang[16];

#endif // CONFIG_LOADER_H
