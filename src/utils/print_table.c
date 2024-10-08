#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/print_table.h"
#include <stddef.h>
#include <stdio.h>

void printTable(char ***table) {
    // Validar que el array de resultados no sea NULL
    if (table == NULL) {
        printf("No data to display.\n");
        return;
    }

    // Iterar sobre cada fila hasta encontrar un NULL
    int i = 0;
    while (table[i] != NULL) {
        printf("Row %d:\n", i);

        // Iterar sobre cada columna hasta encontrar un NULL
        int j = 0;
        while (table[i][j] != NULL) {
            printf("  Column %d: %s\n", j, table[i][j]);
            j++;
        }
        
        i++;
    }
}