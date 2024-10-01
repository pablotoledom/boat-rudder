#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/home_blog_items.h"
#include "../../include/config_loader.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *home_blog() {
    // Get templates
    const char *home_blog_response =
        read_file_to_string("./html/home-blog/home-blog_std2.html");

    const char *home_blog_item_response =
        read_file_to_string("./html/home-blog/home-blog-item_std2.html");

    if (!home_blog_response || !home_blog_item_response) {
        perror("Failed to load HTML templates");
        // Free allocated templates if any
        if (home_blog_response) free((void *)home_blog_response);
        if (home_blog_item_response) free((void *)home_blog_item_response);
        return NULL;
    }

    // Get blog list
    int routeCount = 0;
    HomeBlogItems *home_blog_items = getBlogItems(&routeCount);
    if (home_blog_items == NULL) {
        perror("Failed to load blog items");
        free((void *)home_blog_response);
        free((void *)home_blog_item_response);
        return NULL;
    }

    // Buffer to hold all items
    char *itemsBuffer = NULL;
    size_t itemsBufferSize = 0;

    // Generate the blog items
    for (int i = 0; i < routeCount; i++) {
        char itemBuffer[1024]; // Buffer to hold a single item
        int itemLength = snprintf(itemBuffer, sizeof(itemBuffer), home_blog_item_response,
                                  home_blog_items[i].image_url, home_blog_items[i].tittle,
                                  home_blog_items[i].summary);

        if (itemLength < 0) {
            perror("Error formatting item");
            free(itemsBuffer);
            free(home_blog_items);
            free((void *)home_blog_response);
            free((void *)home_blog_item_response);
            return NULL;
        }

        // Reallocate memory for itemsBuffer
        char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength + 1); // +1 for null terminator
        if (tempBuffer == NULL) {
            perror("Failed to allocate memory for itemsBuffer");
            free(itemsBuffer);
            free(home_blog_items);
            free((void *)home_blog_response);
            free((void *)home_blog_item_response);
            return NULL;
        }

        itemsBuffer = tempBuffer;

        // Copy the new item into itemsBuffer
        memcpy(itemsBuffer + itemsBufferSize, itemBuffer, itemLength);
        itemsBufferSize += itemLength;
        itemsBuffer[itemsBufferSize] = '\0'; // Null-terminate the string
    }

    // Allocate buffer for the full HTML content
    size_t contentBufferSize = snprintf(NULL, 0, home_blog_response, itemsBuffer) + 1; // +1 for null terminator
    char *contentBuffer = malloc(contentBufferSize);
    if (contentBuffer == NULL) {
        perror("Failed to allocate memory for contentBuffer");
        free(itemsBuffer);
        free(home_blog_items);
        free((void *)home_blog_response);
        free((void *)home_blog_item_response);
        return NULL;
    }

    // Format the final HTML content
    snprintf(contentBuffer, contentBufferSize, home_blog_response, itemsBuffer);

    // Free the itemsBuffer as it's no longer needed
    free(itemsBuffer);

    // Free the blog items
    free(home_blog_items);

    // Free the templates
    free((void *)home_blog_response);
    free((void *)home_blog_item_response);

    return contentBuffer;
}
