#ifndef DATA_FETCHER_H
#define DATA_FETCHER_H

// This function returns a 2D array of strings
char ***getData(const char *sheetName, const char *startCell, const char *endCell, int *rowCount);

// Function to free the dynamically allocated data
void freeData(char*** data, int rows);

#endif // DATA_FETCHER_H
