#include "../../include/code_highlight/highlight_css.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define INITIAL_BUFFER_SIZE 1024

// Function to append a string to the result buffer, expanding the buffer if
// necessary
void append_to_result(char **res_p, size_t *res_size, size_t *res_capacity,
                      const char *str) {
  size_t len = strlen(str);
  if (*res_size + len >= *res_capacity) {
    // Increase buffer size if needed
    *res_capacity *= 2;
    size_t offset = *res_p - *res_p;
    *res_p = realloc(*res_p, *res_capacity);
    if (!*res_p) {
      fprintf(stderr, "Error reallocating memory\n");
      exit(1);
    }
    *res_p += offset;
  }
  strcpy(*res_p + *res_size, str); // Append string to the buffer
  *res_size += len;                // Update the size of the result
}

// Function to highlight CSS code and return it as an HTML-formatted string
char *highlight_css(const char *code) {
  size_t res_capacity =
      INITIAL_BUFFER_SIZE;             // Initial buffer size for the result
  size_t res_size = 0;                 // Current size of the result
  char *result = malloc(res_capacity); // Allocate memory for the result
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }
  result[0] = '\0'; // Initialize the result string as empty

  const char *p = code; // Pointer to traverse the input CSS code
  int in_comment = 0;   // 1 if inside a comment block (/* */)
  int in_selector = 1;  // 1 if currently in a CSS selector

  // Main loop to process each character in the input code
  while (*p) {
    if (in_comment) {
      // Handle characters inside a comment block
      if (*p == '*' && *(p + 1) == '/') {
        // Close comment block when encountering */
        char temp[3] = {*p++, *p++, '\0'};

        char escaped[20];
        escape_html_chars(temp, escaped); // Escape for HTML
        append_to_result(&result, &res_size, &res_capacity, escaped);

        // Close the comment's HTML span
        const char *close_span = "</span>";
        append_to_result(&result, &res_size, &res_capacity, close_span);

        in_comment = 0; // Exit comment mode
      } else {
        if (*p == '\n') {
          // Replace newline with <br> for HTML
          append_to_result(&result, &res_size, &res_capacity, "<br>");
          p++;
        } else {
          // Escape and append characters inside the comment
          char temp[2] = {*p++, '\0'};
          char escaped[10];
          escape_html_chars(temp, escaped);
          append_to_result(&result, &res_size, &res_capacity, escaped);
        }
      }
    } else {
      // Handle code outside of comments
      if (*p == '/' && *(p + 1) == '*') {
        // Start of a comment block
        in_comment = 1;

        // Start HTML span for the comment
        const char *open_span = "<span style=\"color:green;\">";
        append_to_result(&result, &res_size, &res_capacity, open_span);

        char temp[3] = {*p++, *p++, '\0'};
        char escaped[20];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else if (*p == '{') {
        // Start of CSS properties block
        in_selector = 0;

        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else if (*p == '}') {
        // End of CSS properties block
        in_selector = 1;

        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else if (in_selector && (*p != '\n')) {
        // Highlight CSS selectors
        char selector[256];
        int selector_index = 0;
        while (*p && *p != '{' && *p != '\n') {
          if ((size_t)selector_index < sizeof(selector) - 1) {
            selector[selector_index++] = *p++;
          } else {
            break; // Avoid overflow
          }
        }
        selector[selector_index] = '\0';

        char escaped_selector[1024];
        escape_html_chars(selector, escaped_selector);

        // Format selector in cyan
        char buffer[2048];
        snprintf(buffer, sizeof(buffer),
                 "<span style=\"color:#55FFFF;\">%s</span>", escaped_selector);
        append_to_result(&result, &res_size, &res_capacity, buffer);
      } else if (isalpha(*p) || *p == '-') {
        // Highlight CSS property names
        char property[256];
        int prop_index = 0;

        while (isalnum(*p) || *p == '-' || *p == '_') {
          if ((size_t)prop_index < sizeof(property) - 1) {
            property[prop_index++] = *p++;
          } else {
            break; // Avoid overflow
          }
        }
        property[prop_index] = '\0';

        char escaped_property[1024];
        escape_html_chars(property, escaped_property);

        // Format property name in red
        char buffer[2048];
        snprintf(buffer, sizeof(buffer), "<span style=\"color:red;\">%s</span>",
                 escaped_property);
        append_to_result(&result, &res_size, &res_capacity, buffer);
      } else if (*p == ':') {
        // Append colon (:) without special formatting
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else if (*p == ';') {
        // Append semicolon (;) without special formatting
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else if (*p == '\n') {
        // Replace newline with <br> for HTML
        append_to_result(&result, &res_size, &res_capacity, "<br>");
        p++;
      } else if (*p == ' ' || *p == '\t') {
        // Escape and append spaces or tabs
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      } else {
        // Escape and append other characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        append_to_result(&result, &res_size, &res_capacity, escaped);
      }
    }
  }

  // Close any open spans (if in comment when loop ends)
  if (in_comment) {
    const char *close_span = "</span>";
    append_to_result(&result, &res_size, &res_capacity, close_span);
  }

  result[res_size] = '\0'; // Null-terminate the result string
  return result;           // Return the highlighted CSS code
}
