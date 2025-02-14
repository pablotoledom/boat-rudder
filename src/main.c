#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "include/config_loader.h"
#include "include/log.h"
#include "include/request_handler.h" // Ahora la firma es:
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h> // For PATH_MAX
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h> // For realpath, malloc, free
#include <string.h>
#include <sys/wait.h> // For waitpid
#include <unistd.h>
#include <sys/select.h>

// Global SSL context pointer
SSL_CTX *ssl_ctx = NULL;

// ----------------------------------------------------------------------------
// Definición del contexto de conexión: encapsula el descriptor y, si aplica, el
// SSL
typedef struct {
  int client_socket;
  SSL *ssl; // Si es distinto de NULL, se utiliza para conexiones SSL.
} connection_ctx_t;

// ----------------------------------------------------------------------------
// Funciones callback para lectura según el tipo de conexión:

// Para conexiones sin SSL, se utiliza read() sobre el descriptor:
ssize_t plain_read(void *ctx, char *buf, size_t count) {
  connection_ctx_t *conn = (connection_ctx_t *)ctx;
  return read(conn->client_socket, buf, count);
}

// Para conexiones SSL, se utiliza SSL_read():
ssize_t ssl_read(void *ctx, char *buf, size_t count) {
    connection_ctx_t *conn = (connection_ctx_t *)ctx;
    if (!conn || !conn->ssl) {
        LOG_ERROR("SSL context is NULL");
        return -1;
    }

    int bytes_read = SSL_read(conn->ssl, buf, count);
    if (bytes_read <= 0) {
        int ssl_error = SSL_get_error(conn->ssl, bytes_read);
        LOG_ERROR("SSL_read failed: %d", ssl_error);
    }
    
    return bytes_read;
}


// ----------------------------------------------------------------------------
// Estructura para pasar argumentos a la función del hilo.
struct thread_args {
  int client_socket;
  const char *root_directory;
  SSL *ssl;
};

