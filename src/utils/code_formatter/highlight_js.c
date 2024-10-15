// highlight_js.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_js.h"
#include "../../include/code_formatter/escape_code.h"

const char* js_keywords[] = {
    // Lista de palabras clave de JavaScript
    "break", "case", "catch", "class", "const", "continue", "debugger", "default",
    "delete", "do", "else", "export", "extends", "finally", "for", "function",
    "if", "import", "in", "instanceof", "let", "new", "return", "super",
    "switch", "this", "throw", "try", "typeof", "var", "void", "while", "with", "yield",
    "null", "true", "false", "await", NULL
};

int is_js_keyword(const char* word) {
    for (int i = 0; js_keywords[i]; i++) {
        if (strcmp(word, js_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

char* highlight_js(const char* code) {
    size_t result_size = strlen(code) * 20; // Tamaño estimado
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    const char* p = code;
    char* res_p = result;
    char token[256];
    int in_string = 0;         // 0 = no en cadena, 1 = en cadena simple, 2 = en cadena doble
    int in_comment_single = 0; // Comentario de una línea
    int in_comment_multi = 0;  // Comentario de múltiples líneas

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
            } else if ((in_string == 1 && *p == '\'') || (in_string == 2 && *p == '\"')) {
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
        } else if (in_comment_single) {
            if (*p == '\n') {
                // Cerrar el span de comentario
                const char* close_span = "</span>";
                strcpy(res_p, close_span);
                res_p += strlen(close_span);

                // Avanzar el puntero
                p++;

                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;

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
                // Avanzar el puntero
                p++;

                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
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
            if (*p == '\'' || *p == '\"') {
                in_string = (*p == '\'') ? 1 : 2;

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
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p;
                    }
                    p++;
                }
                token[token_index] = '\0';

                char escaped_token[1024];
                escape_and_replace_spaces(token, escaped_token);

                if (is_js_keyword(token)) {
                    int n = sprintf(res_p, "<span style=\"color:blue;\">%s</span>", escaped_token);
                    res_p += n;
                } else {
                    strcpy(res_p, escaped_token);
                    res_p += strlen(escaped_token);
                }
            } else if (isdigit(*p)) {
                int token_index = 0;
                while (isdigit(*p) || *p == '.') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p;
                    }
                    p++;
                }
                token[token_index] = '\0';

                char escaped_token[1024];
                escape_and_replace_spaces(token, escaped_token);

                int n = sprintf(res_p, "<span style=\"color:green;\">%s</span>", escaped_token);
                res_p += n;
            } else if (*p == '\n') {
                // Avanzar el puntero
                p++;

                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
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
    if (in_string || in_comment_single || in_comment_multi) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }

    *res_p = '\0';
    return result;
}
