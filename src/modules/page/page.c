#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../api/page_items.h"
#include "../../include/code_highlight/highlight_code.h"
#include "../../include/generate_url_theme.h"
#include "../../include/log.h"
#include "../../include/read_file.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>  

/**
 * dynamic_strcat:
 *  - *dest is the address of your dynamic buffer (where you concatenate).
 *  - *cap is the current capacity (in bytes) of that buffer.
 *  - src is the string you want to concatenate.
 *
 * This function expands (realloc) *dest if there is not enough space.
 */
static void dynamic_strcat(char **dest, size_t *cap, const char *src) {
    size_t destLen = strlen(*dest);
    size_t srcLen  = strlen(src);
    size_t needed  = destLen + srcLen + 1; // +1 for the '\0'

    // If it doesn't fit, we reallocate with some margin (e.g., doubling)
    if (needed > *cap) {
        size_t newCap = (needed < (*cap * 2)) ? (*cap * 2) : (needed * 2);
        char *temp = realloc(*dest, newCap);
        if (!temp) {
            LOG_ERROR("Failed to reallocate memory (requested %zu bytes).", newCap);
            // Error handling: you could abort, or return, etc.
            return;
        }
        *dest = temp;
        *cap  = newCap;

        LOG_DEBUG("Buffer reallocated to %zu bytes", newCap);
    }

    // Finally, concatenate
    strcat(*dest, src);
}

static char* dynamic_sprintf(const char *fmt, ...) {
    va_list args;
    va_list args_copy;
    va_start(args, fmt);
    va_copy(args_copy, args);

    // 1. Calculate required space
    int needed = vsnprintf(NULL, 0, fmt, args);
    va_end(args);

    if (needed < 0) {
        LOG_ERROR("vsnprintf error while calculating size.");
        va_end(args_copy);
        return NULL;
    }

    // 2. Allocate the necessary memory (+1 for '\0')
    char *buf = malloc(needed + 1);
    if (!buf) {
        LOG_ERROR("Failed to allocate %d bytes for dynamic_sprintf.", needed + 1);
        va_end(args_copy);
        return NULL;
    }

    // 3. Perform the actual print
    vsnprintf(buf, needed + 1, fmt, args_copy);
    va_end(args_copy);

    return buf; // Returns the dynamically generated string
}

