#ifndef REQUEST_HANDLER_H
#define REQUEST_HANDLER_H

#include <sys/types.h>  // Para definir ssize_t y size_t

// Definición del tipo de función callback para lectura.
typedef ssize_t (*read_func_t)(void *ctx, char *buf, size_t count);

// Nueva firma de la función handle_request que utiliza el callback de lectura y un contexto unificado.
// El parámetro 'root_directory' se utiliza para determinar la raíz del recurso solicitado.
void handle_request(read_func_t read_func, void *ctx, const char *root_directory);

#endif // REQUEST_HANDLER_H
