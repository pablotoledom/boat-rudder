#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Root directory
char *root_directory;

// Function declarations
const char *get_mime_type(const char *path);
void url_encode(char *dst, const char *src, size_t dst_size);
void url_decode(char *dst, const char *src);
void sanitize_path(char *path);
SSL_CTX *initialize_ssl_context();
void configure_ssl_context(SSL_CTX *ctx);
void handle_request(SSL *ssl);
void handle_ssl_connection(SSL *ssl);

const char *get_mime_type(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot) return "text/plain";
    if (strcmp(dot, ".html") == 0) return "text/html";
    if (strcmp(dot, ".htm") == 0) return "text/html";
    if (strcmp(dot, ".css") == 0) return "text/css";
    if (strcmp(dot, ".js") == 0) return "application/javascript";
    if (strcmp(dot, ".jpg") == 0) return "image/jpeg";
    if (strcmp(dot, ".jpeg") == 0) return "image/jpeg";
    if (strcmp(dot, ".png") == 0) return "image/png";
    if (strcmp(dot, ".gif") == 0) return "image/gif";
    if (strcmp(dot, ".ico") == 0) return "image/x-icon";
    return "application/octet-stream";
}

void url_encode(char *dst, const char *src, size_t dst_size) {
    char *dst_end = dst + dst_size - 1; // Reserve space for '\0'
    for (; *src && dst < dst_end; src++) {
        if (isalnum((unsigned char)*src) || strchr("-_.~/", *src)) {
            *dst++ = *src;
        } else {
            if (dst + 3 >= dst_end) break; // Not enough space
            sprintf(dst, "%%%02X", (unsigned char)*src);
            dst += 3;
        }
    }
    *dst = '\0';
}

void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') &&
            ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            if (a >= 'a') a -= 'a' - 'A';
            if (a >= 'A') a -= 'A' - 10;
            else a -= '0';
            if (b >= 'a') b -= 'a' - 'A';
            if (b >= 'A') b -= 'A' - 10;
            else b -= '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst++ = '\0';
}

void sanitize_path(char *path) {
    char *p = path;
    while ((p = strstr(p, "..")) != NULL) {
        memmove(p, p + 2, strlen(p + 2) + 1);
    }
}

// Function to initialize SSL context
SSL_CTX *initialize_ssl_context() {
    const SSL_METHOD *method;
    SSL_CTX *ctx;

    // Initialize the OpenSSL library
    OpenSSL_add_all_algorithms();
    SSL_load_error_strings();
    method = TLS_server_method();
    ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

// Function to configure SSL context with certificate and key
void configure_ssl_context(SSL_CTX *ctx) {
    // Set the certificate and private key files
    if (SSL_CTX_use_certificate_file(ctx, "./ssl/server.crt", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, "./ssl/private.key", SSL_FILETYPE_PEM) <= 0 ) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

// Function to handle SSL connections
void handle_ssl_connection(SSL *ssl) {
    if (SSL_accept(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        handle_request(ssl);
    }

    // Shutdown the SSL connection
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

// Function to handle HTTP requests over SSL
void handle_request(SSL *ssl) {
    // ... (implementation remains the same)
}

int main(int argc, char *argv[]) {
    int server_fd;
    SSL_CTX *ctx;

    // Check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    root_directory = argv[1];

    // Initialize OpenSSL and create SSL context
    ctx = initialize_ssl_context();
    configure_ssl_context(ctx);

    // Create the socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creating socket");
        exit(EXIT_FAILURE);
    }

    // Configure socket options
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Error on setsockopt");
        exit(EXIT_FAILURE);
    }

    // Configure address and port
    struct sockaddr_in address;
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error on bind");
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("Error on listen");
        exit(EXIT_FAILURE);
    }

    printf("Static HTTPS server listening on port %d\n", PORT);
    printf("Root directory: %s\n", root_directory);

    while (1) {
        struct sockaddr_in client_address;
        int addrlen = sizeof(client_address);
        int client_socket = accept(server_fd, (struct sockaddr *)&client_address, (socklen_t*)&addrlen);

        if (client_socket < 0) {
            perror("Error on accept");
            continue;
        }

        // Create a new SSL structure for the connection
        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client_socket);

        // Handle the connection in a child process
        if (fork() == 0) {
            close(server_fd);
            handle_ssl_connection(ssl);
            close(client_socket);
            exit(0);
        } else {
            close(client_socket);
            // SSL structure will be freed in the child process
        }
    }

    // Clean up SSL context
    close(server_fd);
    SSL_CTX_free(ctx);
    return 0;
}
