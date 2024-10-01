#ifndef GOOGLE_SHEETS_API_H
#define GOOGLE_SHEETS_API_H

// Function to fetch data from a Google Sheets document using the Google Sheets API
// The result is returned as a JSON string, which should be parsed by the caller.
// The caller is responsible for freeing the returned string.
char* fetch_google_sheets_data(const char* range);

#endif // GOOGLE_SHEETS_API_H
