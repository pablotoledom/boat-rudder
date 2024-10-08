#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "include/log.h"
// #include "modules/blog/blog.h"
#include "modules/blog/blog_entry.h"
#include "modules/container/container.h"
#include "modules/home_blog/home_blog.h"
#include "modules/menu/menu.h"
#include "modules/slider/slider.h"
#include <stdio.h>  // For snprintf
#include <stdlib.h> // For malloc and free
#include <string.h> // For strlen

const char *buildHomeWebSite() {
  // Get the HTML content from the modules
  const char *html_container = container();
  const char *html_menu = menu();
  const char *html_slider = slider();
  const char *html_home_blog = home_blog();

  // Check if any component returned NULL
  if (!html_container || !html_menu || !html_slider || !html_home_blog) {
    printf("Error buildHomeWebSite: One or more HTML components are NULL.\n");
    // Free allocated components if any
    if (html_container)
      free((void *)html_container);
    if (html_menu)
      free((void *)html_menu);
    if (html_slider)
      free((void *)html_slider);
    if (html_home_blog)
      free((void *)html_home_blog);
    return NULL;
  }

  // Calculate the total size needed for the formatted string
  size_t total_length = snprintf(NULL, 0, html_container, html_menu,
                                 html_slider, html_home_blog) +
                        1; // +1 for null terminator

  LOG_DEBUG("Total length to allocate: %zu\n", total_length);

  // Allocate memory for the formatted response
  char *buffer = malloc(total_length);

  if (buffer == NULL) {
    perror("Failed to allocate memory");
    // Free allocated components
    free((void *)html_container);
    free((void *)html_menu);
    free((void *)html_slider);
    free((void *)html_home_blog);
    return NULL;
  }

  // Format the string by replacing %s in html_container with other
  // components
  snprintf(buffer, total_length, html_container, html_menu, html_slider,
           html_home_blog);

  // Free the component strings as they are no longer needed
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_slider);
  free((void *)html_home_blog);

  // Calculate total size needed for HTTP headers + body
  size_t header_len = snprintf(NULL, 0,
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %zu\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               strlen(buffer)) +
                      1; // +1 for null terminator

  // Calculate total size for the entire HTTP response
  size_t total_html_size = header_len + strlen(buffer) + 1;

  // Dynamically allocate the response buffer instead of using a static one
  char *root_response = malloc(total_html_size);

  if (root_response == NULL) {
    perror("Failed to allocate memory for HTTP response");
    free(buffer); // Don't forget to free the previous buffer
    return NULL;
  }

  // Build the HTTP response with headers and body
  snprintf(root_response, total_html_size,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n%s",
           strlen(buffer), buffer);

  // Free the dynamically allocated memory for buffer
  free(buffer);

  return root_response;
}

const char *buildBlogWebSite() {
  // Get the HTML content from the modules
  const char *html_container = container();
  const char *html_menu = menu();
  const char *html_home_blog = home_blog();

  // Check if any component returned NULL
  if (!html_container || !html_menu || !html_home_blog) {
    printf("Error buildBlogWebSite: One or more HTML components are NULL.\n");
    // Free allocated components if any
    if (html_container)
      free((void *)html_container);
    if (html_menu)
      free((void *)html_menu);
    if (html_home_blog)
      free((void *)html_home_blog);
    return NULL;
  }

  // Calculate the total size needed for the formatted string
  size_t total_length = snprintf(NULL, 0, html_container, html_menu, "",
                                 html_home_blog) +
                        1; // +1 for null terminator

  LOG_DEBUG("Total length to allocate: %zu\n", total_length);

  // Allocate memory for the formatted response
  char *buffer = malloc(total_length);

  if (buffer == NULL) {
    perror("Failed to allocate memory");
    // Free allocated components
    free((void *)html_container);
    free((void *)html_menu);
    // free((void *)html_slider);
    free((void *)html_home_blog);
    return NULL;
  }

  // Format the string by replacing %s in html_container with other
  // components
  snprintf(buffer, total_length, html_container, html_menu, "", html_home_blog);

  // Free the component strings as they are no longer needed
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_home_blog);

  // Calculate total size needed for HTTP headers + body
  size_t header_len = snprintf(NULL, 0,
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %zu\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               strlen(buffer)) +
                      1; // +1 for null terminator

  // Calculate total size for the entire HTTP response
  size_t total_html_size = header_len + strlen(buffer) + 1;

  // Dynamically allocate the response buffer instead of using a static one
  char *root_response = malloc(total_html_size);

  if (root_response == NULL) {
    perror("Failed to allocate memory for HTTP response");
    free(buffer); // Don't forget to free the previous buffer
    return NULL;
  }

  // Build the HTTP response with headers and body
  snprintf(root_response, total_html_size,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n%s",
           strlen(buffer), buffer);

  // Free the dynamically allocated memory for buffer
  free(buffer);

  return root_response;
}

const char *buildBlogEntryWebSite(const char *id) {
  // Get the HTML content from the modules
  const char *html_container = container();
  const char *html_menu = menu();
  const char *html_blog_entry = blog_entry(id);

  // Check if any component returned NULL
  if (!html_container || !html_menu || !html_blog_entry) {
    printf("Error buildBlogEntryWebSite: One or more HTML components are NULL.\n");
    // Free allocated components if any
    if (html_container)
      free((void *)html_container);
    if (html_menu)
      free((void *)html_menu);
    if (html_blog_entry)
      free((void *)html_blog_entry);
    return NULL;
  }

  // Calculate the total size needed for the formatted string
  size_t total_length = snprintf(NULL, 0, html_container, html_menu, "",
                                 html_blog_entry) +
                        1; // +1 for null terminator

  LOG_DEBUG("Total length to allocate: %zu\n", total_length);

  // Allocate memory for the formatted response
  char *buffer = malloc(total_length);

  if (buffer == NULL) {
    perror("Failed to allocate memory");
    // Free allocated components
    free((void *)html_container);
    free((void *)html_menu);
    free((void *)html_blog_entry);
    return NULL;
  }

  // Format the string by replacing %s in html_container with other
  // components
  snprintf(buffer, total_length, html_container, html_menu, "",
           html_blog_entry);

  // Free the component strings as they are no longer needed
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_blog_entry);

  // Calculate total size needed for HTTP headers + body
  size_t header_len = snprintf(NULL, 0,
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %zu\r\n"
                               "Connection: close\r\n"
                               "\r\n",
                               strlen(buffer)) +
                      1; // +1 for null terminator

  // Calculate total size for the entire HTTP response
  size_t total_html_size = header_len + strlen(buffer) + 1;

  // Dynamically allocate the response buffer instead of using a static one
  char *root_response = malloc(total_html_size);

  if (root_response == NULL) {
    perror("Failed to allocate memory for HTTP response");
    free(buffer); // Don't forget to free the previous buffer
    return NULL;
  }

  // Build the HTTP response with headers and body
  snprintf(root_response, total_html_size,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "Connection: close\r\n"
           "\r\n%s",
           strlen(buffer), buffer);

  // Free the dynamically allocated memory for buffer
  free(buffer);

  return root_response;
}