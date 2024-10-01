#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "home_blog_items.h"
#include "../include/config_loader.h"
#include "../include/print_table.h"
#include "data_fetcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to get the home blog items
HomeBlogItems *getBlogItems(int *routeCount) {
    // Call getData and pass a pointer to routeCount
    char ***responseList = getData("BlogSummary", "A2", "E20", routeCount);

    if (verbose) {
        // Print the table for debugging (optional)
        printTable(responseList);
        printf("\nNumber of Home Blog: %d\n\n", *routeCount);
    }

    // Allocate memory for the blog items list
    HomeBlogItems *home_blog_list =
        (HomeBlogItems *)malloc(*routeCount * sizeof(HomeBlogItems));
    if (home_blog_list == NULL) {
        perror("Failed to allocate memory for home blog list");
        freeData(responseList, *routeCount); // Free responseList in case of failure
        return NULL;
    }

    // Initialize all fields to empty strings
    for (int i = 0; i < *routeCount; i++) {
        memset(&home_blog_list[i], 0, sizeof(HomeBlogItems));
    }

    // Process the response list
    for (int i = 0; i < *routeCount; i++) {
        if (responseList[i] != NULL) {
            if (responseList[i][0] != NULL)
                strncpy(home_blog_list[i].id, responseList[i][0], sizeof(home_blog_list[i].id) - 1);
            else
                home_blog_list[i].id[0] = '\0';

            if (responseList[i][1] != NULL)
                strncpy(home_blog_list[i].url, responseList[i][1], sizeof(home_blog_list[i].url) - 1);
            else
                home_blog_list[i].url[0] = '\0';

            if (responseList[i][2] != NULL)
                strncpy(home_blog_list[i].image_url, responseList[i][2], sizeof(home_blog_list[i].image_url) - 1);
            else
                home_blog_list[i].image_url[0] = '\0';

            if (responseList[i][3] != NULL)
                strncpy(home_blog_list[i].tittle, responseList[i][3], sizeof(home_blog_list[i].tittle) - 1);
            else
                home_blog_list[i].tittle[0] = '\0';

            if (responseList[i][4] != NULL)
                strncpy(home_blog_list[i].summary, responseList[i][4], sizeof(home_blog_list[i].summary) - 1);
            else
                home_blog_list[i].summary[0] = '\0';
        } else {
            printf("Row %d is NULL, skipping.\n", i);
        }
    }

    // Freeing responseList memory
    freeData(responseList, *routeCount);

    return home_blog_list;
}
