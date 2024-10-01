#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

// Function to read a text file and load its content into a dynamically
// allocated string
char *read_file_to_string(const char *filename) {
  // Open the file in read mode
  FILE *file = fopen(filename, "r");

  if (file == NULL) {
    perror("Failed to open file");
    return NULL;
  }

  // Move the file pointer to the end to determine the file size
  fseek(file, 0, SEEK_END);
  long file_size = ftell(file);
  rewind(file); // Move back to the start

  // Allocate memory for the file contents (+1 for the null terminator)
  char *buffer = malloc(file_size + 1);

  if (buffer == NULL) {
    perror("Failed to allocate memory");
    fclose(file);
    return NULL;
  }

  // Read the file into the buffer
  size_t read_size = fread(buffer, 1, file_size, file);
  buffer[read_size] = '\0'; // Null-terminate the string

  // Close the file
  fclose(file);

  return buffer; // The caller is responsible for freeing the buffer
}
