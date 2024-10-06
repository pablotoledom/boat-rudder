#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "blog_entry_items.h"
#include "../include/log.h"
#include "data_fetcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the home blog items
BlogEntryItems *getBlogEntryItems(const char *EntryId, int *blogHomeCount) {
  // Call getData and get the total number of rows
  int totalRowCount = 0;
  char ***responseList = getData("Blog", "A2", "E200", &totalRowCount);

  if (responseList == NULL) {
    perror("Failed to get data");
    *blogHomeCount = 0;
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
    *blogHomeCount = 0;
    return NULL;
  }

  // Allocate memory for the blog items list based on matchingRowCount
  BlogEntryItems *home_blog_list =
      (BlogEntryItems *)malloc(matchingRowCount * sizeof(BlogEntryItems));
  if (home_blog_list == NULL) {
    perror("Failed to allocate memory for home blog list");
    freeData(responseList,
             totalRowCount); // Free responseList in case of failure
    *blogHomeCount = 0;
    return NULL;
  }

  // Initialize all fields to empty strings
  for (int i = 0; i < matchingRowCount; i++) {
    memset(&home_blog_list[i], 0, sizeof(BlogEntryItems));
  }

  // Process the response list and copy matching entries
  int index = 0;
  for (int i = 0; i < totalRowCount; i++) {
    if (responseList[i] != NULL && responseList[i][0] != NULL &&
        strcmp(responseList[i][0], EntryId) == 0) {

      if (responseList[i][0] != NULL)
        strncpy(home_blog_list[index].entry_id, responseList[i][0],
                sizeof(home_blog_list[index].entry_id) - 1);
      else
        home_blog_list[index].entry_id[0] = '\0';

      if (responseList[i][1] != NULL)
        strncpy(home_blog_list[index].content_id, responseList[i][1],
                sizeof(home_blog_list[index].content_id) - 1);
      else
        home_blog_list[index].content_id[0] = '\0';

      if (responseList[i][2] != NULL)
        strncpy(home_blog_list[index].type, responseList[i][2],
                sizeof(home_blog_list[index].type) - 1);
      else
        home_blog_list[index].type[0] = '\0';

      if (responseList[i][3] != NULL)
        strncpy(home_blog_list[index].content, responseList[i][3],
                sizeof(home_blog_list[index].content) - 1);
      else
        home_blog_list[index].content[0] = '\0';

      if (responseList[i][4] != NULL)
        strncpy(home_blog_list[index].extra_data, responseList[i][4],
                sizeof(home_blog_list[index].extra_data) - 1);
      else
        home_blog_list[index].extra_data[0] = '\0';

      index++;
    }
  }

  // Update the blogHomeCount to the number of matching entries
  *blogHomeCount = matchingRowCount;

  // Freeing responseList memory
  freeData(responseList, totalRowCount);

  return home_blog_list;
}
