#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/page_items.h"
#include "../../include/code_highlight/highlight_code.h"
#include "../../include/generate_url_theme.h"
#include "../../include/log.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

const char *page(const char *id, int epoch) {
  // Get templates
  char *filename_page_container_html =
      generate_url_theme("page/page-container_epoch%d.html", epoch);
  const char *page_container_html =
      read_file_to_string(filename_page_container_html);
  free(filename_page_container_html);

  char *filename_page_html =
      generate_url_theme("page/page-entry_epoch%d.html", epoch);
  const char *page_html = read_file_to_string(filename_page_html);
  free(filename_page_html);

  // Get elements templates
  char *filename_element_paragraph_html =
      generate_url_theme("elements/paragraph/paragraph_epoch%d.html", epoch);
  const char *element_paragraph_html =
      read_file_to_string(filename_element_paragraph_html);
  free(filename_element_paragraph_html);

  char *filename_element_tittle_html =
      generate_url_theme("elements/tittle/tittle_epoch%d.html", epoch);
  const char *element_tittle_html =
      read_file_to_string(filename_element_tittle_html);
  free(filename_element_tittle_html);

  char *filename_element_image_html =
      generate_url_theme("elements/image/image_epoch%d.html", epoch);
  const char *element_image_html =
      read_file_to_string(filename_element_image_html);
  free(filename_element_image_html);

  char *filename_element_image_gallery_html = generate_url_theme(
      "elements/image-gallery/image-gallery_epoch%d.html", epoch);
  const char *element_image_gallery_html =
      read_file_to_string(filename_element_image_gallery_html);
  free(filename_element_image_gallery_html);

  /////////////////////////
  char *filename_element_gallery_container_html = generate_url_theme(
      "elements/gallery/gallery-container_epoch%d.html", epoch);
  const char *element_gallery_container_html =
      read_file_to_string(filename_element_gallery_container_html);
  free(filename_element_gallery_container_html);

  char *filename_element_gallery_row_html =
      generate_url_theme("elements/gallery/gallery-row_epoch%d.html", epoch);
  const char *element_gallery_row_html =
      read_file_to_string(filename_element_gallery_row_html);
  free(filename_element_gallery_row_html);

  char *filename_element_gallery_item_html =
      generate_url_theme("elements/gallery/gallery-item_epoch%d.html", epoch);
  const char *element_gallery_item_html =
      read_file_to_string(filename_element_gallery_item_html);
  free(filename_element_gallery_item_html);

  /////////////////////////

    char *filename_element_list_container_html = generate_url_theme(
      "elements/list/list-container_epoch%d.html", epoch);
  const char *element_list_container_html =
      read_file_to_string(filename_element_list_container_html);
  free(filename_element_list_container_html);

  char *filename_element_list_item_html =
      generate_url_theme("elements/list/list-item_epoch%d.html", epoch);
  const char *element_list_item_html =
      read_file_to_string(filename_element_list_item_html);
  free(filename_element_list_item_html);

  char *filename_element_image_paragraph_html = generate_url_theme(
      "elements/image-paragraph/image-paragraph_epoch%d.html", epoch);
  const char *element_image_paragraph_html =
      read_file_to_string(filename_element_image_paragraph_html);
  free(filename_element_image_paragraph_html);

  char *filename_element_date_time_html =
      generate_url_theme("elements/date-time/date-time_epoch%d.html", epoch);
  const char *element_date_time_html =
      read_file_to_string(filename_element_date_time_html);
  free(filename_element_date_time_html);

  char *filename_element_link_html =
      generate_url_theme("elements/link/link_epoch%d.html", epoch);
  const char *element_link_html =
      read_file_to_string(filename_element_link_html);
  free(filename_element_link_html);

  char *filename_element_byline_html =
      generate_url_theme("elements/byline/byline_epoch%d.html", epoch);
  const char *element_byline_html =
      read_file_to_string(filename_element_byline_html);
  free(filename_element_byline_html);

  char *filename_element_code_text_html =
      generate_url_theme("elements/code-text/code-text_epoch%d.html", epoch);
  const char *element_code_text_html =
      read_file_to_string(filename_element_code_text_html);
  free(filename_element_code_text_html);

  char *filename_element_form_start_html =
      generate_url_theme("elements/form/form-start_epoch%d.html", epoch);
  const char *element_form_start_html =
      read_file_to_string(filename_element_form_start_html);
  free(filename_element_form_start_html);

  char *filename_element_form_end_html =
      generate_url_theme("elements/form/form-end_epoch%d.html", epoch);
  const char *element_form_end_html =
      read_file_to_string(filename_element_form_end_html);
  free(filename_element_form_end_html);

  if (!page_container_html || !page_html || !element_paragraph_html ||
      !element_tittle_html || !element_image_html ||
      !element_image_gallery_html || !element_image_paragraph_html ||
      !element_date_time_html || !element_link_html || !element_byline_html ||
      !element_gallery_container_html || !element_gallery_row_html ||
      !element_gallery_item_html || !element_code_text_html || 
      !element_list_container_html || !element_list_item_html || 
      !element_form_start_html || !element_form_end_html) {
    perror("Failed to load HTML templates");
    // Free allocated templates if any
    if (page_container_html)
      free((void *)page_container_html);
    if (page_html)
      free((void *)page_html);
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
    if (element_byline_html)
      free((void *)element_byline_html);
    if (element_image_gallery_html)
      free((void *)element_image_gallery_html);
    if (element_gallery_container_html)
      free((void *)element_gallery_container_html);
    if (element_gallery_row_html)
      free((void *)element_gallery_row_html);
    if (element_gallery_item_html)
      free((void *)element_gallery_item_html);
    if (element_code_text_html)
      free((void *)element_code_text_html);
    if (element_list_container_html)
      free((void *)element_list_container_html);
    if (element_list_item_html)
      free((void *)element_list_item_html);
    if (element_form_start_html)
      free((void *)element_form_start_html);
    if (element_form_end_html)
      free((void *)element_form_end_html);
    return NULL;
  }

  // Get page list
  int PageItemsCount = 0;
  PageItems *home_page_items = getPageItems(id, &PageItemsCount);
  if (home_page_items == NULL) {
    perror("Failed to load page items");
    free((void *)page_container_html);
    free((void *)page_html);
    free((void *)element_paragraph_html);
    free((void *)element_tittle_html);
    free((void *)element_image_html);
    free((void *)element_image_paragraph_html);
    free((void *)element_date_time_html);
    free((void *)element_link_html);
    free((void *)element_byline_html);
    free((void *)element_image_gallery_html);
    free((void *)element_gallery_container_html);
    free((void *)element_gallery_row_html);
    free((void *)element_gallery_item_html);
    free((void *)element_code_text_html);
    free((void *)element_list_container_html);
    free((void *)element_list_item_html);
    free((void *)element_form_start_html);
    free((void *)element_form_end_html);
    return NULL;
  }

  // Buffer to hold all items
  char *itemsBuffer = NULL;
  size_t itemsBufferSize = 0;

  // Generate the page items
  for (int i = 0; i < PageItemsCount; i++) {
    char itemBuffer[20480]; // Buffer to hold a single item
    int itemLength = 0;
    if (strcmp(home_page_items[i].type, "paragraph") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_paragraph_html,
                   home_page_items[i].extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "tittle") == 0) {
      const char *extra_data = home_page_items[i].extra_data;
      if (extra_data == NULL || strcmp(extra_data, "") == 0) {
          extra_data = "1"; // Set 1 in case null or empty, get h1 result on template
      }

      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_tittle_html,
                  extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "image") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_image_html,
                   home_page_items[i].content, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "image-gallery") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_image_gallery_html,
                   home_page_items[i].content, home_page_items[i].content,
                   home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "image-paragraph") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_image_paragraph_html,
                   home_page_items[i].content, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "date-time") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_date_time_html,
                   home_page_items[i].extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "link") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_link_html,
                   home_page_items[i].extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "byline") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_byline_html,
                   home_page_items[i].content, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "code-text") == 0) {
      // Resaltar sintaxis
      char *codigo_resaltado = highlight_code(home_page_items[i].content);

      // Formatear para HTML
      char *html_final = malloc(strlen(codigo_resaltado) + 20);
      sprintf(html_final, "%s", codigo_resaltado);

      // Imprimir resultado
      LOG_DEBUG("%s\n\n", home_page_items[i].content);
      LOG_DEBUG("%s\n\n", html_final);

      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_code_text_html,
                   html_final, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "gallery") == 0) {
      char *image;
      int columns;
      int image_width;
      int table_width;

      int counter = 0;
      int image_count = 0;
      char *images;

      // Allocate memory for images
      images = malloc(strlen(home_page_items[i].content) +
                      1); // Allocate enough memory
      if (images == NULL) {
        LOG_ERROR("Failed to allocate memory");
      }

      // Copy the content of home_page_items[i].content
      strcpy(images, home_page_items[i].content);

      // Count the number of images (separated by ;)
      image = strtok(images, ";");
      while (image != NULL) {
        image_count++;
        image = strtok(NULL, ";");
      }

      // Determine the number of columns based on the number of images
      if (image_count == 1) {
        columns = 1;
        image_width = 500;
        table_width = 100;
      } else if (image_count == 2) {
        columns = 2;
        image_width = 200;
        table_width = 50;
      } else if (image_count == 3) {
        columns = 3;
        image_width = 150;
        table_width = 33;
      } else if (image_count == 4) {
        columns = 2;
        image_width = 200;
        table_width = 50;
      } else {
        columns = 3;
        image_width = 150;
        table_width = 33;
      }

      LOG_DEBUG("Number of columns: %i", columns);
      LOG_DEBUG("Image width: %i px", image_width);
      LOG_DEBUG("Table width: %i", table_width);

      // Buffers to store the results
      // Final buffer that will contain the complete table
      char tableBuffer[4096] = "";
      char rowBuffer[1024];              // Temporary buffer for rows
      char cellBuffer[512];              // Temporary buffer for each cell
      char rowsContentBuffer[2048] = ""; // Stores all the rows

      // Reset the images to traverse them again
      strcpy(images, home_page_items[i].content);

      // Use strtok to split the string by the ';' delimiter
      image = strtok(images, ";");

      // Traverse each image
      while (image != NULL) {
        // If counter is 0 or a multiple of columns, prepare a new row
        if (counter % columns == 0) {
          // Clear the row buffer for each new row
          rowBuffer[0] = '\0';
        }

        // Prepare the cell content using the template for each image
        snprintf(cellBuffer, sizeof(cellBuffer), element_gallery_item_html,
                 table_width, image, image, image_width);

        // Concatenate the cell content to the row
        strcat(rowBuffer, cellBuffer);

        counter++;

        // If the counter is a multiple of columns, close the row
        if (counter % columns == 0) {
          // Add the row to the general content of rows
          char completeRow[1024];
          snprintf(completeRow, sizeof(completeRow), element_gallery_row_html,
                   rowBuffer);
          strcat(rowsContentBuffer, completeRow);
        }

        // Continue with the next token (image)
        image = strtok(NULL, ";");
      }

      // If the last row is not complete, add it anyway
      if (counter % columns != 0) {
        char completeRow[1024];
        snprintf(completeRow, sizeof(completeRow), element_gallery_row_html,
                 rowBuffer);
        strcat(rowsContentBuffer, completeRow);
      }

      // Print the final result
      LOG_DEBUG("%s\n", tableBuffer);

      itemLength = snprintf(itemBuffer, sizeof(itemBuffer),
                            element_gallery_container_html, rowsContentBuffer);
    } else if (strcmp(home_page_items[i].type, "list") == 0) {
      char *item;
      int counter = 0;
      char *items;

      // Allocate memory for items
      items = malloc(strlen(home_page_items[i].content) + 1); // Allocate enough memory
      if (items == NULL) {
          LOG_ERROR("Failed to allocate memory for list items");
          continue; // Skip this item and move to the next
      }

      // Copy the content of home_page_items[i].content
      strcpy(items, home_page_items[i].content);

      // Buffers to store the results
      char listBuffer[4096] = "";              // Final buffer for the entire list
      char listItemBuffer[512];                // Temporary buffer for each list item

      // Use strtok to split the string by the ';' delimiter
      item = strtok(items, ";");

      // Traverse each item in the list
      while (item != NULL) {
          // Prepare the list item content using the template
          snprintf(listItemBuffer, sizeof(listItemBuffer), element_list_item_html, item);

          // Concatenate the list item to the final list buffer
          strcat(listBuffer, listItemBuffer);

          counter++;

          // Get the next token (item)
          item = strtok(NULL, ";");
      }

      // Free the allocated items buffer
      free(items);

      // Format the full list using the list container template
      itemLength = snprintf(itemBuffer, sizeof(itemBuffer), element_list_container_html, listBuffer);
    } else if (strcmp(home_page_items[i].type, "form-start") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_form_start_html,
                   home_page_items[i].content, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "form-end") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_form_end_html,
                   home_page_items[i].content, home_page_items[i].extra_data);
    }

    if (itemLength < 0) {
      perror("Error formatting item");
      free(itemsBuffer);
      free(home_page_items);
      free((void *)page_container_html);
      free((void *)page_html);
      free((void *)element_paragraph_html);
      free((void *)element_tittle_html);
      free((void *)element_image_html);
      free((void *)element_image_paragraph_html);
      free((void *)element_date_time_html);
      free((void *)element_link_html);
      free((void *)element_byline_html);
      free((void *)element_image_gallery_html);
      free((void *)element_gallery_container_html);
      free((void *)element_gallery_row_html);
      free((void *)element_gallery_item_html);
      free((void *)element_code_text_html);
      free((void *)element_list_container_html);
      free((void *)element_list_item_html);
      free((void *)element_form_start_html);
      free((void *)element_form_end_html);
      return NULL;
    }

    // Reallocate memory for itemsBuffer
    char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength +
                                                1); // +1 for null terminator
    if (tempBuffer == NULL) {
      perror("Failed to allocate memory for itemsBuffer");
      free(itemsBuffer);
      free(home_page_items);
      free((void *)page_container_html);
      free((void *)page_html);
      free((void *)element_paragraph_html);
      free((void *)element_tittle_html);
      free((void *)element_image_html);
      free((void *)element_image_paragraph_html);
      free((void *)element_date_time_html);
      free((void *)element_link_html);
      free((void *)element_byline_html);
      free((void *)element_image_gallery_html);
      free((void *)element_gallery_container_html);
      free((void *)element_gallery_row_html);
      free((void *)element_gallery_item_html);
      free((void *)element_code_text_html);
      free((void *)element_list_container_html);
      free((void *)element_list_item_html);
      free((void *)element_form_start_html);
      free((void *)element_form_end_html);
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
      snprintf(NULL, 0, page_container_html, itemsBuffer) +
      1; // +1 for null terminator
  char *contentBuffer = malloc(contentBufferSize);
  if (contentBuffer == NULL) {
    perror("Failed to allocate memory for contentBuffer");
    free(itemsBuffer);
    free(home_page_items);
    free((void *)page_container_html);
    free((void *)page_html);
    free((void *)element_paragraph_html);
    free((void *)element_tittle_html);
    free((void *)element_image_html);
    free((void *)element_image_paragraph_html);
    free((void *)element_date_time_html);
    free((void *)element_link_html);
    free((void *)element_byline_html);
    free((void *)element_image_gallery_html);
    free((void *)element_gallery_container_html);
    free((void *)element_gallery_row_html);
    free((void *)element_gallery_item_html);
    free((void *)element_code_text_html);
    free((void *)element_list_container_html);
    free((void *)element_list_item_html);
    free((void *)element_form_start_html);
    free((void *)element_form_end_html);
    return NULL;
  }

  // Format the final HTML content
  snprintf(contentBuffer, contentBufferSize, page_container_html, itemsBuffer);

  // Free the itemsBuffer as it's no longer needed
  free(itemsBuffer);

  // Free the page items
  free(home_page_items);

  // Free the templates
  free((void *)page_container_html);
  free((void *)page_html);

  return contentBuffer;
}
