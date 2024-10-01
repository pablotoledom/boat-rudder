// route.h

#ifndef ROUTE_H
#define ROUTE_H

#define MAX_ID_LENGTH 64
#define MAX_NAME_LENGTH 128
#define MAX_LINK_LENGTH 256

typedef struct {
    char id[MAX_ID_LENGTH];
    char name[MAX_NAME_LENGTH];
    char link[MAX_LINK_LENGTH];
} Route;

// Función prototipo
Route *getRoutes(int *routeCount);

#endif // ROUTE_H
