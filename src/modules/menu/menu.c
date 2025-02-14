#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/route.h"
#include "../../include/generate_url_theme.h"
#include "../../include/log.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *menu(char *url, int epoch, char *lang, char *theme) {
  LOG_DEBUG("Menu route: %s\n", url);
  // Load HTML templates
  char *filename_menu_html = generate_url_theme("menu/menu_epoch%d.html", epoch);
  const char *menu_response = read_file_to_string(filename_menu_html);
  free(filename_menu_html);

  char *filename_menu_item_html = generate_url_theme("menu/menu-item_epoch%d.html", epoch);
  const char *menu_item_response = read_file_to_string(filename_menu_item_html);
  free(filename_menu_item_html);

  char *filename_menu_item_selected_html = generate_url_theme("menu/menu-item-selected_epoch%d.html", epoch);
  const char *menu_item_selected_response = read_file_to_string(filename_menu_item_selected_html);
  free(filename_menu_item_selected_html);

  char *filename_item_separator_html = generate_url_theme("menu/menu-item-separator_epoch%d.html", epoch);
  const char *menu_item_separator_response = read_file_to_string(filename_item_separator_html);
  free(filename_item_separator_html);

  char *filename_option_html = generate_url_theme("menu/menu-option_epoch%d.html", epoch);
  const char *menu_option_response = read_file_to_string(filename_option_html);
  free(filename_option_html);

  if (!menu_response || !menu_item_response || !menu_item_separator_response ||
      !menu_item_selected_response || !menu_option_response) {
    perror("Failed to load HTML templates");
    // Free any loaded templates if necessary
    if (menu_response)
      free((void *)menu_response);
    if (menu_item_response)
      free((void *)menu_item_response);
    if (menu_item_separator_response)
      free((void *)menu_item_separator_response);
    if (menu_item_selected_response)
      free((void *)menu_item_selected_response);
    if (menu_option_response)
      free((void *)menu_option_response);
    return NULL;
  }

  // Get the list of routes
  int routeCount = 0;
  Route *routes = getRoutes(&routeCount);
  if (routes == NULL) {
    perror("Failed to get routes");
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    free((void *)menu_item_selected_response);
    free((void *)menu_option_response);
    return NULL;
  }

  // Buffer to hold route menu items
  char *itemsBuffer = NULL;
  size_t itemsBufferSize = 0;

  // Generate menu items from routes
  for (int i = 0; i < routeCount; i++) {
    char itemBuffer[512]; // Buffer for a single item
    int itemLength = snprintf(
        itemBuffer, sizeof(itemBuffer),
        strcmp(url, routes[i].link) == 0 ? menu_item_selected_response : menu_item_response,
        routes[i].link, routes[i].name,
        i < (routeCount - 1) ? menu_item_separator_response : "");
    if (itemLength < 0) {
      perror("Error formatting item");
      free(itemsBuffer);
      free(routes);
      free((void *)menu_response);
      free((void *)menu_item_response);
      free((void *)menu_item_separator_response);
      free((void *)menu_item_selected_response);
      free((void *)menu_option_response);
      return NULL;
    }
    char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength + 1); // +1 for null terminator
    if (tempBuffer == NULL) {
      perror("Failed to allocate memory for itemsBuffer");
      free(itemsBuffer);
      free(routes);
      free((void *)menu_response);
      free((void *)menu_item_response);
      free((void *)menu_item_separator_response);
      free((void *)menu_item_selected_response);
      free((void *)menu_option_response);
      return NULL;
    }
    itemsBuffer = tempBuffer;
    memcpy(itemsBuffer + itemsBufferSize, itemBuffer, itemLength);
    itemsBufferSize += itemLength;
    itemsBuffer[itemsBufferSize] = '\0'; // Ensure null termination
  }

  // Create a separate buffer for additional options (language and theme)
  char *optionsBuffer = NULL;
  size_t optionsBufferSize = 0;
  char optionItem[512];
  int optionLength = 0;

  // Language option
  if (strcmp(lang, "Esp") == 0) {
    // If current language is Spanish, show option for English
    optionLength = snprintf(optionItem, sizeof(optionItem),
                              menu_option_response, "lang=Eng", "english", "English");
  } else {
    // If current language is English, show option for Spanish
    optionLength = snprintf(optionItem, sizeof(optionItem),
                              menu_option_response, "lang=Esp", "spanish", "Español");
  }
  char *tempOptionsBuffer = realloc(optionsBuffer, optionsBufferSize + optionLength + 1);
  if (tempOptionsBuffer == NULL) {
    perror("Failed to allocate memory for language option");
    free(optionsBuffer);
    free(itemsBuffer);
    free(routes);
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    free((void *)menu_item_selected_response);
    free((void *)menu_option_response);
    return NULL;
  }
  optionsBuffer = tempOptionsBuffer;
  memcpy(optionsBuffer + optionsBufferSize, optionItem, optionLength);
  optionsBufferSize += optionLength;
  optionsBuffer[optionsBufferSize] = '\0';

  // Theme option
  int themeOptionLength = 0;
  if (strcmp(theme, "dark") == 0) {
    // If current theme is dark, show option for light
    themeOptionLength = snprintf(optionItem, sizeof(optionItem),
                                   menu_option_response, "theme=light", "light", "Light Mode");
  } else {
    // If current theme is light, show option for dark
    themeOptionLength = snprintf(optionItem, sizeof(optionItem),
                                   menu_option_response, "theme=dark", "dark", "Dark Mode");
  }
  tempOptionsBuffer = realloc(optionsBuffer, optionsBufferSize + themeOptionLength + 1);
  if (tempOptionsBuffer == NULL) {
    perror("Failed to allocate memory for theme option");
    free(optionsBuffer);
    free(itemsBuffer);
    free(routes);
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    free((void *)menu_item_selected_response);
    free((void *)menu_option_response);
    return NULL;
  }
  optionsBuffer = tempOptionsBuffer;
  memcpy(optionsBuffer + optionsBufferSize, optionItem, themeOptionLength);
  optionsBufferSize += themeOptionLength;
  optionsBuffer[optionsBufferSize] = '\0';

  // Create final HTML content by concatenating itemsBuffer and optionsBuffer into the template
  size_t contentBufferSize = snprintf(NULL, 0, menu_response, itemsBuffer, optionsBuffer) + 1; // +1 for null terminator
  char *contentBuffer = malloc(contentBufferSize);
  if (contentBuffer == NULL) {
    perror("Failed to allocate memory for contentBuffer");
    free(optionsBuffer);
    free(itemsBuffer);
    free(routes);
    free((void *)menu_response);
    free((void *)menu_item_response);
    free((void *)menu_item_separator_response);
    free((void *)menu_item_selected_response);
    free((void *)menu_option_response);
    return NULL;
  }
  snprintf(contentBuffer, contentBufferSize, menu_response, itemsBuffer, optionsBuffer);

  // Free allocated resources
  free(optionsBuffer);
  free(itemsBuffer);
  free(routes);
  free((void *)menu_response);
  free((void *)menu_item_response);
  free((void *)menu_item_separator_response);
  free((void *)menu_item_selected_response);
  free((void *)menu_option_response);

  return contentBuffer;
}
