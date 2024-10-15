#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_c.h"
#include "../../include/code_formatter/escape_code.h"

const char* c_keywords[] = {
    // Lista de palabras clave de C
    "int", "float", "double", "char", "void", "if", "else", "for", "while",
    "do", "switch", "case", "break", "continue", "return", "struct", "typedef",
    "enum", "union", "const", "static", "extern", "unsigned", "signed", "sizeof",
    "NULL", "volatile", "register", "goto", "long", "short", "default", "auto", NULL
};

int is_c_keyword(const char* word) {
    for (int i = 0; c_keywords[i]; i++) {
        if (strcmp(word, c_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

char* highlight_c(const char* code) {
    size_t result_size = strlen(code) * 20; // Tamaño estimado
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    const char* p = code;
    char* res_p = result;
    char token[256];
    int in_string = 0; // 0 = no en cadena, 1 = en cadena doble
    int in_char = 0;   // 0 = no en carácter, 1 = en carácter
    int in_comment_single = 0;
    int in_comment_multi = 0;

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
            } else if (*p == '\"') {
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
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else if (in_char) {
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
            } else if (*p == '\'') {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);

                // Cerrar el span de carácter
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                in_char = 0;
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else if (in_comment_single) {
            if (*p == '\n') {
                // Cerrar el span de comentario
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;

                in_comment_single = 0;
            } else {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            }
        } else if (in_comment_multi) {
            if (*p == '*' && *(p + 1) == '/') {
                char temp[3];
                temp[0] = *p++;
                temp[1] = *p++;
                temp[2] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);

                // Cerrar el span de comentario
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                in_comment_multi = 0;
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
        } else {
            if (*p == '\"') {
                in_string = 1;

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
            } else if (*p == '\'') {
                in_char = 1;

                // Iniciar span para carácter
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
            } else if (*p == '/' && *(p + 1) == '/') {
                in_comment_single = 1;

                // Iniciar span para comentario
                const char* open_span = "<span style=\"color:green;\">";
                strcpy(res_p, open_span);
                res_p += strlen(open_span);

                char temp[3];
                temp[0] = *p++;
                temp[1] = *p++;
                temp[2] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else if (*p == '/' && *(p + 1) == '*') {
                in_comment_multi = 1;

                // Iniciar span para comentario
                const char* open_span = "<span style=\"color:green;\">";
                strcpy(res_p, open_span);
                res_p += strlen(open_span);

                char temp[3];
                temp[0] = *p++;
                temp[1] = *p++;
                temp[2] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else if (isalpha(*p) || *p == '_') {
                int token_index = 0;
                
                while (isalnum(*p) || *p == '_') {
                    if ((size_t)token_index < sizeof(token) - 1) {
                        token[token_index++] = *p;
                    }
                    p++;
                }
                token[token_index] = '\0';

                char escaped_token[1024];
                escape_and_replace_spaces(token, escaped_token);

                if (is_c_keyword(token)) {
                    int n = sprintf(res_p, "<span style=\"color:#55FFFF;\">%s</span>", escaped_token);
                    res_p += n;
                } else {
                    strcpy(res_p, escaped_token);
                    res_p += strlen(escaped_token);
                }
            } else if (isdigit(*p)) {
                int token_index = 0;
                
                while (isdigit(*p) || *p == '.' || *p == 'x' || *p == 'X' || (*p >= 'a' && *p <= 'f') || (*p >= 'A' && *p <= 'F')) {
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
    if (in_string || in_char || in_comment_single || in_comment_multi) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }

    *res_p = '\0';
    return result;
}
