// highlight_prolog.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_prolog.h"
#include "../../include/code_formatter/escape_code.h"

const char* prolog_keywords[] = {
    // Lista de palabras clave de Prolog
    "is", "not", "true", "false", "fail", "repeat", "mod", "div",
    "assert", "retract", "clause", "call", "once", "findall", "bagof", "setof",
    "consult", "write", "read", "nl", "halt", "dynamic", "static", "spy",
    "no_trace", "trace", "var", "nonvar", "number", "atom", "functor", "arg",
    "copy_term", "=", "\\=", "==", "\\==", "@<", "@=<", "@>", "@>=", "<", "=<", ">", ">=",
    ":-", "-->", NULL
};

int is_prolog_keyword(const char* word) {
    for (int i = 0; prolog_keywords[i]; i++) {
        if (strcmp(word, prolog_keywords[i]) == 0)
            return 1;
    }
    return 0;
}

char* highlight_prolog(const char* code) {
    size_t result_size = strlen(code) * 20; // Tamaño estimado
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    const char* p = code;
    char* res_p = result;
    char token[256];
    int in_string = 0;   // 0 = no en cadena, 1 = en cadena simple (')
    int in_comment = 0;  // Comentario iniciado con %

    while (*p) {
        if (in_string) {
            if (*p == '\'') {
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
            } else if (*p == '\\' && (*(p + 1) == '\'' || *(p + 1) == '\\')) {
                // Caracter de escape
                char temp[3];
                temp[0] = *p++;
                temp[1] = *p++;
                temp[2] = '\0';

                char escaped[20];
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
            if (*p == '%') {
                in_comment = 1;

                // Iniciar span para comentario
                const char* open_span = "<span style=\"color:green;\">";
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
            } else if (isupper(*p) || *p == '_') {
                // Variables (inician con mayúscula o '_')
                int token_index = 0;
                while (isalnum(*p) || *p == '_') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    } else {
                        p++;
                    }
                }
                token[token_index] = '\0';

                char escaped_token[256];
                escape_and_replace_spaces(token, escaped_token);

                int n = sprintf(res_p, "<span style=\"color:purple;\">%s</span>", escaped_token);
                res_p += n;
            } else if (islower(*p)) {
                // Átomos o predicados
                int token_index = 0;
                while (isalnum(*p) || *p == '_') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    } else {
                        p++;
                    }
                }
                token[token_index] = '\0';

                char escaped_token[256];
                escape_and_replace_spaces(token, escaped_token);

                if (is_prolog_keyword(token)) {
                    int n = sprintf(res_p, "<span style=\"color:blue;\">%s</span>", escaped_token);
                    res_p += n;
                } else {
                    strcpy(res_p, escaped_token);
                    res_p += strlen(escaped_token);
                }
            } else if (isdigit(*p)) {
                // Manejo de números
                int token_index = 0;
                while (isdigit(*p) || *p == '.') {
                    if (token_index < sizeof(token) - 1) {
                        token[token_index++] = *p++;
                    } else {
                        p++;
                    }
                }
                token[token_index] = '\0';

                char escaped_token[256];
                escape_and_replace_spaces(token, escaped_token);

                int n = sprintf(res_p, "<span style=\"color:brown;\">%s</span>", escaped_token);
                res_p += n;
            } else if (*p == '\n') {
                // Reemplazar salto de línea por <br>
                strcpy(res_p, "<br>");
                res_p += 4;
                p++;
            } else if (*p == ' ' || *p == '\t') {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);
            } else {
                // Operadores y otros caracteres
                char temp[2] = { *p++, '\0' };
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
