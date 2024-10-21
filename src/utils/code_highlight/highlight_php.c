#include "../../include/code_highlight/highlight_php.h"
#include "../../include/code_highlight/escape_code.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// List of PHP keywords
const char *php_keywords[] = {
    "abstract",  "and",        "array",        "as",           "break",
    "callable",  "case",       "catch",        "class",        "clone",
    "const",     "continue",   "declare",      "default",      "do",
    "echo",      "else",       "elseif",       "empty",        "enddeclare",
    "endfor",    "endforeach", "endif",        "endswitch",    "endwhile",
    "eval",      "exit",       "extends",      "final",        "finally",
    "for",       "foreach",    "function",     "global",       "goto",
    "if",        "implements", "include",      "include_once", "instanceof",
    "insteadof", "interface",  "isset",        "list",         "namespace",
    "new",       "or",         "print",        "private",      "protected",
    "public",    "require",    "require_once", "return",       "static",
    "switch",    "throw",      "trait",        "try",          "unset",
    "use",       "var",        "while",        "xor",          "yield",
    NULL};

// Function to check if a word is a PHP keyword
int is_php_keyword(const char *word) {
  for (int i = 0; php_keywords[i]; i++) {
    if (strcmp(word, php_keywords[i]) == 0)
      return 1;
  }
  return 0;
}

// Function to highlight PHP code by adding HTML tags
char *highlight_php(const char *code) {
  size_t result_size = strlen(code) * 20; // Estimated result size
  char *result = malloc(result_size);
  if (!result) {
    fprintf(stderr, "Memory allocation error\n");
    exit(1);
  }

  const char *p = code; // Pointer to traverse the input code
  char *res_p = result; // Pointer to build the result
  char token[256];      // Buffer to store tokens (e.g., keywords, variables)
  int in_string = 0; // 0 = not in string, 1 = in single-quoted string, 2 = in
                     // double-quoted string
  int in_comment_single = 0; // 1 = in single-line comment
  int in_comment_multi = 0;  // 1 = in multi-line comment
  int in_variable = 0;       // 1 = reading a variable (starting with $)

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
                 (in_string == 2 && *p == '"')) {
        // Handle closing of string
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
      } else {
        // Regular characters inside string
        char temp[2] = {*p++, '\0'};

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

        // Replace newline with <br> in HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;

        in_comment_single = 0; // Exit single-line comment mode
      } else {
        // Regular characters inside comment
        char temp[2] = {*p++, '\0'};

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
        // Replace newline with <br> in HTML
        strcpy(res_p, "<br>");
        res_p += 4;
        p++;
      } else {
        // Regular characters inside comment
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    } else {
      // Handle code outside of strings and comments
      if ((*p == '/' && *(p + 1) == '/') || (*p == '#' && !in_string)) {
        // Start of a single-line comment
        in_comment_single = 1;

        // Start comment span
        const char *open_span = "<span style=\"color:green;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};
        if (temp[0] == '/' && *p == '/') {
          temp[1] = *p++;
          temp[2] = '\0';
        }

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

        char escaped[20];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '\'' || *p == '"') {
        // Start of a string
        in_string = (*p == '\'') ? 1 : 2;

        // Start string span
        const char *open_span = "<span style=\"color:orange;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      } else if (*p == '$' && isalpha(*(p + 1))) {
        // Handle PHP variable
        in_variable = 1;

        // Start variable span
        const char *open_span = "<span style=\"color:purple;\">";
        strcpy(res_p, open_span);
        res_p += strlen(open_span);

        char temp[2] = {*p++, '\0'};

        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);

        // Read the variable name
        int token_index = 0;
        while (isalnum(*p) || *p == '_') {
          if ((size_t)token_index < sizeof(token) - 1) {
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

        // Close variable span
        const char *close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);

        in_variable = 0;
      } else if (isalpha(*p) || *p == '_') {
        // Handle identifiers (keywords or variables)
        int token_index = 0;

        while (isalnum(*p) || *p == '_') {
          if ((size_t)token_index < sizeof(token) - 1) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';

        char escaped_token[1024];
        escape_html_chars(token, escaped_token);

        if (is_php_keyword(token)) {
          // Highlight PHP keywords
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

        while (isdigit(*p) || *p == '.' || *p == 'x' || *p == 'X' ||
               (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
          if ((size_t)token_index < sizeof(token) - 1) {
            token[token_index++] = *p++;
          } else {
            p++;
          }
        }
        token[token_index] = '\0';

        char escaped_token[1024];
        escape_html_chars(token, escaped_token);

        // Highlight numbers
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
        // Handle other characters
        char temp[2] = {*p++, '\0'};
        char escaped[10];
        escape_html_chars(temp, escaped);
        strcpy(res_p, escaped);
        res_p += strlen(escaped);
      }
    }
  }

  // Close any open spans (for strings, comments, or variables)
  if (in_string || in_comment_single || in_comment_multi || in_variable) {
    const char *close_span = "</span>";
    strcpy(res_p, close_span);
    res_p += strlen(close_span);
  }

  *res_p = '\0'; // Null-terminate the result
  return result; // Return the highlighted PHP code
}
