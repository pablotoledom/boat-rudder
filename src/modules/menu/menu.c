#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/route.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *menu() {
  // Get templates
  const char *menu_response = read_file_to_string("./html/menu_template.html");
  const char *menu_item_response =
      read_file_to_string("./html/menu_item_template.html");

  // Get menu
  int routeCount = 0;
  Route *routes = getRoutes(&routeCount);

  // Buffer to hold all rows
  char *rowsBuffer = NULL;
  size_t rowsBufferSize = 0;

  // Generate the menu routes
  for (int i = 0; i < routeCount; i++) {
    char itemBuffer[512]; // Buffer to hold a single <td> element
    snprintf(itemBuffer, sizeof(itemBuffer), menu_item_response, routes[i].link,
             routes[i].name);

    // printf("itemBuffer: %s\n", itemBuffer);

    // Reallocate memory to concatenate each row into the rowsBuffer
    size_t itemLength = strlen(itemBuffer);
    rowsBuffer = realloc(rowsBuffer, rowsBufferSize + itemLength +
                                         1); // +1 for null terminator
    if (rowsBuffer == NULL) {
      perror("Failed to allocate memory for rowsBuffer");
      return NULL;
    }

    // Append the item to rowsBuffer
    strcpy(rowsBuffer + rowsBufferSize, itemBuffer);
    rowsBufferSize += itemLength;
  }

  // Allocate buffer for the full HTML table (including the table template)
  size_t tableBufferSize = snprintf(NULL, 0, menu_response, rowsBuffer) +
                           1; // +1 for null terminator
  char *tableBuffer = malloc(tableBufferSize);
  if (tableBuffer == NULL) {
    perror("Failed to allocate memory for tableBuffer");
    free(rowsBuffer); // Free the rowsBuffer before exiting
    return NULL;
  }

  // Format the final HTML table string
  snprintf(tableBuffer, tableBufferSize, menu_response, rowsBuffer);

  // Free the rowsBuffer as it's no longer needed
  free(rowsBuffer);

  // Free the routes
  free(routes);

  return tableBuffer;
}
