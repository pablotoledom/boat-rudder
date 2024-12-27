#include "../include/generate_url_theme.h"
#include "../include/config_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to generate the complete URL
char *generate_url_theme(const char *subpath, int epoch) {
  // Template for the base path
  const char *base_path = "./html/themes/%s/";

  // Calculate the necessary size for the base path with the theme
  int size_base_path = snprintf(NULL, 0, base_path, theme);

  // Calculate the necessary size for the subpath with the epoch
  int size_subpath = snprintf(NULL, 0, subpath, epoch);

  // Calculate the total size needed for the complete URL, including null terminator
  int needed_size = size_base_path + size_subpath + 1;

  // Allocate dynamic memory for the complete URL
  char *full_url = (char *)malloc(needed_size);

  if (full_url == NULL) {
    printf("Error: Could not allocate memory\n");
    return NULL;
  }

  // Build the complete URL: first the base path
  snprintf(full_url, size_base_path + 1, base_path, theme);

  // Append the subpath with epoch
  snprintf(full_url + size_base_path, size_subpath + 1, subpath, epoch);

  return full_url; // Return the generated URL
}
