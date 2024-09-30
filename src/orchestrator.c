#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "modules/blog_home/blog_home.h"
#include "modules/container/container.h"
#include "modules/menu/menu.h"
#include "modules/slider/slider.h"
#include <stdio.h>  // For snprintf
#include <stdlib.h> // For malloc and free
#include <string.h> // For strlen

const char *buildWebSite() {
  // Get the HTML content from the container module (template with two %s
  // placeholders)
  const char *response_container =
      container();                    // Assume it contains two %s placeholders
  const char *response_menu = menu(); // HTML content for the menu
  const char *response_slider = slider(); // HTML content for the slider
  const char *response_blog_home =
      blog_home(); // HTML content for the blog_home

  // Calculate the total size needed for the formatted string
  size_t total_length = snprintf(NULL, 0, response_container, response_menu,
                                 response_slider, response_blog_home) +
                        1; // +1 for null terminator

  // Allocate memory for the formatted response
  char *buffer = malloc(total_length);
  if (buffer == NULL) {
    perror("Failed to allocate memory");
    return NULL;
  }

  // Format the string by replacing %s in response_container with response_menu
  // and response_slider
  snprintf(buffer, total_length, response_container, response_menu,
           response_slider, response_blog_home);

  // Create a buffer large enough to hold the entire HTTP response (headers +
  // body)
  static char root_response[4096]; // Adjust if your content is larger

  // Build the HTTP response
  int header_len =
      snprintf(root_response, sizeof(root_response),
               "HTTP/1.1 200 OK\r\n"
               "Content-Type: text/html\r\n"
               "Content-Length: %zu\r\n" // Dynamically calculate content length
               "Connection: close\r\n"
               "\r\n",
               strlen(buffer));

  // Concatenate the HTML content (the body) to the HTTP headers
  snprintf(root_response + header_len, sizeof(root_response) - header_len, "%s",
           buffer);

  // Free the dynamically allocated memory for buffer
  free(buffer);

  return root_response;
}
