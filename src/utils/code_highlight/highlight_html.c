#include "../../include/code_highlight/highlight_html.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function to highlight HTML code by adding HTML tags for formatting
char *highlight_html(const char *code) {
  size_t result_size = strlen(code) * 10; // Estimated size for the result
  char *result = malloc(result_size);     // Allocate memory for the result
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input HTML code
  char *res_p = result; // Pointer to construct the output HTML

  int in_tag = 0;        // 1 if inside an HTML tag
  int in_attr_name = 0;  // 1 if inside an attribute name
  int in_attr_value = 0; // 1 if inside an attribute value
  int attr_value_quote =
      0; // Holds the quote character (' or ") used for attribute values

  // Main loop to process each character in the input HTML code
  while (*p) {
    if (*p == '<') {
      in_tag = 1; // Start of an HTML tag
      // Add span for the HTML tag
      const char *open_span = "<span style=\"color:#55FFFF;\">";
      strcpy(res_p, open_span);
      res_p += strlen(open_span);

      // Escape and append the '<' character
      char temp[2] = {*p++, '\0'};
      char escaped[10];
      escape_html_chars(temp, escaped);
      strcpy(res_p, escaped);
      res_p += strlen(escaped);
    } else if (*p == '>') {
      // Handle the closing '>' of the HTML tag
      char temp[2] = {*p++, '\0'};
      char escaped[10];
      escape_html_chars(temp, escaped);
      strcpy(res_p, escaped);
      res_p += strlen(escaped);

      // Close the tag span
      const char *close_span = "</span>";
      strcpy(res_p, close_span);
      res_p += strlen(close_span);

      // Reset tag and attribute flags
      in_tag = 0;
      in_attr_name = 0;
      in_attr_value = 0;
      attr_value_quote = 0;
    } else if (in_tag) {
      // Process characters inside an HTML tag
      if (isspace(*p)) {
        // Handle spaces
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        if (!in_attr_value) {
          in_attr_name = 1; // Prepare to capture attribute name
        }
      } else if (*p == '=') {
        // Handle '=' between attribute name and value
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        in_attr_name = 0;
        in_attr_value = 1; // Prepare to capture attribute value
      } else if (*p == '\'' || *p == '\"') {
        // Handle quotes around attribute values
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);

        if (in_attr_value) {
          if (attr_value_quote == 0) {
            // Start of attribute value
            attr_value_quote = temp[0];

            // Add span for the attribute value
            const char *open_span = "<span style=\"color:orange;\">";
            strcpy(res_p, open_span);
            res_p += strlen(open_span);

            strcpy(res_p, escaped); // Append the opening quote
            res_p += strlen(escaped);
          } else if (attr_value_quote == temp[0]) {
            // End of attribute value
            strcpy(res_p, escaped); // Append the closing quote
            res_p += strlen(escaped);

            // Close the attribute value span
            const char *close_span = "</span>";
            strcpy(res_p, close_span);
            res_p += strlen(close_span);

            in_attr_value = 0;
            attr_value_quote = 0; // Reset the quote flag
          } else {
            // Inside the value, append the escaped character
            strcpy(res_p, escaped);
            res_p += strlen(escaped);
          }
        } else {
          strcpy(res_p, escaped); // Append unescaped character
          res_p += strlen(escaped);
        }
      } else {
        // Handle attribute names or values
        if (in_attr_name) {
          // Highlight attribute names
          const char *open_span = "<span style=\"color:red;\">";
          strcpy(res_p, open_span);
          res_p += strlen(open_span);

          char attr_name[256];
          int attr_index = 0;
          while (*p && !isspace(*p) && *p != '=' && *p != '>' && *p != '/' &&
                 *p != '<') {
            if ((size_t)attr_index < sizeof(attr_name) - 1) {
              attr_name[attr_index++] = *p++;
            } else {
              p++;
            }
          }
          attr_name[attr_index] = '\0';

          char escaped_attr_name[1024];
          escape_html_chars(attr_name, escaped_attr_name);

          strcpy(res_p, escaped_attr_name); // Append escaped attribute name
          res_p += strlen(escaped_attr_name);

          // Close the attribute name span
          const char *close_span = "</span>";
          strcpy(res_p, close_span);
          res_p += strlen(close_span);

          in_attr_name = 0;
        } else if (in_attr_value) {
          // Handle characters inside an attribute value
          char temp[2] = {*p++, '\0'};
          char escaped[10];
          escape_html_chars(temp, escaped);
          strcpy(res_p, escaped);
          res_p += strlen(escaped);
        } else {
          // Handle other characters inside the tag
          char temp[2] = {*p++, '\0'};
          char escaped[10];
          escape_html_chars(temp, escaped);
          strcpy(res_p, escaped);
          res_p += strlen(escaped);
        }
      }
    } else {
      // Handle code outside of tags
      if (*p == '\n') {
        // Replace newline with <br> for HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Escape and append other characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any open spans for attribute values or tags
  if (attr_value_quote != 0) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }
  if (in_tag) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the result string
  return result; // Return the highlighted HTML code
}
