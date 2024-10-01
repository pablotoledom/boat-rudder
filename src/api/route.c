#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/config_loader.h"
#include "route.h"
#include "data_fetcher.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Implementación de la función getRoutes
Route *getRoutes(int *routeCount) {
    // Llamamos a getData y pasamos un puntero a routeCount
    char ***responseList = getData("Routes", "A3", "C20", routeCount);

    if (verbose) {
        printf("\nNumber of routes: %d\n\n", *routeCount);
    }

    // Verificamos si responseList es NULL
    if (responseList == NULL) {
        perror("Failed to get data");
        return NULL;
    }

    // Asignamos memoria para la lista de rutas
    Route *routeList = (Route *)malloc(*routeCount * sizeof(Route));
    if (routeList == NULL) {
        perror("Failed to allocate memory for route list");
        freeData(responseList, *routeCount);  // Liberamos responseList en caso de fallo
        return NULL;
    }

    // Inicializamos todos los campos a cadenas vacías
    for (int i = 0; i < *routeCount; i++) {
        memset(&routeList[i], 0, sizeof(Route));
    }

    // Procesamos la lista de respuestas
    for (int i = 0; i < *routeCount; i++) {
        if (responseList[i] != NULL) {
            if (responseList[i][0] != NULL)
                strncpy(routeList[i].id, responseList[i][0], sizeof(routeList[i].id) - 1);
            else
                routeList[i].id[0] = '\0';

            if (responseList[i][1] != NULL)
                strncpy(routeList[i].name, responseList[i][1], sizeof(routeList[i].name) - 1);
            else
                routeList[i].name[0] = '\0';

            if (responseList[i][2] != NULL)
                strncpy(routeList[i].link, responseList[i][2], sizeof(routeList[i].link) - 1);
            else
                routeList[i].link[0] = '\0';
        } else {
            printf("Row %d is NULL, skipping.\n", i);
        }
    }

    // Liberamos la memoria de responseList
    freeData(responseList, *routeCount);

    return routeList;
}
