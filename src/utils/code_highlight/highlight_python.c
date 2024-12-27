#include "../../include/code_highlight/highlight_python.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of Python keywords
const char *python_keywords[] = {
    "def",    "class",    "if",     "elif",   "else",  "for",    "while",
    "break",  "continue", "return", "import", "from",  "as",     "pass",
    "None",   "True",     "False",  "and",    "or",    "not",    "in",
    "is",     "lambda",   "with",   "yield",  "try",   "except", "finally",
    "global", "nonlocal", "assert", "del",    "async", "await",  "raise",
    "print",  NULL};

// Function to check if a word is a Python keyword
int is_python_keyword(const char *word) {
  for (int i = 0; python_keywords[i]; i++) {
    if (strcmp(word, python_keywords[i]) == 0)
      return 1;
  }
  return 0;
}

// Function to highlight Python code by adding HTML tags
char *highlight_python(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the output HTML
  char token[256];      // Buffer to store tokens (e.g., keywords)
  int in_string = 0;    // 0 = not in string, 1 = single-quoted string, 2 =
                        // double-quoted string, 3 = triple-quoted string
  int string_quote = 0; // Stores the type of quote (single or double)
  int in_comment = 0;   // 1 = in a comment (starts with #)

  // Main loop to process each character in the input code
  while (*p) {
    if (in_string) {
      if (*p == '\\') {
        // Handle escape characters
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p ? *p++ : '\0';
        temp[2] = '\0';

        char escaped[20];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if ((*p == string_quote && in_string == 1) ||
                 (*p == string_quote && in_string == 2)) {
        // Handle closing of the string
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close string span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_string = 0; // Exit string mode
      } else if (*p == '\n') {
        // Replace newline with <br> for HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Handle regular characters inside the string
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment) {
      if (*p == '\n') {
        // Close comment span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        // Replace newline with <br>
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;

        in_comment = 0; // Exit comment mode
      } else {
        // Handle characters inside comment
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      if (*p == '#' || (*p == '/' && *(p + 1) == '/')) {
        // Start of a comment
        in_comment = 1;

        // Start comment span
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[3];
        temp[0] = *p++;
        if (temp[0] == '/' && *p == '/') {
          temp[1] = *p++;
          temp[2] = '\0';
        } else {
          temp[1] = '\0';
        }

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'' || *p == '\"') {
        // Start of a string (single or double-quoted)
        in_string = 1;
        string_quote = *p;

        // Start string span
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (isalpha(*p) || *p == '_') {
        // Handle identifiers (e.g., keywords or variables)
        int token_index = 0;

        while (isalnum(*p) || *p == '_') {
          if ((size_t)token_index < sizeof(token) - 1) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0'; // Null-terminate the token

        char escaped_token[1024];
        escape_html_chars(token, escaped_token); // Escape the token for HTML

        if (is_python_keyword(token)) {
          // Highlight Python keywords
          int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
          // Regular identifiers
          strcpy(res_p, escaped_token);
          res_p += strlen(escaped_token);
        }
      } else if (isdigit(*p)) {
        // Handle numbers
        int token_index = 0;

        while (isdigit(*p) || *p == '.') {
          if ((size_t)token_index < sizeof(token) - 1) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0'; // Null-terminate the number

        char escaped_token[1024];
        escape_html_chars(token, escaped_token); // Escape the number for HTML

        int n = sprintf(res_p, "<span style=\"color:purple;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (*p == '\n') {
        // Replace newline with <br>
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else if (*p == ' ' || *p == '\t') {
        // Handle spaces and tabs
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else {
        // Handle other characters (e.g., operators)
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any open spans (for strings or comments)
  if (in_string || in_comment) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the result
  return result; // Return the highlighted Python code
}