// Signal handler para evitar procesos zombie
void sigchld_handler(int s) {
  (void)s; // Parámetro no usado
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

// ----------------------------------------------------------------------------
// Inicializa el contexto SSL
SSL_CTX *create_ssl_context() {
  SSL_CTX *ctx = SSL_CTX_new(TLS_server_method());
  if (!ctx) {
    LOG_ERROR("Failed to create SSL context.");
    exit(EXIT_FAILURE);
  }

  if (SSL_CTX_use_certificate_file(ctx, ssl_cert, SSL_FILETYPE_PEM) <= 0 ||
      SSL_CTX_use_PrivateKey_file(ctx, ssl_key, SSL_FILETYPE_PEM) <= 0) {
    LOG_ERROR("Failed to load SSL certificate or key.");
    exit(EXIT_FAILURE);
  }

  return ctx;
}

// ----------------------------------------------------------------------------
// Función del hilo para manejar cada conexión.
// Se crea un contexto de conexión unificado que se pasa a handle_request.
void *thread_function(void *arg) {
    struct thread_args *args = arg;
    if (!args) {
        LOG_ERROR("Thread args are NULL");
        return NULL;
    }

    LOG_DEBUG("Thread started for client %d", args->client_socket);

    connection_ctx_t *conn = malloc(sizeof(connection_ctx_t));
    if (!conn) {
        LOG_ERROR("Failed to allocate memory for connection context");
        close(args->client_socket);
        return NULL;
    }

    conn->client_socket = args->client_socket;
    conn->ssl = args->ssl;

    if (args->ssl) {
        LOG_DEBUG("SSL connection detected, performing SSL_accept...");
        if (SSL_accept(conn->ssl) <= 0) {
            LOG_ERROR("SSL_accept failed: %s", ERR_reason_error_string(ERR_get_error()));
            SSL_free(conn->ssl);
            conn->ssl = NULL;
            close(conn->client_socket);
            free(conn);
            return NULL;
        }
    }

    LOG_DEBUG("Handling request...");
    handle_request(args->ssl ? ssl_read : plain_read, conn, args->root_directory);
    LOG_DEBUG("Finished request handling. Cleaning up...");

    if (conn) {
        if (conn->ssl) {
            LOG_DEBUG("Performing SSL shutdown...");

            // Validar si SSL aún es válido antes de llamar SSL_shutdown
            int fd = SSL_get_fd(conn->ssl);
            if (fd < 0) {
                LOG_DEBUG("Invalid SSL file descriptor. Skipping SSL_shutdown.");
            } else {
                int shutdown_status = SSL_shutdown(conn->ssl);
                if (shutdown_status == 0) {
                    LOG_WARN("SSL_shutdown returned 0, retrying...");
                    shutdown_status = SSL_shutdown(conn->ssl);
                }

                if (shutdown_status < 0) {
                    int err = SSL_get_error(conn->ssl, shutdown_status);
                    LOG_ERROR("SSL_shutdown failed with error: %d (%s)", err, ERR_reason_error_string(ERR_get_error()));
                } else {
                    LOG_DEBUG("SSL_shutdown completed successfully.");
                }

                LOG_DEBUG("Freeing SSL structure...");
                SSL_free(conn->ssl);
                conn->ssl = NULL;
            }
        } else {
            LOG_DEBUG("conn->ssl is NULL before SSL_shutdown()");
        }

        LOG_DEBUG("Closing client socket...");
        close(conn->client_socket);
        free(conn);
    } else {
        LOG_ERROR("conn is NULL before cleanup!");
    }

    free(args);
    LOG_DEBUG("Thread finished");
    return NULL;
}


// ----------------------------------------------------------------------------
// Función principal
int main(int argc, char *argv[]) {
  // Load configuration
  if (load_config("./configs/config.txt") == 0) {
    log_level = verbose_level;
    LOG_INFO("verbose_level: %d\n", verbose_level);
    LOG_INFO("HTTP Server Port: %d\n", http_port);
    if (ssl_enabled) {
      LOG_INFO("HTTPS Server Port: %d\n", https_port);
      LOG_INFO("SSL Enabled. Cert: %s, Key: %s\n", ssl_cert, ssl_key);
    }
  } else {
    LOG_ERROR("Error loading configuration.");
    exit(EXIT_FAILURE);
  }

  int server_fd_http, server_fd_https = -1, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  // Resolve the root directory
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  char resolved_path[PATH_MAX];
  if (realpath(argv[1], resolved_path) == NULL) {
    LOG_ERROR("Invalid root directory: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }
  char *root_directory = strdup(resolved_path);
  if (!root_directory) {
    LOG_ERROR("Failed to allocate memory for root_directory: %s", strerror(errno));
    exit(EXIT_FAILURE);
  }

  // Create HTTP socket
  if ((server_fd_http = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    LOG_ERROR("Error creating HTTP socket: %s", strerror(errno));
    free(root_directory);
    exit(EXIT_FAILURE);
  }

  if (setsockopt(server_fd_http, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    LOG_ERROR("Error on setsockopt: %s", strerror(errno));
    free(root_directory);
    close(server_fd_http);
    exit(EXIT_FAILURE);
  }

  // Configure HTTP address
  address.sin_family = AF_INET;
  address.sin_addr.s_addr = INADDR_ANY;
  address.sin_port = htons(http_port);

  if (bind(server_fd_http, (struct sockaddr *)&address, sizeof(address)) < 0) {
    LOG_ERROR("Error on HTTP bind: %s", strerror(errno));
    free(root_directory);
    close(server_fd_http);
    exit(EXIT_FAILURE);
  }

  if (listen(server_fd_http, 10) < 0) {
    LOG_ERROR("Error on HTTP listen: %s", strerror(errno));
    free(root_directory);
    close(server_fd_http);
    exit(EXIT_FAILURE);
  }

  LOG_INFO("HTTP Server listening on port %d\n", http_port);

  // Create HTTPS socket if SSL is enabled
  if (ssl_enabled) {
    if ((server_fd_https = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
      LOG_ERROR("Error creating HTTPS socket: %s", strerror(errno));
      free(root_directory);
      close(server_fd_http);
      exit(EXIT_FAILURE);
    }

    if (setsockopt(server_fd_https, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
      LOG_ERROR("Error on setsockopt (HTTPS): %s", strerror(errno));
      free(root_directory);
      close(server_fd_http);
      close(server_fd_https);
      exit(EXIT_FAILURE);
    }

    address.sin_port = htons(https_port);

    if (bind(server_fd_https, (struct sockaddr *)&address, sizeof(address)) < 0) {
      LOG_ERROR("Error on HTTPS bind: %s", strerror(errno));
      free(root_directory);
      close(server_fd_http);
      close(server_fd_https);
      exit(EXIT_FAILURE);
    }

    if (listen(server_fd_https, 10) < 0) {
      LOG_ERROR("Error on HTTPS listen: %s", strerror(errno));
      free(root_directory);
      close(server_fd_http);
      close(server_fd_https);
      exit(EXIT_FAILURE);
    }

    // Initialize SSL context
    ssl_ctx = create_ssl_context();
    LOG_INFO("HTTPS Server listening on port %d\n", https_port);
  }

  // Main loop with select()
  fd_set readfds;
  int max_sd = server_fd_http > server_fd_https ? server_fd_http : server_fd_https;

  while (1) {
    FD_ZERO(&readfds);
    FD_SET(server_fd_http, &readfds);
    if (ssl_enabled) {
      FD_SET(server_fd_https, &readfds);
    }

    int activity = select(max_sd + 1, &readfds, NULL, NULL, NULL);
    if ((activity < 0) && (errno != EINTR)) {
      LOG_ERROR("Select error: %s", strerror(errno));
    }

    // Handle new HTTP connection
    if (FD_ISSET(server_fd_http, &readfds)) {
      client_socket = accept(server_fd_http, (struct sockaddr *)&address, &addrlen);
      if (client_socket < 0) {
        LOG_ERROR("Error on HTTP accept: %s", strerror(errno));
        continue;
      }

      struct thread_args *args = malloc(sizeof(struct thread_args));
      if (!args) {
        LOG_ERROR("Failed to allocate memory for HTTP connection: %s", strerror(errno));
        close(client_socket);
        continue;
      }

      args->client_socket = client_socket;
      args->root_directory = root_directory;
      args->ssl = NULL; // Plain HTTP

      pthread_t thread_id;
      if (pthread_create(&thread_id, NULL, thread_function, (void *)args) != 0) {
        LOG_ERROR("Could not create HTTP thread: %s", strerror(errno));
        close(client_socket);
        free(args);
        continue;
      }
      pthread_detach(thread_id);
    }

    // Handle new HTTPS connection
    if (ssl_enabled && FD_ISSET(server_fd_https, &readfds)) {
      client_socket = accept(server_fd_https, (struct sockaddr *)&address, &addrlen);
      if (client_socket < 0) {
        LOG_ERROR("Error on HTTPS accept: %s", strerror(errno));
        continue;
      }

      struct thread_args *args = malloc(sizeof(struct thread_args));
      if (!args) {
        LOG_ERROR("Failed to allocate memory for HTTPS connection: %s", strerror(errno));
        close(client_socket);
        continue;
      }

      args->client_socket = client_socket;
      args->root_directory = root_directory;
      args->ssl = SSL_new(ssl_ctx);
      SSL_set_fd(args->ssl, client_socket);

      pthread_t thread_id;
      if (pthread_create(&thread_id, NULL, thread_function, (void *)args) != 0) {
        LOG_ERROR("Could not create HTTPS thread: %s", strerror(errno));
        close(client_socket);
        free(args);
        continue;
      }
      pthread_detach(thread_id);
    }
  }

  // Cleanup
  free(root_directory);
  close(server_fd_http);
  if (ssl_enabled) {
    close(server_fd_https);
    SSL_CTX_free(ssl_ctx);
  }

  return 0;
}