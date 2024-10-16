#include "../../include/code_highlight/escape_code.h"
#include <string.h>

// Function to escape special HTML characters and replace
void escape_html_chars(const char *input, char *output) {
  // Loop through each character of the input string
  while (*input) {
    // If the current character is a space, replace it with "&nbsp;"
    if (*input == ' ') {
      strcpy(output, "&nbsp;");
      output += 6; // Move the output pointer forward by the length of "&nbsp;"
      input++;     // Move to the next input character
    }
    // If the current character is '&', replace it with "&amp;"
    else if (*input == '&') {
      strcpy(output, "&amp;");
      output += 5; // Move the output pointer forward by the length of "&amp;"
      input++;     // Move to the next input character
    }
    // If the current character is '<', replace it with "&lt;"
    else if (*input == '<') {
      strcpy(output, "&lt;");
      output += 4; // Move the output pointer forward by the length of "&lt;"
      input++;     // Move to the next input character
    }
    // If the current character is '>', replace it with "&gt;"
    else if (*input == '>') {
      strcpy(output, "&gt;");
      output += 4; // Move the output pointer forward by the length of "&gt;"
      input++;     // Move to the next input character
    }
    // If it's not a special character, copy it as-is
    else {
      *output++ = *input++; // Copy the character and move both pointers
    }
  }
  *output = '\0'; // Terminate the output string with a null character
}
