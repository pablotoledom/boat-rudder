#include "../../include/code_highlight/highlight_js.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of JavaScript keywords
const char *js_keywords[] = {
    "break",    "case",       "catch",  "class",    "const",  "continue",
    "debugger", "default",    "delete", "do",       "else",   "export",
    "extends",  "finally",    "for",    "function", "if",     "import",
    "in",       "instanceof", "let",    "new",      "return", "super",
    "switch",   "this",       "throw",  "try",      "typeof", "var",
    "void",     "while",      "with",   "yield",    "null",   "true",
    "false",    "await",      NULL};

// Function to check if a word is a JavaScript keyword
int is_js_keyword(const char *word) {
  for (int i = 0; js_keywords[i]; i++) {
    if (strcmp(word, js_keywords[i]) == 0)
      return 1;
  }
  return 0;
}

// Function to highlight JavaScript code
char *highlight_js(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the result HTML
  char token[256];      // Buffer to hold tokens (e.g., keywords)
  int in_string = 0; // 0 = not in string, 1 = in single-quoted string, 2 = in
                     // double-quoted string
  int in_comment_single = 0; // 1 = in single-line comment
  int in_comment_multi = 0;  // 1 = in multi-line comment

  // Main loop to process each character in the input code
  while (*p) {
    if (in_string) {
      // Handle characters inside a string
      if (*p == '\\') {
        // Handle escape sequences
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p ? *p++ : '\0';
        temp[2] = '\0';

        char escaped[20];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if ((in_string == 1 && *p == '\'') ||
                 (in_string == 2 && *p == '\"')) {
        // Handle closing of string
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close string span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_string = 0; // Exit string mode
      } else {
        // Handle regular characters inside string
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment_single) {
      // Handle single-line comments
      if (*p == '\n') {
        // Close comment span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        // Move pointer and replace newline with <br>
        p++;
        strcpy(res_p, "<br>");
        res_p += 4;

        in_comment_single = 0; // Exit single-line comment mode
      } else {
        // Handle regular characters inside comment
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment_multi) {
      // Handle multi-line comments
      if (*p == '*' && *(p + 1) == '/') {
        // End of multi-line comment
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close comment span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_comment_multi = 0; // Exit multi-line comment mode
      } else if (*p == '\n') {
        // Handle newlines inside multi-line comments
        p++;
        strcpy(res_p, "<br>");
        res_p += 4;
      } else {
        // Handle regular characters inside comment
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      // Handle code outside strings and comments
      if (*p == '\'' || *p == '\"') {
        // Start of a string (single or double quote)
        in_string = (*p == '\'') ? 1 : 2;

        // Start string span
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '/' && *(p + 1) == '/') {
        // Start of a single-line comment
        in_comment_single = 1;

        // Start comment span
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '/' && *(p + 1) == '*') {
        // Start of a multi-line comment
        in_comment_multi = 1;

        // Start comment span
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (isalpha(*p) || *p == '_') {
        // Handle identifiers (e.g., keywords or variables)
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0'; // Null-terminate the token

        char escaped_token[1024];
        escape_html_chars(token, escaped_token); // Escape the token

        if (is_js_keyword(token)) {
          // Highlight JavaScript keywords
          int n = sprintf(res_p, "<span style=\"color:blue;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
          // Handle non-keyword identifiers
          strcpy(res_p, escaped_token);
          res_p += strlen(escaped_token);
        }
      } else if (isdigit(*p)) {
        // Handle numbers
        int token_index = 0;
        while (isdigit(*p) || *p == '.') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p;
          }
          p++;
        }
        token[token_index] = '\0'; // Null-terminate the number

        char escaped_token[1024];
        escape_html_chars(token, escaped_token);

        // Highlight numbers
        int n = sprintf(res_p, "<span style=\"color:green;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (*p == '\n') {
        // Replace newline with <br>
        p++;
        strcpy(res_p, "<br>");
        res_p += 4;
      } else {
        // Handle other characters
        char temp[2];
        temp[0] = *p++;
        temp[1] = '\0';

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any open spans (for strings or comments)
  if (in_string || in_comment_single || in_comment_multi) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the result
  return result;
}
