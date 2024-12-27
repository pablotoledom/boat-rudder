#include "../../include/code_highlight/highlight_bash.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of Bash keywords
const char *bash_keywords[] = {
    "if",    "then",  "else",     "fi",       "for",     "while", "do",
    "done",  "case",  "esac",     "function", "return",  "in",    "echo",
    "exit",  "break", "continue", "read",     "declare", "local", "export",
    "unset", "trap",  "shift",    NULL};

// Function to check if a word is a Bash keyword
int is_bash_keyword(const char *word) {
  for (int i = 0; bash_keywords[i]; i++) {
    if (strcmp(word, bash_keywords[i]) == 0)
      return 1;
  }
  return 0;
}

// Function to highlight Bash code by adding HTML tags
char *highlight_bash(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the result HTML
  char token[256];      // Buffer to store tokens (e.g., keywords)
  int in_string = 0;    // 1 = single-quoted string, 2 = double-quoted string
  int in_comment = 0;   // 1 = in a comment (starts with #)

  // Main loop to process each character in the input code
  while (*p) {
    if (in_string) {
      if (*p == '\\') {
        // Handle escape sequences
        char temp[3];
        temp[0] = *p++;
        temp[1] = *p ? *p++ : '\0'; // Check for null terminator
        temp[2] = '\0';
        char escaped[20];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if ((*p == '\'' && in_string == 1) ||
                 (*p == '"' && in_string == 2)) {
        // Close string span
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
        in_string = 0; // Exit string mode
      } else {
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
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
        in_comment = 0;
      } else {
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      if (*p == '#') {
        // Start of a comment
        in_comment = 1;
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'' || *p == '"') {
        // Start of a string
        in_string = (*p == '\'') ? 1 : 2;
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '$' && isalpha(*(p + 1))) {
        // Handle variables
        const char *open_span = "<span style=\"color:purple;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
        // Read variable name
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';
        char escaped_token[1024];
        escape_html_chars(token, escaped_token);
        strcpy(res_p, escaped_token);
        res_p += strlen(escaped_token);
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
      } else if (isalpha(*p) || *p == '_') {
        // Handle keywords or commands
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';
        char escaped_token[1024];
        escape_html_chars(token, escaped_token);
        if (is_bash_keyword(token)) {
          int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
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
        char escaped_token[1024];
        escape_html_chars(token, escaped_token);
        int n = sprintf(res_p, "<span style=\"color:brown;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (*p == '\n') {
        // Replace newline with <br>
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Handle other characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any open spans
  if (in_string || in_comment) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0';
  return result;
}
