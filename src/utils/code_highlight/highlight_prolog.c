#include "../../include/code_highlight/highlight_prolog.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of Prolog keywords
const char *prolog_keywords[] = {
    "is",       "not",     "true",      "false",   "fail",    "repeat",
    "mod",      "div",     "assert",    "retract", "clause",  "call",
    "once",     "findall", "bagof",     "setof",   "consult", "write",
    "read",     "nl",      "halt",      "dynamic", "static",  "spy",
    "no_trace", "trace",   "var",       "nonvar",  "number",  "atom",
    "functor",  "arg",     "copy_term", "=",       "\\=",     "==",
    "\\==",     "@<",      "@=<",       "@>",      "@>=",     "<",
    "=<",       ">",       ">=",        ":-",      "-->",     NULL};

// Function to check if a word is a Prolog keyword
int is_prolog_keyword(const char *word) {
  for (int i = 0; prolog_keywords[i]; i++) {
    if (strcmp(word, prolog_keywords[i]) == 0)
      return 1;
  }
  return 0;
}

// Function to highlight Prolog code by adding HTML tags
char *highlight_prolog(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the result HTML
  char token[256];      // Buffer to store tokens (e.g., variables, atoms)
  int in_string = 0;    // 0 = not in string, 1 = in single-quoted string (')
  int in_comment = 0;   // 1 = in comment (starts with %)

  // Main loop to process each character in the input code
  while (*p) {
    if (in_string) {
      if (*p == '\'') {
        // End of a string
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
      } else if (*p == '\\' && (*(p + 1) == '\'' || *(p + 1) == '\\')) {
        // Handle escape sequences inside string
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p++;
        temp[2] = '\0';

        char escaped[20];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else {
        // Regular characters inside string
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment) {
      if (*p == '\n') {
        // End of a comment
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        // Replace newline with <br>
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;

        in_comment = 0; // Exit comment mode
      } else {
        // Regular characters inside comment
        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      if (*p == '%') {
        // Start of a comment
        in_comment = 1;

        // Start comment span
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'') {
        // Start of a string
        in_string = 1;

        // Start string span
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (isupper(*p) || *p == '_') {
        // Variables (start with uppercase letter or _)
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';

        char escaped_token[256];
        escape_html_chars(token, escaped_token);

        int n = sprintf(res_p, "<span style=\"color:purple;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (islower(*p)) {
        // Atoms or predicates (start with lowercase letter)
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';

        char escaped_token[256];
        escape_html_chars(token, escaped_token);

        if (is_prolog_keyword(token)) {
          // Highlight Prolog keywords
          int n = sprintf(res_p, "<span style=\"color:blue;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
          // Regular atoms or predicates
          strcpy(res_p, escaped_token);
          res_p += strlen(escaped_token);
        }
      } else if (isdigit(*p)) {
        // Handle numbers
        int token_index = 0;
        while (isdigit(*p) || *p == '.') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';

        char escaped_token[256];
        escape_html_chars(token, escaped_token);

        int n = sprintf(res_p, "<span style=\"color:brown;\">%s</span>",
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
        // Operators and other characters
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
  return result; // Return the highlighted Prolog code
}
