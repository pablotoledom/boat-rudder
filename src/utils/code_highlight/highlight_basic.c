#include "../../include/code_highlight/highlight_basic.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>

// List of BASIC keywords
const char *basic_keywords[] = {
    "PRINT",  "INPUT", "LET",   "IF",   "THEN",     "ELSE", "ENDIF", "FOR",
    "TO",     "NEXT",  "WHILE", "WEND", "DO",       "LOOP", "GOTO",  "GOSUB",
    "RETURN", "DIM",   "REM",   "END",  "FUNCTION", "SUB",  "CALL",  "SELECT",
    "CASE",   "CONST", "EXIT",  "MOD",  NULL}; // NULL marks the end of the list

// Function to check if a word is a BASIC keyword (case insensitive)
int is_basic_keyword(const char *word) {
  for (int i = 0; basic_keywords[i]; i++) {
    if (strcasecmp(word, basic_keywords[i]) == 0) // Compare case-insensitively
      return 1;                                   // Return 1 if it's a keyword
  }
  return 0; // Return 0 if it's not a keyword
}

// Function to highlight BASIC code in HTML format
char *highlight_basic(const char *code) {
  size_t result_size =
      strlen(code) * 20; // Estimated size for the result buffer
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr,
            "Memory allocation error\n"); // Error handling if malloc fails
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to construct the result HTML
  char token[256];      // Buffer for holding words or tokens
  int in_string = 0;    // 0 = not inside a string, 1 = inside a string (")
  int in_comment = 0;   // 0 = not in comment, 1 = in comment (REM or ')
  int line_start = 1;   // 1 = start of a line, helps detect REM comments

  // Main loop to process each character in the input code
  while (*p) {
    if (in_string) {
      // Handle characters inside a string literal
      if (*p == '\"') {
        char temp[2] = {*p++, '\0'}; // Capture and escape closing quote
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Close the string's HTML span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_string = 0; // Exit string mode
      } else {
        // Escape and append characters inside the string
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else if (in_comment) {
      // Handle comment text
      if (*p == '\n') {
        // Close the comment's HTML span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        // Replace newline with <br> in HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;

        in_comment = 0; // Exit comment mode
        line_start = 1; // Reset line start for the next line
      } else {
        // Escape and append comment characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      // Handle code outside strings and comments
      if (*p == '\"') {
        // Enter string mode and start HTML span
        in_string = 1;
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        // Escape and append the starting quote
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if ((*p == '\'' ||
                  (line_start && strncasecmp(p, "REM", 3) == 0))) {
        // Enter comment mode and start HTML span
        in_comment = 1;
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        if (*p == '\'') {
          // Escape and append single-quote comment
          char temp[2] = {*p++, '\0'};
          char escaped[10];
          escape_html_chars(temp, escaped);
          strcpy(res_p, escaped);
          res_p += strlen(escaped);
        } else {
          // Append "REM" for REM-style comment
          for (int i = 0; i < 3; i++) {
            char temp[2] = {*p++, '\0'};
            char escaped[10];
            escape_html_chars(temp, escaped);
            strcpy(res_p, escaped);
            res_p += strlen(escaped);
          }
        }
      } else if (isalpha(*p)) {
        // Handle identifiers (keywords or variables)
        int token_index = 0;

        while (isalnum(*p) || *p == '_') {
          if (token_index < (int)(sizeof(token) - 1)) {
            token[token_index++] = *p++;
          } else {
            p++; // Skip overlong tokens
          }
        }
        token[token_index] = '\0'; // Null-terminate the token

        char escaped_token[1024];
        escape_html_chars(token, escaped_token);

        if (is_basic_keyword(token)) {
          // Highlight keywords in blue
          int n = sprintf(res_p, "<span style=\"color:blue;\">%s</span>",
                          escaped_token);
          res_p += n;
        } else {
          // Append non-keyword identifiers as-is
          strcpy(res_p, escaped_token);
          res_p += strlen(escaped_token);
        }

        line_start = 0; // We are no longer at the start of a line
      } else if (isdigit(*p)) {
        // Handle numeric literals
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

        // Highlight numbers in brown
        int n = sprintf(res_p, "<span style=\"color:brown;\">%s</span>",
                        escaped_token);
        res_p += n;
      } else if (*p == '\n') {
        // Replace newlines with <br> for HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
        line_start = 1; // Mark the start of a new line
      } else if (*p == ' ' || *p == '\t') {
        // Escape and append spaces or tabs
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else {
        // Escape and append other characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
        line_start = 0; // Not at the start of a line anymore
      }
    }
  }

  // Close any remaining open spans (if string or comment was not closed)
  if (in_string || in_comment) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the final result
  return result; // Return the highlighted HTML code
}
