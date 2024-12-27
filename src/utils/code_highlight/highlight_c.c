#include "../../include/code_highlight/highlight_c.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of C keywords
const char *c_keywords[] = {
    "int",    "float",    "double",  "char",    "void",  "if",       "else",
    "for",    "while",    "do",      "switch",  "case",  "break",    "continue",
    "return", "struct",   "typedef", "enum",    "union", "const",    "static",
    "extern", "unsigned", "signed",  "sizeof",  "NULL",  "volatile", "register",
    "goto",   "long",     "short",   "default", "auto",  NULL};

// Function to check if a word is a C keyword
int is_c_keyword(const char *word) {
  for (int i = 0; c_keywords[i]; i++) {
    if (strcmp(word, c_keywords[i]) == 0)
      return 1; // Return 1 if the word matches a C keyword
  }
  return 0; // Return 0 if the word is not a keyword
}

// Function to highlight C code with HTML tags
char *highlight_c(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);     // Allocate memory for the result
  if (!result) {
    fprintf(stderr, "Memory allocation error\n"); // Error if allocation fails
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the output HTML
  char token[256];      // Buffer to store tokens (e.g., keywords, identifiers)
  int in_string =
      0; // 0 = not inside a string, 1 = inside a double-quoted string
  int in_char =
      0; // 0 = not inside a char literal, 1 = inside a single-quoted char
  int in_comment_single = 0; // 1 = inside a single-line comment (//)
  int in_comment_multi = 0;  // 1 = inside a multi-line comment (/* */)

  // Main loop to process the input code character by character
  while (*p) {
    if (in_string) {
      // Handle characters inside a double-quoted string
      if (*p == '\\') {
        // Handle escape sequences
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p ? *p++ : '\0'; // Check for null terminator
        temp[2] = '\0';

        char escaped[20];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\"') {
        // End of the string
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close the span for the string
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_string = 0; // Exit string mode
      } else {
        // Regular characters inside the string
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_char) {
      // Handle characters inside a single-quoted char literal
      if (*p == '\\') {
        // Handle escape sequences in char literals
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p ? *p++ : '\0';
        temp[2] = '\0';

        char escaped[20];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'') {
        // End of the char literal
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close the span for the char literal
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_char = 0; // Exit char mode
      } else {
        // Regular characters inside the char literal
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment_single) {
      // Handle single-line comments
      if (*p == '\n') {
        // End of the comment, replace newline with <br>
        const char *close_span = "</span>";
        strcpy(res_p, close_span); // Close the comment span
        res_p += strlen(close_span);

        strcpy(res_p, "<br>"); // Add <br> for newline in HTML
        res_p += 4;
        p++;

        in_comment_single = 0; // Exit single-line comment mode
      } else {
        // Characters inside the comment
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment_multi) {
      // Handle multi-line comments
      if (*p == '*' && *(p + 1) == '/') {
        // End of the comment (*/)
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        const char *close_span = "</span>";
        strcpy(res_p, close_span); // Close the comment span
        res_p += strlen(close_span);

        in_comment_multi = 0; // Exit multi-line comment mode
      } else if (*p == '\n') {
        // Replace newline with <br> in multi-line comments
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Characters inside the comment
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      // Handle code outside of strings, chars, and comments
      if (*p == '\"') {
        // Start of a string
        in_string = 1;

        // Start a span for the string
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'') {
        // Start of a char literal
        in_char = 1;

        // Start a span for the char literal
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '/' && *(p + 1) == '/') {
        // Start of a single-line comment
        in_comment_single = 1;

        // Start a span for the comment
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '/' && *(p + 1) == '*') {
        // Start of a multi-line comment
        in_comment_multi = 1;

        // Start a span for the comment
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (isalpha(*p) || *p == '_') {
        // Handle identifiers (keywords or variables)
        int token_index = 0;

        // Collect the identifier
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0';

        char escaped_token[1024];
        escape_html_chars(token, escaped_token); // Escape the token

        if (is_c_keyword(token)) {
          // Highlight C keywords in cyan
          int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
          // Regular identifiers
          strcpy(res_p, escaped_token);
          res_p += strlen(escaped_token);
        }
      } else if (isdigit(*p)) {
        // Handle numeric literals
        int token_index = 0;

        // Collect the number
        while (isdigit(*p) || *p == '.' || *p == 'x' || *p == 'X' ||
               (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0';

        char escaped_token[1024];
        escape_html_chars(token, escaped_token); // Escape the token

        // Highlight numbers in purple
        int n = sprintf(res_p, "<span style=\"color:purple;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (*p == '\n') {
        // Replace newline with <br>
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Handle other characters
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped); // Escape for HTML
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any remaining open spans (for strings, chars, or comments)
  if (in_string || in_char || in_comment_single || in_comment_multi) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the final result
  return result; // Return the highlighted HTML code
}
