#ifndef ROUTE_H
#define ROUTE_H

// Define the Route structure
typedef struct {
    char id[10];
    char name[50];
    char link[100];
} Route;

// Function to get the routes
Route* getRoutes(int* routeCount);

#endif // ROUTE_H
