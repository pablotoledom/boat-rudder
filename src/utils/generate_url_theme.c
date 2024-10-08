#include "../include/generate_url_theme.h"
#include "../include/config_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to generate the complete URL
char *generate_url_theme(const char *subpath) {
  // Template for the base path
  const char *base_path = "./html/themes/%s/";

  // Calculate the necessary size for the complete URL
  int needed_size = snprintf(NULL, 0, base_path, theme) + strlen(subpath);

  // Allocate dynamic memory for the complete URL
  char *full_url = (char *)malloc(needed_size + 1); // +1 for the null character

  if (full_url == NULL) {
    printf("Error: Could not allocate memory\n");
    return NULL;
  }

  // Build the complete URL
  sprintf(full_url, base_path, theme);
  strcat(full_url, subpath); // Concatenate the subpath

  return full_url; // Return the generated URL
}