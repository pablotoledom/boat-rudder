#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "modules/home_blog/home_blog.h"
#include "modules/container/container.h"
#include "modules/menu/menu.h"
#include "modules/slider/slider.h"
#include <stdio.h>  // For snprintf
#include <stdlib.h> // For malloc and free
#include <string.h> // For strlen

const char *buildWebSite() {
    // Get the HTML content from the modules
    const char *response_container = container();
    const char *response_menu = menu();
    const char *response_slider = slider();
    const char *response_home_blog = home_blog();

    // Check if any component returned NULL
    if (!response_container || !response_menu || !response_slider || !response_home_blog) {
        printf("Error: One or more HTML components are NULL.\n");
        // Free allocated components if any
        if (response_container) free((void *)response_container);
        if (response_menu) free((void *)response_menu);
        if (response_slider) free((void *)response_slider);
        if (response_home_blog) free((void *)response_home_blog);
        return NULL;
    }

    // Calculate the total size needed for the formatted string
    size_t total_length = snprintf(NULL, 0, response_container, response_menu,
                                   response_slider, response_home_blog) +
                          1; // +1 for null terminator

    printf("Total length to allocate: %zu\n", total_length);  // Debug output

    // Allocate memory for the formatted response
    char *buffer = malloc(total_length);

    if (buffer == NULL) {
        perror("Failed to allocate memory");
        // Free allocated components
        free((void *)response_container);
        free((void *)response_menu);
        free((void *)response_slider);
        free((void *)response_home_blog);
        return NULL;
    }

    // Format the string by replacing %s in response_container with other components
    snprintf(buffer, total_length, response_container, response_menu,
             response_slider, response_home_blog);

    // Free the component strings as they are no longer needed
    free((void *)response_container);
    free((void *)response_menu);
    free((void *)response_slider);
    free((void *)response_home_blog);

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
    size_t total_response_size = header_len + strlen(buffer) + 1;

    // Dynamically allocate the response buffer instead of using a static one
    char *root_response = malloc(total_response_size);

    if (root_response == NULL) {
        perror("Failed to allocate memory for HTTP response");
        free(buffer);  // Don't forget to free the previous buffer
        return NULL;
    }

    // Build the HTTP response with headers and body
    snprintf(root_response, total_response_size,
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