const char *page(const char *id, int epoch) {
  // Get templates
  char *filename_page_container_html =
      generate_url_theme("page/page-container_epoch%d.html", epoch);
  const char *page_container_html =
      read_file_to_string(filename_page_container_html);
  free(filename_page_container_html);

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

  char *filename_element_button_primary_html =
      generate_url_theme("elements/button/button-primary_epoch%d.html", epoch);
  const char *element_button_primary_html =
      read_file_to_string(filename_element_button_primary_html);
  free(filename_element_button_primary_html);

  char *filename_element_button_secondary_html =
      generate_url_theme("elements/button/button-secondary_epoch%d.html", epoch);
  const char *element_button_secondary_html =
      read_file_to_string(filename_element_button_secondary_html);
  free(filename_element_button_secondary_html);

  char *filename_element_input_text_html =
      generate_url_theme("elements/input/input-text_epoch%d.html", epoch);
  const char *element_input_text_html =
      read_file_to_string(filename_element_input_text_html);
  free(filename_element_input_text_html);

  char *filename_element_radio_button_html =
      generate_url_theme("elements/radio-button/radio-button_epoch%d.html", epoch);
  const char *element_radio_button_html =
      read_file_to_string(filename_element_radio_button_html);
  free(filename_element_radio_button_html);

  char *filename_element_checkbox_html =
      generate_url_theme("elements/checkbox/checkbox_epoch%d.html", epoch);
  const char *element_checkbox_html =
      read_file_to_string(filename_element_checkbox_html);
  free(filename_element_checkbox_html);

  char *filename_element_youtube_embed_html =
      generate_url_theme("elements/youtube-embed/video_epoch%d.html", epoch);
  const char *element_youtube_embed_html =
      read_file_to_string(filename_element_youtube_embed_html);
  free(filename_element_youtube_embed_html);

  char *filename_element_separator_html =
      generate_url_theme("elements/separator/separator_epoch%d.html", epoch);
  const char *element_separator_html =
      read_file_to_string(filename_element_separator_html);
  free(filename_element_separator_html);

  if (!page_container_html || !element_paragraph_html ||
      !element_tittle_html || !element_image_html ||
      !element_image_gallery_html || !element_image_paragraph_html ||
      !element_date_time_html || !element_link_html || !element_byline_html ||
      !element_gallery_container_html || !element_gallery_row_html ||
      !element_gallery_item_html || !element_code_text_html ||
      !element_list_container_html || !element_list_item_html ||
      !element_form_start_html || !element_form_end_html ||
      !element_button_primary_html  || !element_button_secondary_html ||
      !element_input_text_html  || !element_radio_button_html ||
      !element_checkbox_html || !element_youtube_embed_html ||
			!element_separator_html) {
    perror("Failed to load HTML templates");
    // Free allocated templates if any
    if (page_container_html)
      free((void *)page_container_html);
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
    if (element_button_primary_html)
      free((void *)element_button_primary_html);
    if (element_button_secondary_html)
      free((void *)element_button_secondary_html);
    if (element_input_text_html)
      free((void *)element_input_text_html);
    if (element_radio_button_html)
      free((void *)element_radio_button_html);
    if (element_checkbox_html)
      free((void *)element_checkbox_html); 
    if (element_youtube_embed_html)
      free((void *)element_youtube_embed_html);
		if (element_separator_html)
      free((void *)element_separator_html);
    return NULL;
  }

  // Get page list
  int PageItemsCount = 0;
  PageItems *home_page_items = getPageItems(id, &PageItemsCount);
  if (home_page_items == NULL) {
    perror("Failed to load page items");
    free((void *)page_container_html);
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
    free((void *)element_button_primary_html);
    free((void *)element_button_secondary_html);
    free((void *)element_input_text_html);
    free((void *)element_radio_button_html);
    free((void *)element_checkbox_html);
    free((void *)element_youtube_embed_html);
    free((void *)element_separator_html);
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
      // Syntax highlighting
      char *codigo_resaltado = highlight_code(home_page_items[i].content);

      // Format for HTML
      char *html_final = malloc(strlen(codigo_resaltado) + 20);
      sprintf(html_final, "%s", codigo_resaltado);

      // Print result
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

      // 1) Copy the content for tokenizing
      images = malloc(strlen(home_page_items[i].content) + 1);
      if (images == NULL) {
          LOG_ERROR("Failed to allocate memory for images");
          // Error handling: skip or abort
          continue;
      }
      strcpy(images, home_page_items[i].content);

      // 2) Count how many images are there
      image = strtok(images, ";");
      while (image != NULL) {
          image_count++;
          image = strtok(NULL, ";");
      }

      // Determine columns
      if (image_count == 1) {
          columns = 1; image_width = 500; table_width = 100;
      } else if (image_count == 2) {
          columns = 2; image_width = 200; table_width = 50;
      } else if (image_count == 3) {
          columns = 3; image_width = 150; table_width = 33;
      } else if (image_count == 4) {
          columns = 2; image_width = 200; table_width = 50;
      } else {
          columns = 3; image_width = 150; table_width = 33;
      }

      LOG_DEBUG("Number of columns: %d", columns);
      LOG_DEBUG("Image width: %d px", image_width);
      LOG_DEBUG("Table width: %d", table_width);

      // 3) Put 'images' back to its original content for iteration
      strcpy(images, home_page_items[i].content);

      // 4) Initialize dynamic buffers for the final content
      //    (current row and all rows)
      size_t rowBufferCap = 1;
      size_t rowsContentCap = 1;
      char *rowBuffer = calloc(rowBufferCap, 1);         // starts as ""
      char *rowsContentBuffer = calloc(rowsContentCap, 1);// starts as ""

      if (!rowBuffer || !rowsContentBuffer) {
          LOG_ERROR("Failed to allocate memory for rowBuffer/rowsContentBuffer.");
          free(images);
          if (rowBuffer) free(rowBuffer);
          if (rowsContentBuffer) free(rowsContentBuffer);
          continue;
      }

      image = strtok(images, ";");
      counter = 0;

      // 5) Traverse each image and build cells + rows
      while (image != NULL) {
          // If "counter" is 0 or a multiple of "columns", we reset the current row
          if (counter % columns == 0) {
              // Make rowBuffer an empty string:
              rowBuffer[0] = '\0';
          }

          // Create the cell content with dynamic_sprintf (or sprintf and a temp buffer)
          char *cellBuffer = dynamic_sprintf(element_gallery_item_html,
                                             table_width, image, image, image_width);
          if (!cellBuffer) {
              LOG_ERROR("Failed to build cellBuffer.");
              break; // Error handling
          }

          // Concatenate that cell into the row
          dynamic_strcat(&rowBuffer, &rowBufferCap, cellBuffer);

          free(cellBuffer); // No longer needed in memory
          counter++;

          // If a row is completed (multiple of columns), we close that row
          if (counter % columns == 0) {
              // Wrap the row with <tr>...</tr>
              char *completeRow = dynamic_sprintf(element_gallery_row_html, rowBuffer);
              if (!completeRow) {
                  LOG_ERROR("Failed to build completeRow.");
                  break;
              }

              // Concatenate it into the rows accumulator
              dynamic_strcat(&rowsContentBuffer, &rowsContentCap, completeRow);
              free(completeRow);
          }

          // Next image
          image = strtok(NULL, ";");
      }

      // 6) If the last row is not complete, add it anyway
      if (counter % columns != 0) {
          // Wrap the row with <tr>...</tr>
          char *completeRow = dynamic_sprintf(element_gallery_row_html, rowBuffer);
          if (completeRow) {
              dynamic_strcat(&rowsContentBuffer, &rowsContentCap, completeRow);
              free(completeRow);
          }
      }

      // 7) We have all rows in rowsContentBuffer,
      //    now we put them inside the container <table>...</table>.
      //    Finally, we place everything into 'itemBuffer' or wherever you store output.
      char *finalTable = dynamic_sprintf(element_gallery_container_html, rowsContentBuffer);
      if (!finalTable) {
          LOG_ERROR("Failed to build finalTable.");
          // Error handling
      } else {
          // Here we format itemBuffer (or whichever buffer you use)
          itemLength = snprintf(itemBuffer, sizeof(itemBuffer), "%s", finalTable);
          LOG_DEBUG("Final gallery:\n%s\n", finalTable);
          free(finalTable);
      }

      // Free everything
      free(images);
      free(rowBuffer);
      free(rowsContentBuffer);
    } else if (strcmp(home_page_items[i].type, "list") == 0) {
      char *item;
      int counter = 0;
      char *items;

      // 1) Dynamic copy of the content
      items = malloc(strlen(home_page_items[i].content) + 1);
      if (items == NULL) {
          LOG_ERROR("Failed to allocate memory for list items");
          continue; // Skip to the next element
      }
      strcpy(items, home_page_items[i].content);

      // 2) Initialize a dynamic buffer for the entire list
      size_t listBufferCap = 1;            // Minimal initial capacity
      char *listBuffer = calloc(listBufferCap, 1); // Starts as empty string
      if (!listBuffer) {
          LOG_ERROR("Failed to allocate memory for listBuffer.");
          free(items);
          continue;
      }

      // 3) Process each element in the list
      item = strtok(items, ";");
      while (item != NULL) {
          // Dynamically generate the list item content using the template
          char *listItemBuffer = dynamic_sprintf(element_list_item_html, item);
          if (!listItemBuffer) {
              LOG_ERROR("Failed to build listItemBuffer for item: %s", item);
              break; // Error handling
          }

          // Concatenate the newly generated item into the final list buffer
          dynamic_strcat(&listBuffer, &listBufferCap, listItemBuffer);

          free(listItemBuffer); // Free the temporary buffer
          counter++;

          // Move to the next token (item)
          item = strtok(NULL, ";");
      }

      // 4) Format the full list using the list container
      char *finalList = dynamic_sprintf(element_list_container_html, listBuffer);
      if (!finalList) {
          LOG_ERROR("Failed to build finalList.");
          // Error handling: free buffers and continue
          free(items);
          free(listBuffer);
          continue;
      }

      // Copy the final content into itemBuffer
      itemLength = snprintf(itemBuffer, sizeof(itemBuffer), "%s", finalList);

      // Debug logs
      LOG_DEBUG("Final list:\n%s\n", finalList);

      // Free memory
      free(items);
      free(listBuffer);
      free(finalList);
    } else if (strcmp(home_page_items[i].type, "form-start") == 0) {
      const char *extra_data = home_page_items[i].extra_data;
      
      if (extra_data == NULL || strcmp(extra_data, "") == 0) {
          extra_data = "post";
      }

      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_form_start_html,
                  home_page_items[i].content, extra_data);
    } else if (strcmp(home_page_items[i].type, "form-end") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_form_end_html,
                  home_page_items[i].extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "button-primary") == 0) {
      const char *extra_data = home_page_items[i].extra_data;
      if (extra_data == NULL || strcmp(extra_data, "") == 0) {
          extra_data = "submit"; // Default to "submit" if null or empty
      }

      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_button_primary_html,
                  extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "button-secondary") == 0) {
      const char *extra_data = home_page_items[i].extra_data;
      if (extra_data == NULL || strcmp(extra_data, "") == 0) {
          extra_data = "submit";
      }

      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_button_secondary_html,
                  extra_data, home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "input-text") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_input_text_html,
                  home_page_items[i].content_id, home_page_items[i].content, 
                  home_page_items[i].content_id, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "radio-button") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_radio_button_html,
                  home_page_items[i].content_id, home_page_items[i].extra_data, 
                  home_page_items[i].content, home_page_items[i].content_id,
                  home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "checkbox") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_checkbox_html,
                  home_page_items[i].content_id, home_page_items[i].extra_data, 
                  home_page_items[i].content, home_page_items[i].content_id,
                  home_page_items[i].content);
    } else if (strcmp(home_page_items[i].type, "youtube-embed") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_youtube_embed_html,
                  home_page_items[i].content, home_page_items[i].extra_data);
    } else if (strcmp(home_page_items[i].type, "separator") == 0) {
      itemLength =
          snprintf(itemBuffer, sizeof(itemBuffer), element_separator_html,
                  home_page_items[i].extra_data);
    }

    if (itemLength < 0) {
      perror("Error formatting item");
      free(itemsBuffer);
      free(home_page_items);
      free((void *)page_container_html);
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
      free((void *)element_button_primary_html);
      free((void *)element_button_secondary_html);
      free((void *)element_input_text_html);
      free((void *)element_radio_button_html);
      free((void *)element_checkbox_html);
      free((void *)element_youtube_embed_html);
      free((void *)element_separator_html);
      return NULL;
    }

    // Reallocate memory for itemsBuffer
    char *tempBuffer = realloc(itemsBuffer, itemsBufferSize + itemLength + 1); // +1 for null terminator
    if (tempBuffer == NULL) {
      perror("Failed to allocate memory for itemsBuffer");
      free(itemsBuffer);
      free(home_page_items);
      free((void *)page_container_html);
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
      free((void *)element_button_primary_html);
      free((void *)element_button_secondary_html);
      free((void *)element_input_text_html);
      free((void *)element_radio_button_html);
      free((void *)element_checkbox_html);
      free((void *)element_youtube_embed_html);
      free((void *)element_separator_html);
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
      snprintf(NULL, 0, page_container_html, itemsBuffer) + 1; // +1 for null terminator
  char *contentBuffer = malloc(contentBufferSize);
  if (contentBuffer == NULL) {
    perror("Failed to allocate memory for contentBuffer");
    free(itemsBuffer);
    free(home_page_items);
    free((void *)page_container_html);
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
    free((void *)element_button_primary_html);
    free((void *)element_button_secondary_html);
    free((void *)element_input_text_html);
    free((void *)element_radio_button_html);
    free((void *)element_checkbox_html);
    free((void *)element_youtube_embed_html);
    free((void *)element_separator_html);
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

  return contentBuffer;
}
