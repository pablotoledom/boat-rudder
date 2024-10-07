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

  // Get elements templates
  const char *element_paragraph_html =
      read_file_to_string("./html/elements/paragraph/paragraph_std2.html");

  const char *element_tittle_html =
      read_file_to_string("./html/elements/tittle/tittle_std2.html");

  const char *element_image_html =
      read_file_to_string("./html/elements/image/image_std2.html");

  const char *element_image_paragraph_html = read_file_to_string(
      "./html/elements/image-paragraph/image-paragraph_std2.html");
  const char *element_date_time_html =
      read_file_to_string("./html/elements/date-time/date-time_std2.html");
  const char *element_link_html =
      read_file_to_string("./html/elements/link/link_std2.html");

  if (!blog_container_html || !blog_entry_html || !element_paragraph_html ||
      !element_tittle_html || !element_image_html ||
      !element_image_paragraph_html || !element_date_time_html || !element_link_html) {
    perror("Failed to load HTML templates");
    // Free allocated templates if any
    if (blog_container_html)
      free((void *)blog_container_html);
    if (blog_entry_html)
      free((void *)blog_entry_html);
    if (element_paragraph_html)
      free((void *)element_paragraph_html);
    if (element_tittle_html)
      free((void *)element_tittle_html);
    if (element_image_html)
      free((void *)element_image_html);
    if (element_image_paragraph_html)
      free((void *)element_image_paragraph_html);
    if (element_date_time_html)
      free((void *)element_date_time_html);
    if (element_link_html)
      free((void *)element_link_html);
    return NULL;
  }

  // Get blog list
  int BlogEntryItemsCount = 0;
  BlogEntryItems *home_blog_items = getBlogEntryItems(id, &BlogEntryItemsCount);
  if (home_blog_items == NULL) {
    perror("Failed to load blog items");
    free((void *)blog_container_html);
    free((void *)blog_entry_html);
    free((void *)element_paragraph_html);
    free((void *)element_tittle_html);
    free((void *)element_image_html);
    free((void *)element_image_paragraph_html);
    free((void *)element_date_time_html);
    free((void *)element_link_html);
    return NULL;
  }

  // Buffer to hold all items
  char *itemsBuffer = NULL;
  size_t itemsBufferSize = 0;

  // Generate the blog items
  for (int i = 0; i < BlogEntryItemsCount; i++) {
    char itemBuffer[20480]; // Buffer to hold a single item
    int itemLength = 0;
    if (strcmp(home_blog_items[i].type, "paragraph") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_paragraph_html,
                   home_blog_items[i].extra_data, home_blog_items[i].content);
    } else if (strcmp(home_blog_items[i].type, "tittle") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_tittle_html,
                   home_blog_items[i].extra_data, home_blog_items[i].content);
    } else if (strcmp(home_blog_items[i].type, "image") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_image_html,
                   home_blog_items[i].content, home_blog_items[i].extra_data);
    } else if (strcmp(home_blog_items[i].type, "image-paragraph") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_image_paragraph_html,
                   home_blog_items[i].content, home_blog_items[i].extra_data);
    } else if (strcmp(home_blog_items[i].type, "date-time") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_date_time_html,
                   home_blog_items[i].extra_data, home_blog_items[i].content);
    }else if (strcmp(home_blog_items[i].type, "link") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_link_html,
                   home_blog_items[i].extra_data, home_blog_items[i].content);
    }

    if (itemLength < 0) {
      perror("Error formatting item");
      free(itemsBuffer);
      free(home_blog_items);
      free((void *)blog_container_html);
      free((void *)blog_entry_html);
      free((void *)element_paragraph_html);
      free((void *)element_tittle_html);
      free((void *)element_image_html);
      free((void *)element_image_paragraph_html);
      free((void *)element_date_time_html);
      free((void *)element_link_html);
      return NULL;
    }

    // Reallocate memory for itemsBuffer
    char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength +
                                                1); // +1 for null terminator
    if (tempBuffer == NULL) {
      perror("Failed to allocate memory for itemsBuffer");
      free(itemsBuffer);
      free(home_blog_items);
      free((void *)blog_container_html);
      free((void *)blog_entry_html);
      free((void *)element_paragraph_html);
      free((void *)element_tittle_html);
      free((void *)element_image_html);
      free((void *)element_image_paragraph_html);
      free((void *)element_date_time_html);
      free((void *)element_link_html);
      return NULL;
    }

    itemsBuffer = tempBuffer;

    // Copy the new item into itemsBuffer
    memcpy(itemsBuffer + itemsBufferSize, itemBuffer, itemLength);
    itemsBufferSize += itemLength;
    itemsBuffer[itemsBufferSize] = '\0'; // Null-terminate the string
  }

  // Allocate buffer for the full HTML content
  size_t contentBufferSize =
      snprintf(NULL, 0, blog_container_html, itemsBuffer) +
      1; // +1 for null terminator
  char *contentBuffer = malloc(contentBufferSize);
  if (contentBuffer == NULL) {
    perror("Failed to allocate memory for contentBuffer");
    free(itemsBuffer);
    free(home_blog_items);
    free((void *)blog_container_html);
    free((void *)blog_entry_html);
    free((void *)element_paragraph_html);
    free((void *)element_tittle_html);
    free((void *)element_image_html);
    free((void *)element_image_paragraph_html);
    free((void *)element_date_time_html);
    free((void *)element_link_html);
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
