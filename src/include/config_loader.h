#ifndef CONFIG_LOADER_H
#define CONFIG_LOADER_H

#include <stdbool.h>

int load_config(const char *filename);

// Declare global variables
extern bool verbose;
extern int server_port;
extern char spreadsheet_id[128];
extern char api_key[128];

#endif
