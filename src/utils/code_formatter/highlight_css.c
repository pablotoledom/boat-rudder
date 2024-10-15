#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_css.h"
#include "../../include/code_formatter/escape_code.h"


#define INITIAL_BUFFER_SIZE 1024

void append_to_result(char** res_p, size_t* res_size, size_t* res_capacity, const char* str) {
    size_t len = strlen(str);
    if (*res_size + len >= *res_capacity) {
        // Aumentar el tamaño del buffer
        *res_capacity *= 2;
        size_t offset = *res_p - *res_p;
        *res_p = realloc(*res_p, *res_capacity);
        if (!*res_p) {
            fprintf(stderr, "Error al reasignar memoria\n");
            exit(1);
        }
        *res_p += offset;
    }
    strcpy(*res_p + *res_size, str);
    *res_size += len;
}

char* highlight_css(const char* code) {
    size_t res_capacity = INITIAL_BUFFER_SIZE;
    size_t res_size = 0;
    char* result = malloc(res_capacity);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }
    result[0] = '\0'; // Inicializar la cadena

    const char* p = code;
    int in_comment = 0;
    int in_selector = 1;

    while (*p) {
        if (in_comment) {
            if (*p == '*' && *(p + 1) == '/') {
                char temp[3] = { *p++, *p++, '\0' };

                char escaped[20];
                escape_and_replace_spaces(temp, escaped);

                append_to_result(&result, &res_size, &res_capacity, escaped);

                // Cerrar el span de comentario
                const char* close_span = "</span>";
                append_to_result(&result, &res_size, &res_capacity, close_span);

                in_comment = 0;
            } else {
                if (*p == '\n') {
                    // Reemplazar salto de línea por <br>
                    append_to_result(&result, &res_size, &res_capacity, "<br>");
                    p++;
                } else {
                    char temp[2] = { *p++, '\0' };
                    char escaped[10];
                    escape_and_replace_spaces(temp, escaped);
                    append_to_result(&result, &res_size, &res_capacity, escaped);
                }
            }
        } else {
            if (*p == '/' && *(p + 1) == '*') {
                in_comment = 1;

                // Iniciar span para comentario
                const char* open_span = "<span style=\"color:green;\">";
                append_to_result(&result, &res_size, &res_capacity, open_span);

                char temp[3] = { *p++, *p++, '\0' };
                char escaped[20];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else if (*p == '{') {
                in_selector = 0;

                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else if (*p == '}') {
                in_selector = 1;

                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else if (in_selector && (*p != '\n')) {
                // Resaltar selectores
                char selector[256];
                int selector_index = 0;
                while (*p && *p != '{' && *p != '\n') {
                    if ((size_t)selector_index < sizeof(selector) - 1) {
                        selector[selector_index++] = *p++;
                    } else {
                        break; // Evitar desbordamiento
                    }
                }
                selector[selector_index] = '\0';

                char escaped_selector[1024];
                escape_and_replace_spaces(selector, escaped_selector);

                char buffer[2048];
                snprintf(buffer, sizeof(buffer), "<span style=\"color:#55FFFF;\">%s</span>", escaped_selector);
                append_to_result(&result, &res_size, &res_capacity, buffer);
            } else if (isalpha(*p) || *p == '-') {
                // Resaltar propiedades
                char property[256];
                int prop_index = 0;

                while (isalnum(*p) || *p == '-' || *p == '_') {
                    if ((size_t)prop_index < sizeof(property) - 1) {
                        property[prop_index++] = *p++;
                    } else {
                        break; // Evitar desbordamiento
                    }
                }
                property[prop_index] = '\0';

                char escaped_property[1024];
                escape_and_replace_spaces(property, escaped_property);

                char buffer[2048];
                snprintf(buffer, sizeof(buffer), "<span style=\"color:red;\">%s</span>", escaped_property);
                append_to_result(&result, &res_size, &res_capacity, buffer);
            } else if (*p == ':') {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else if (*p == ';') {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else if (*p == '\n') {
                // Reemplazar salto de línea por <br>
                append_to_result(&result, &res_size, &res_capacity, "<br>");
                p++;
            } else if (*p == ' ' || *p == '\t') {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            } else {
                char temp[2] = { *p++, '\0' };
                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                append_to_result(&result, &res_size, &res_capacity, escaped);
            }
        }
    }

    // Cerrar spans abiertos
    if (in_comment) {
        const char* close_span = "</span>";
        append_to_result(&result, &res_size, &res_capacity, close_span);
    }

    result[res_size] = '\0'; // Asegurarse de terminar la cadena
    return result;
}
