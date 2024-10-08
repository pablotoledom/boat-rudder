#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/route.h"
#include "../../include/generate_url_theme.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *menu() {
  // Get templates
  char *filename_menu_html = generate_url_theme("menu/menu_std2.html");
  const char *menu_response = read_file_to_string(filename_menu_html);
  free(filename_menu_html);

  char *filename_menu_item_html =
      generate_url_theme("menu/menu-item_std2.html");
  const char *menu_item_response = read_file_to_string(filename_menu_item_html);
  free(filename_menu_item_html);

  char *filename_item_separator_html =
      generate_url_theme("menu/menu-item-separator_std2.html");
  const char *menu_item_separator_response =
      read_file_to_string(filename_item_separator_html);
  free(filename_item_separator_html);

  if (!menu_response || !menu_item_response || !menu_item_separator_response) {
    perror("Failed to load HTML templates");
    // Free allocated templates if any
    if (menu_response)
      free((void *)menu_response);
    if (menu_item_response)
      free((void *)menu_item_response);
    if (menu_item_separator_response)
      free((void *)menu_item_separator_response);
    return NULL;
  }

  // Get menu list
  int routeCount = 0;
  Route *routes = getRoutes(&routeCount);

  if (routes == NULL) {
    perror("Failed to get routes");
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    return NULL;
  }

  // Buffer to hold all items
  char *itemsBuffer = NULL;
  size_t itemsBufferSize = 0;

  // Generate the menu routes
  for (int i = 0; i < routeCount; i++) {
    char itemBuffer[512]; // Buffer to hold a single item
    int itemLength =
        snprintf(itemBuffer, sizeof(itemBuffer), menu_item_response,
                 routes[i].link, routes[i].name,
                 i < (routeCount - 1) ? menu_item_separator_response : "");

    if (itemLength < 0) {
      perror("Error formatting item");
      free(itemsBuffer);
      free(routes);
      free((void *)menu_response);
      free((void *)menu_item_response);
      free((void *)menu_item_separator_response);
      return NULL;
    }

    // Reallocate memory for itemsBuffer
    char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength +
                                                1); // +1 for null terminator
    if (tempBuffer == NULL) {
      perror("Failed to allocate memory for itemsBuffer");
      free(itemsBuffer);
      free(routes);
      free((void *)menu_response);
      free((void *)menu_item_response);
      free((void *)menu_item_separator_response);
      return NULL;
    }

    itemsBuffer = tempBuffer;

    // Copy the new item into itemsBuffer
    memcpy(itemsBuffer + itemsBufferSize, itemBuffer, itemLength);
    itemsBufferSize += itemLength;
    itemsBuffer[itemsBufferSize] = '\0'; // Null-terminate the string
  }

  // Allocate buffer for the full HTML content
  size_t contentBufferSize = snprintf(NULL, 0, menu_response, itemsBuffer) +
                             1; // +1 for null terminator
  char *contentBuffer = malloc(contentBufferSize);
  if (contentBuffer == NULL) {
    perror("Failed to allocate memory for contentBuffer");
    free(itemsBuffer);
    free(routes);
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    return NULL;
  }

  // Format the final HTML content
  snprintf(contentBuffer, contentBufferSize, menu_response, itemsBuffer);

  // Free the itemsBuffer as it's no longer needed
  free(itemsBuffer);

  // Free the routes
  free(routes);

  // Free the templates
  free((void *)menu_response);
  free((void *)menu_item_response);
  free((void *)menu_item_separator_response);

  return contentBuffer;
}
