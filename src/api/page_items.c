#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "page_items.h"
#include "../include/log.h"
#include "data_fetcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the home page items
PageItems *getPageItems(const char *EntryId, int *pageCount) {
  // Call getData and get the total number of rows
  int totalRowCount = 0;
  char ***responseList = getData("Pages", "A2", "E200", &totalRowCount);

  if (responseList == NULL) {
    perror("Failed to get data");
    *pageCount = 0;
    return NULL;
  }

  // First, count the number of rows where the first column matches EntryId
  int matchingRowCount = 0;
  for (int i = 0; i < totalRowCount; i++) {
    if (responseList[i] != NULL && responseList[i][0] != NULL &&
        strcmp(responseList[i][0], EntryId) == 0) {
      matchingRowCount++;
    }
  }

  if (matchingRowCount == 0) {
    LOG_DEBUG("No matching entries found for EntryId: %s\n", EntryId);
    freeData(responseList, totalRowCount);
    *pageCount = 0;
    return NULL;
  }

  // Allocate memory for the page items list based on matchingRowCount
  PageItems *page_list =
      (PageItems *)malloc(matchingRowCount * sizeof(PageItems));
  if (page_list == NULL) {
    perror("Failed to allocate memory for home page list");
    freeData(responseList,
             totalRowCount); // Free responseList in case of failure
    *pageCount = 0;
    return NULL;
  }

  // Initialize all fields to empty strings
  for (int i = 0; i < matchingRowCount; i++) {
    memset(&page_list[i], 0, sizeof(PageItems));
  }

  // Process the response list and copy the matching entries
  int index = 0;
  for (int i = 0; i < totalRowCount; i++) {
    if (responseList[i] != NULL && responseList[i][0] != NULL &&
      strcmp(responseList[i][0], EntryId) == 0) {

      // Check and copy mandatory values
      if (responseList[i][0] != NULL) {
          strncpy(page_list[index].entry_id, responseList[i][0],
                  sizeof(page_list[index].entry_id) - 1);
      } else {
          LOG_ERROR("Missing entry_id value in row %d", i + 1);
          continue; // Skip this row
      }

      if (responseList[i][1] != NULL) {
          strncpy(page_list[index].content_id, responseList[i][1],
                  sizeof(page_list[index].content_id) - 1);
      } else {
          LOG_ERROR("Missing content_id value in row %d", i + 1);
          continue; // Skip this row
      }

      if (responseList[i][2] != NULL) {
          strncpy(page_list[index].type, responseList[i][2],
                  sizeof(page_list[index].type) - 1);
      } else {
          LOG_ERROR("Missing type value in row %d", i + 1);
          continue; // Skip this row
      }

      if (responseList[i][3] != NULL) {
          strncpy(page_list[index].content, responseList[i][3],
                  sizeof(page_list[index].content) - 1);
      } else {
          LOG_ERROR("Missing content value in row %d", i + 1);
          continue; // Skip this row
      }

      // Copy the optional `extra_data` value, if present
      if (responseList[i][4] != NULL) {
          strncpy(page_list[index].extra_data, responseList[i][4],
                  sizeof(page_list[index].extra_data) - 1);
      } else {
          page_list[index].extra_data[0] = '\0'; // Initialize as an empty string
      }

      index++;
    }
  }

  // Update the pageCount to the number of matching entries
  *pageCount = matchingRowCount;

  // Freeing responseList memory
  freeData(responseList, totalRowCount);

  return page_list;
}
