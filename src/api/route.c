#define _XOPEN_SOURCE 700  // Define POSIX.1-2008 compliance level

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "route.h"
#include "data_fetcher.h"

// Function to get the routes
Route* getRoutes(int* routeCount) {
    // Call getData and pass a pointer to routeCount
    char*** responseList = getData("Routes", "A3", "C20", routeCount);

    printf("Number of routes: %d\n", *routeCount);

    // Allocate memory for the route list
    Route* routeList = (Route*)malloc(*routeCount * sizeof(Route));

    // Process the response list
    for (int i = 0; i < *routeCount; i++) {
        strcpy(routeList[i].id, responseList[i][0]);
        strcpy(routeList[i].name, responseList[i][1]);
        strcpy(routeList[i].link, responseList[i][2]);
    }

    // Freeing responseList memory
    freeData(responseList, *routeCount);

    return routeList;
}
