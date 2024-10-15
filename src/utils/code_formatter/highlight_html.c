// highlight_html.c

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "../../include/code_formatter/highlight_html.h"
#include "../../include/code_formatter/escape_code.h"

char* highlight_html(const char* code) {
    size_t result_size = strlen(code) * 10; // Tamaño estimado
    char* result = malloc(result_size);
    if (!result) {
        fprintf(stderr, "Error al asignar memoria\n");
        exit(1);
    }

    const char* p = code;
    char* res_p = result;

    int in_tag = 0;
    int in_attr_name = 0;
    int in_attr_value = 0;
    int attr_value_quote = 0;

    while (*p) {
        if (*p == '<') {
            in_tag = 1;
            // Añadir <span para la etiqueta
            const char* open_span = "<span style=\"color:#55FFFF;\">";
            strcpy(res_p, open_span);
            res_p += strlen(open_span);

            char temp[2];
            temp[0] = *p++;
            temp[1] = '\0';

            char escaped[10];
            escape_and_replace_spaces(temp, escaped);
            strcpy(res_p, escaped);
            res_p += strlen(escaped);
        } else if (*p == '>') {
            char temp[2];
            temp[0] = *p++;
            temp[1] = '\0';

            char escaped[10];
            escape_and_replace_spaces(temp, escaped);
            strcpy(res_p, escaped);
            res_p += strlen(escaped);

            // Cerrar </span> de la etiqueta
            const char* close_span = "</span>";
            strcpy(res_p, close_span);
            res_p += strlen(close_span);

            in_tag = 0;
            in_attr_name = 0;
            in_attr_value = 0;
            attr_value_quote = 0;
        } else if (in_tag) {
            if (isspace(*p)) {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);

                if (!in_attr_value) {
                    in_attr_name = 1;
                }
            } else if (*p == '=') {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);
                strcpy(res_p, escaped);
                res_p += strlen(escaped);

                in_attr_name = 0;
                in_attr_value = 1;
            } else if (*p == '\'' || *p == '\"') {
                char temp[2];
                temp[0] = *p++;
                temp[1] = '\0';

                char escaped[10];
                escape_and_replace_spaces(temp, escaped);

                if (in_attr_value) {
                    if (attr_value_quote == 0) {
                        // Inicio del valor del atributo
                        attr_value_quote = temp[0];

                        // Añadir <span para el valor del atributo
                        const char* open_span = "<span style=\"color:orange;\">";
                        strcpy(res_p, open_span);
                        res_p += strlen(open_span);

                        strcpy(res_p, escaped);
                        res_p += strlen(escaped);
                    } else if (attr_value_quote == temp[0]) {
                        // Fin del valor del atributo
                        strcpy(res_p, escaped);
                        res_p += strlen(escaped);

                        // Cerrar </span> del valor del atributo
                        const char* close_span = "</span>";
                        strcpy(res_p, close_span);
                        res_p += strlen(close_span);

                        in_attr_value = 0;
                        attr_value_quote = 0;
                    } else {
                        strcpy(res_p, escaped);
                        res_p += strlen(escaped);
                    }
                } else {
                    strcpy(res_p, escaped);
                    res_p += strlen(escaped);
                }
            } else {
                if (in_attr_name) {
                    // Resaltar nombres de atributos
                    const char* open_span = "<span style=\"color:red;\">";
                    strcpy(res_p, open_span);
                    res_p += strlen(open_span);

                    char attr_name[256];
                    int attr_index = 0;
                    while (*p && !isspace(*p) && *p != '=' && *p != '>' && *p != '/' && *p != '<') {
                        if ((size_t)attr_index < sizeof(attr_name) - 1) {
                            attr_name[attr_index++] = *p++;
                        } else {
                            p++;
                        }
                    }
                    attr_name[attr_index] = '\0';

                    char escaped_attr_name[1024];
                    escape_and_replace_spaces(attr_name, escaped_attr_name);

                    strcpy(res_p, escaped_attr_name);
                    res_p += strlen(escaped_attr_name);

                    // Cerrar </span> del nombre del atributo
                    const char* close_span = "</span>";
                    strcpy(res_p, close_span);
                    res_p += strlen(close_span);

                    in_attr_name = 0;
                } else if (in_attr_value) {
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
        } else {
            if (*p == '\n') {
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
    if (attr_value_quote != 0) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }
    if (in_tag) {
        const char* close_span = "</span>";
        strcpy(res_p, close_span);
        res_p += strlen(close_span);
    }

    *res_p = '\0';
    return result;
}
