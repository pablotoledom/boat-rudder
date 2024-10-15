// escape_code.c

#include <string.h>
#include "../../include/code_formatter/escape_code.h"

void escape_and_replace_spaces(const char* input, char* output) {
    while (*input) {
        if (*input == ' ') {
            strcpy(output, "&nbsp;");
            output += 6;
            input++;
        } else if (*input == '&') {
            strcpy(output, "&amp;");
            output += 5;
            input++;
        } else if (*input == '<') {
            strcpy(output, "&lt;");
            output += 4;
            input++;
        } else if (*input == '>') {
            strcpy(output, "&gt;");
            output += 4;
            input++;
        } else {
            *output++ = *input++;
        }
    }
    *output = '\0';
}
