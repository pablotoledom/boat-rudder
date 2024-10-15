#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_python.h"
#include "../../include/code_formatter/escape_code.h"

const char* python_keywords[] = {
    "def", "class", "if", "elif", "else", "for", "while", "break", "continue",
    "return", "import", "from", "as", "pass", "None", "True", "False", "and",
    "or", "not", "in", "is", "lambda", "with", "yield", "try", "except", "finally",
    "global", "nonlocal", "assert", "del", "async", "await", "raise", "print", NULL
};

int is_python_keyword(const char* word) {
    for (int i = 0; python_keywords[i]; i++) {
        if (strcmp(word, python_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

char* highlight_python(const char* code) {
    size_t result_size = strlen(code) * 20; // Tamaño estimado
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    const char* p = code;
    char* res_p = result;
    char token[256];
    int in_string = 0; // 0 = no en cadena, 1 = en cadena simple ('), 2 = en cadena doble ("), 3 = cadena triple (''' o """)
    int string_quote = 0;
    int in_comment = 0;

    while (*p) {
        if (in_string) {
            if (*p == '\\') {
                // Caracter de escape
                char temp[3];
                temp[0] = *p++;
                temp[1] = *p ? *p++ : '\0';
                temp[2] = '\0';

                char escaped[20];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else if ((*p == string_quote && in_string == 1) || (*p == string_quote && in_string == 2)) {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);

                // Cerrar el span de cadena
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                in_string = 0;
            } else if (*p == '\n') {
                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else if (in_comment) {
            if (*p == '\n') {
                // Cerrar el span de comentario
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;

                in_comment = 0;
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else {
            if (*p == '#' || (*p == '/' && *(p + 1) == '/')) {
                in_comment = 1;

                // Iniciar span para comentario
                const char* open_span = "<span style=\"color:green;\">";
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
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else if (*p == '\'' || *p == '\"') {
                in_string = 1;
                string_quote = *p;

                // Iniciar span para cadena
                const char* open_span = "<span style=\"color:orange;\">";
                strcpy(res_p, open_span);
                res_p += strlen(open_span);

                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else if (isalpha(*p) || *p == '_') {
                int token_index = 0;
                const char* start_p = p;
                while (isalnum(*p) || *p == '_') {
                    if ((size_t)token_index < sizeof(token) - 1) {
                        token[token_index++] = *p;
                    }
                    p++;
                }
                token[token_index] = '\0';

                char escaped_token[1024];
                escape_and_replace_spaces(token, escaped_token);

                if (is_python_keyword(token)) {
                    int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>", escaped_token);
                    res_p += n;
                } else {
                    strcpy(res_p, escaped_token);
                    res_p += strlen(escaped_token);
                }
            } else if (isdigit(*p)) {
                int token_index = 0;
                const char* start_p = p;
                while (isdigit(*p) || *p == '.') {
                    if ((size_t)token_index < sizeof(token) - 1) {
                        token[token_index++] = *p;
                    }
                    p++;
                }
                token[token_index] = '\0';

                char escaped_token[1024];
                escape_and_replace_spaces(token, escaped_token);

                int n = sprintf(res_p, "<span style=\"color:purple;\">%s</span>", escaped_token);
                res_p += n;
            } else if (*p == '\n') {
                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;
            } else if (*p == ' ' || *p == '\t') {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        }
    }

    // Cerrar spans abiertos
    if (in_string || in_comment) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }

    *res_p = '\0';
    return result;
}
