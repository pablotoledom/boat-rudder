#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "home_blog_items.h"
#include "../include/log.h"
#include "data_fetcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the home blog items
HomeBlogItems *getBlogItems(int *blogHomeCount) {
  // Call getData and pass a pointer to blogHomeCount
  char ***responseList = getData("BlogSummary", "A2", "G20", blogHomeCount);

  // Print the table for debugging (optional)
  LOG_DEBUG("\nNumber of Home Blog: %d\n\n", *blogHomeCount);

  // Allocate memory for the blog items list
  HomeBlogItems *home_blog_list =
      (HomeBlogItems *)malloc(*blogHomeCount * sizeof(HomeBlogItems));
  if (home_blog_list == NULL) {
    perror("Failed to allocate memory for home blog list");
    freeData(responseList,
             *blogHomeCount); // Free responseList in case of failure
    return NULL;
  }

  // Initialize all fields to empty strings
  for (int i = 0; i < *blogHomeCount; i++) {
    memset(&home_blog_list[i], 0, sizeof(HomeBlogItems));
  }

  // Process the response list
  for (int i = 0; i < *blogHomeCount; i++) {
    if (responseList[i] != NULL) {
      if (responseList[i][0] != NULL)
        strncpy(home_blog_list[i].id, responseList[i][0],
                sizeof(home_blog_list[i].id) - 1);
      else
        home_blog_list[i].id[0] = '\0';

      if (responseList[i][1] != NULL)
        strncpy(home_blog_list[i].url, responseList[i][1],
                sizeof(home_blog_list[i].url) - 1);
      else
        home_blog_list[i].url[0] = '\0';

      if (responseList[i][2] != NULL)
        strncpy(home_blog_list[i].image_url, responseList[i][2],
                sizeof(home_blog_list[i].image_url) - 1);
      else
        home_blog_list[i].image_url[0] = '\0';

      if (responseList[i][3] != NULL)
        strncpy(home_blog_list[i].tittle, responseList[i][3],
                sizeof(home_blog_list[i].tittle) - 1);
      else
        home_blog_list[i].tittle[0] = '\0';

      if (responseList[i][4] != NULL)
        strncpy(home_blog_list[i].summary, responseList[i][4],
                sizeof(home_blog_list[i].summary) - 1);
      else
        home_blog_list[i].summary[0] = '\0';

      if (responseList[i][5] != NULL)
        strncpy(home_blog_list[i].author, responseList[i][5],
                sizeof(home_blog_list[i].author) - 1);
      else
        home_blog_list[i].author[0] = '\0';

      if (responseList[i][6] != NULL)
        strncpy(home_blog_list[i].summary_date, responseList[i][6],
                sizeof(home_blog_list[i].summary_date) - 1);
      else
        home_blog_list[i].summary_date[0] = '\0';
    } else {
      printf("Row %d is NULL, skipping.\n", i);
    }
  }

  // Freeing responseList memory
  freeData(responseList, *blogHomeCount);

  return home_blog_list;
}
