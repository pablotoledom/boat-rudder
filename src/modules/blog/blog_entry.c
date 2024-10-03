#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/blog_entry_items.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *blog_entry(const char *id) {
    // Get templates
    const char *blog_container_html =
        read_file_to_string("./html/blog/blog-container_std2.html");

    const char *blog_entry_html =
        read_file_to_string("./html/blog/blog-entry_std2.html");

    if (!blog_container_html || !blog_entry_html) {
        perror("Failed to load HTML templates");
        // Free allocated templates if any
        if (blog_container_html) free((void *)blog_container_html);
        if (blog_entry_html) free((void *)blog_entry_html);
        return NULL;
    }

    // Get blog list
    int BlogEntryItemsCount = 0;
    BlogEntryItems *home_blog_items = getBlogEntryItems(id, &BlogEntryItemsCount);
    if (home_blog_items == NULL) {
        perror("Failed to load blog items");
        free((void *)blog_container_html);
        free((void *)blog_entry_html);
        return NULL;
    }

    // Buffer to hold all items
    char *itemsBuffer = NULL;
    size_t itemsBufferSize = 0;

    // Generate the blog items
    for (int i = 0; i < BlogEntryItemsCount; i++) {
        char itemBuffer[1024]; // Buffer to hold a single item
        int itemLength = snprintf(itemBuffer, sizeof(itemBuffer), blog_entry_html, home_blog_items[i].entry_id,
                                  home_blog_items[i].content_id, home_blog_items[i].type, home_blog_items[i].content,
                                  home_blog_items[i].extra_data);

        if (itemLength < 0) {
            perror("Error formatting item");
            free(itemsBuffer);
            free(home_blog_items);
            free((void *)blog_container_html);
            free((void *)blog_entry_html);
            return NULL;
        }

        // Reallocate memory for itemsBuffer
        char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength + 1); // +1 for null terminator
        if (tempBuffer == NULL) {
            perror("Failed to allocate memory for itemsBuffer");
            free(itemsBuffer);
            free(home_blog_items);
            free((void *)blog_container_html);
            free((void *)blog_entry_html);
            return NULL;
        }

        itemsBuffer = tempBuffer;

        // Copy the new item into itemsBuffer
        memcpy(itemsBuffer + itemsBufferSize, itemBuffer, itemLength);
        itemsBufferSize += itemLength;
        itemsBuffer[itemsBufferSize] = '\0'; // Null-terminate the string
    }

    // Allocate buffer for the full HTML content
    size_t contentBufferSize = snprintf(NULL, 0, blog_container_html, itemsBuffer) + 1; // +1 for null terminator
    char *contentBuffer = malloc(contentBufferSize);
    if (contentBuffer == NULL) {
        perror("Failed to allocate memory for contentBuffer");
        free(itemsBuffer);
        free(home_blog_items);
        free((void *)blog_container_html);
        free((void *)blog_entry_html);
        return NULL;
    }

    // Format the final HTML content
    snprintf(contentBuffer, contentBufferSize, blog_container_html, itemsBuffer);

    // Free the itemsBuffer as it's no longer needed
    free(itemsBuffer);

    // Free the blog items
    free(home_blog_items);

    // Free the templates
    free((void *)blog_container_html);
    free((void *)blog_entry_html);

    return contentBuffer;
}
