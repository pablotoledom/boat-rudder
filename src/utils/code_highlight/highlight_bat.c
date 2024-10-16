#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_highlight/highlight_bat.h"
#include "../../include/code_highlight/escape_code.h"

// List of Batch keywords
const char* bat_keywords[] = {
    "echo", "set", "if", "else", "goto", "call", "exit", "for", "in", "do", "pause", "rem", NULL
};

// Function to compare strings without case sensitivity
int strncasecmp_custom(const char *s1, const char *s2, size_t n) {
    while (n && *s1 && (tolower((unsigned char)*s1) == tolower((unsigned char)*s2))) {
        s1++;
        s2++;
        n--;
    }
    return n ? (tolower((unsigned char)*s1) - tolower((unsigned char)*s2)) : 0;
}

// Function to check if a word is a Batch keyword
int is_bat_keyword(const char* word) {
    for (int i = 0; bat_keywords[i]; i++) {
        if (strcmp(word, bat_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

// Function to highlight Batch code by adding HTML tags
char* highlight_bat(const char* code) {
    size_t result_size = strlen(code) * 20; // Estimated result size
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Memory allocation error\n");
        exit(1);
    }

    const char* p = code;  // Pointer to traverse the input code
    char* res_p = result;  // Pointer to build the result HTML
    char token[256];       // Buffer to store tokens (e.g., keywords)
    int in_comment = 0;    // 1 = in a comment (starts with REM or @rem)

    // Main loop to process each character in the input code
    while (*p) {
        if (in_comment) {
            if (*p == '\n') {
                // Close comment span
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;
                in_comment = 0;
            } else {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_html_chars(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else {
            if (*p == '@' || strncasecmp_custom(p, "rem", 3) == 0) {
                // Start of a comment
                in_comment = 1;
                const char* open_span = "<span style=\"color:green;\">";
                strcpy(res_p, open_span);
                res_p += strlen(open_span);
                if (*p == '@') {
                    char temp[2] = { *p++, '\0' };
                    char escaped[10];
                    escape_html_chars(temp, escaped);
                    strcpy(res_p, escaped);
                    res_p += strlen(escaped);
                } else {
                    for (int i = 0; i < 3; i++) {
                        char temp[2] = { *p++, '\0' };
                        char escaped[10];
                        escape_html_chars(temp, escaped);
                        strcpy(res_p, escaped);
                        res_p += strlen(escaped);
                    }
                }
            } else if (*p == '%' && *(p + 1) != '\0') {
                // Handle variables (e.g., %VAR%)
                const char* open_span = "<span style=\"color:purple;\">";
                strcpy(res_p, open_span);
                res_p += strlen(open_span);
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_html_chars(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
                // Read variable
                int token_index = 0;
                while (*p && *p != '%') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    }
                }
                token[token_index] = '\0';
                if (*p == '%') {
                    temp[0] = *p++;
                    temp[1] = '\0';
                    escape_html_chars(temp, escaped);
                    strcpy(res_p, escaped);
                    res_p += strlen(escaped);
                }
                char escaped_token[1024];
                escape_html_chars(token, escaped_token);
                strcpy(res_p, escaped_token);
                res_p += strlen(escaped_token);
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);
            } else if (isalpha(*p)) {
                // Handle keywords or commands
                int token_index = 0;
                while (isalnum(*p) || *p == '_') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    } else {
                        p++;
                    }
                }
                token[token_index] = '\0';
                char escaped_token[1024];
                escape_html_chars(token, escaped_token);
                if (is_bat_keyword(token)) {
                    int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>", escaped_token);
                    res_p += n;
                } else {
                    strcpy(res_p, escaped_token);
                    res_p += strlen(escaped_token);
                }
            } else if (isdigit(*p)) {
                // Handle numbers
                int token_index = 0;
                while (isdigit(*p) || *p == '.') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    } else {
                        p++;
                    }
                }
                token[token_index] = '\0';
                char escaped_token[1024];
                escape_html_chars(token, escaped_token);
                int n = sprintf(res_p, "<span style=\"color:brown;\">%s</span>", escaped_token);
                res_p += n;
            } else if (*p == '\n') {
                // Replace newline with <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;
            } else {
                // Handle other characters
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_html_chars(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        }
    }

    // Close any open spans
    if (in_comment) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }

    *res_p = '\0';
    return result;
}
