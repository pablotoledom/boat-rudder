#ifndef DATA_FETCHER_H
#define DATA_FETCHER_H

// Simulate the getData function
// This function returns a 2D array of strings, similar to what you'd get from TypeScript's getData.
char*** getData();

// Function to free the dynamically allocated data
void freeData(char*** data, int rows);

#endif // DATA_FETCHER_H
