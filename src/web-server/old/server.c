#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <inttypes.h>   // For PRIdMAX
#include <netinet/in.h>
#include <signal.h>     // For signal handling
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>   // For waitpid
#include <unistd.h>
#include <pthread.h>    // For threading

#define PORT 8080
#define BUFFER_SIZE 4096

// Root directory
char *root_directory;

// Function to get the file extension
const char *get_mime_type(const char *path) {
    const char *dot = strrchr(path, '.');
    if (!dot)
        return "application/octet-stream";
    if (strcmp(dot, ".html") == 0)
        return "text/html";
    if (strcmp(dot, ".htm") == 0)
        return "text/html";
    if (strcmp(dot, ".css") == 0)
        return "text/css";
    if (strcmp(dot, ".js") == 0)
        return "application/javascript";
    if (strcmp(dot, ".jpg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".jpeg") == 0)
        return "image/jpeg";
    if (strcmp(dot, ".png") == 0)
        return "image/png";
    if (strcmp(dot, ".gif") == 0)
        return "image/gif";
    if (strcmp(dot, ".ico") == 0)
        return "image/x-icon";
    if (strcmp(dot, ".svg") == 0)
        return "image/svg+xml";
    if (strcmp(dot, ".mp4") == 0)
        return "video/mp4";
    return "application/octet-stream";
}

// Function to URL encode
void url_encode(char *dst, const char *src, size_t dst_size) {
    char *dst_end = dst + dst_size - 1; // Reserve space for '\0'
    while (*src && dst < dst_end) {
        if (isalnum((unsigned char)*src) || strchr("-_.~/", *src)) {
            *dst++ = *src;
        } else {
            if (dst + 3 >= dst_end)
                break; // Not enough space
            snprintf(dst, dst_end - dst, "%%%02X", (unsigned char)*src);
            dst += 3;
        }
        src++;
    }
    *dst = '\0';
}

// Function to URL decode
void url_decode(char *dst, const char *src) {
    char a, b;
    while (*src) {
        if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
            (isxdigit(a) && isxdigit(b))) {
            a = tolower(a);
            b = tolower(b);
            a = (a >= 'a') ? a - 'a' + 10 : a - '0';
            b = (b >= 'a') ? b - 'a' + 10 : b - '0';
            *dst++ = 16 * a + b;
            src += 3;
        } else if (*src == '+') {
            *dst++ = ' ';
            src++;
        } else {
            *dst++ = *src++;
        }
    }
    *dst = '\0';
}

// Function to HTML encode
void html_encode(char *dst, const char *src, size_t dst_size) {
    char *dst_end = dst + dst_size - 1;
    while (*src && dst < dst_end) {
        if (*src == '&') {
            if (dst + 5 >= dst_end) break;
            strncpy(dst, "&amp;", dst_end - dst);
            dst += 5;
        } else if (*src == '<') {
            if (dst + 4 >= dst_end) break;
            strncpy(dst, "&lt;", dst_end - dst);
            dst += 4;
        } else if (*src == '>') {
            if (dst + 4 >= dst_end) break;
            strncpy(dst, "&gt;", dst_end - dst);
            dst += 4;
        } else if (*src == '"') {
            if (dst + 6 >= dst_end) break;
            strncpy(dst, "&quot;", dst_end - dst);
            dst += 6;
        } else {
            *dst++ = *src;
        }
        src++;
    }
    *dst = '\0';
}

// Function to prevent directory traversal attacks
void sanitize_path(const char *url_path, char *safe_path, size_t size) {
    char path[PATH_MAX];
    snprintf(path, sizeof(path), "%s%s", root_directory, url_path);

    char resolved_path[PATH_MAX];
    if (realpath(path, resolved_path) == NULL) {
        // Invalid path
        safe_path[0] = '\0';
    } else {
        // Ensure the resolved path starts with the root directory
        if (strncmp(resolved_path, root_directory, strlen(root_directory)) == 0) {
            strncpy(safe_path, resolved_path, size - 1);
            safe_path[size - 1] = '\0';
        } else {
            // Attempted access outside root directory
            safe_path[0] = '\0';
        }
    }
}

// Signal handler to reap zombie processes
void sigchld_handler(int s) {
    (void)s; // Unused parameter
    int saved_errno = errno;
    while (waitpid(-1, NULL, WNOHANG) > 0)
        ;
    errno = saved_errno;
}

// Function to handle HTTP requests
void handle_request(int client_socket) {
    char buffer[BUFFER_SIZE];
    char method[16], url[256], protocol[16];

    int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

    if (bytes_read <= 0) {
        close(client_socket);
        return;
    }

    buffer[bytes_read] = '\0';

    // Parse the first line of the request
    if (sscanf(buffer, "%15s %255s %15s", method, url, protocol) != 3) {
        // Bad request
        const char *bad_request_response =
            "HTTP/1.1 400 Bad Request\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 51\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>400 Bad Request</h1></body></html>";
        write(client_socket, bad_request_response, strlen(bad_request_response));
        close(client_socket);
        return;
    }

    // We only handle the GET method
    if (strcmp(method, "GET") != 0) {
        const char *method_not_allowed_response =
            "HTTP/1.1 405 Method Not Allowed\r\n"
            "Allow: GET\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 54\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>405 Method Not Allowed</h1></body></html>";
        write(client_socket, method_not_allowed_response, strlen(method_not_allowed_response));
        close(client_socket);
        return;
    }

    // Decode the URL
    char decoded_url[256];
    url_decode(decoded_url, url);

    // Prevent directory traversal attacks and resolve the safe path
    char safe_path[PATH_MAX];
    sanitize_path(decoded_url, safe_path, sizeof(safe_path));

    if (strlen(safe_path) == 0) {
        // Forbidden
        const char *forbidden_response =
            "HTTP/1.1 403 Forbidden\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 53\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>403 Forbidden</h1></body></html>";
        write(client_socket, forbidden_response, strlen(forbidden_response));
        close(client_socket);
        return;
    }

    // Check if the path is a directory or a file
    struct stat path_stat;
    if (stat(safe_path, &path_stat) < 0) {
        // File not found
        const char *not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 48\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        write(client_socket, not_found_response, strlen(not_found_response));
        close(client_socket);
        return;
    }

    if (S_ISDIR(path_stat.st_mode)) {
        // Attempt to serve index.html or index.htm
        char index_path[PATH_MAX];
        snprintf(index_path, sizeof(index_path), "%s/index.html", safe_path);
        if (stat(index_path, &path_stat) == 0) {
            strncpy(safe_path, index_path, sizeof(safe_path) - 1);
            safe_path[sizeof(safe_path) - 1] = '\0';
        } else {
            snprintf(index_path, sizeof(index_path), "%s/index.htm", safe_path);
            if (stat(index_path, &path_stat) == 0) {
                strncpy(safe_path, index_path, sizeof(safe_path) - 1);
                safe_path[sizeof(safe_path) - 1] = '\0';
            } else {
                // List the content of the directory
                DIR *dir = opendir(safe_path);
                if (!dir) {
                    const char *forbidden_response =
                        "HTTP/1.1 403 Forbidden\r\n"
                        "Content-Type: text/html\r\n"
                        "Content-Length: 53\r\n"
                        "Connection: close\r\n"
                        "\r\n"
                        "<html><body><h1>403 Forbidden</h1></body></html>";
                    write(client_socket, forbidden_response, strlen(forbidden_response));
                    close(client_socket);
                    return;
                }

                char response[BUFFER_SIZE * 10];
                int len = snprintf(response, sizeof(response),
                                   "HTTP/1.1 200 OK\r\n"
                                   "Content-Type: text/html\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "<html><body><h1>Directory Listing</h1><ul>");

                struct dirent *entry;
                while ((entry = readdir(dir)) != NULL) {
                    // Ignore "."
                    if (strcmp(entry->d_name, ".") == 0)
                        continue;

                    char item_path[PATH_MAX];
                    char encoded_item_path[PATH_MAX];

                    // Build the element path
                    if (strcmp(decoded_url, "/") == 0 || strcmp(decoded_url, "") == 0) {
                        snprintf(item_path, sizeof(item_path), "/%s", entry->d_name);
                    } else {
                        snprintf(item_path, sizeof(item_path), "%s/%s", decoded_url, entry->d_name);
                    }
                    url_encode(encoded_item_path, item_path, sizeof(encoded_item_path));

                    // Add trailing slash if directory
                    char display_name[256];
                    html_encode(display_name, entry->d_name, sizeof(display_name));

                    char entry_full_path[PATH_MAX];
                    snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s", safe_path, entry->d_name);

                    struct stat entry_stat;
                    if (stat(entry_full_path, &entry_stat) == 0 &&
                        S_ISDIR(entry_stat.st_mode)) {
                        strncat(display_name, "/", sizeof(display_name) - strlen(display_name) - 1);
                        strncat(encoded_item_path, "/", sizeof(encoded_item_path) - strlen(encoded_item_path) - 1);
                    }

                    len += snprintf(response + len, sizeof(response) - len,
                                    "<li><a href=\"%s\">%s</a></li>", encoded_item_path,
                                    display_name);
                }
                closedir(dir);
                len += snprintf(response + len, sizeof(response) - len,
                                "</ul></body></html>");

                write(client_socket, response, len);
                close(client_socket);
                return;
            }
        }
    }

    // Send the file
    int file_fd = open(safe_path, O_RDONLY);
    if (file_fd < 0) {
        perror("Failed to open file");
        const char *not_found_response =
            "HTTP/1.1 404 Not Found\r\n"
            "Content-Type: text/html\r\n"
            "Content-Length: 48\r\n"
            "Connection: close\r\n"
            "\r\n"
            "<html><body><h1>404 Not Found</h1></body></html>";
        write(client_socket, not_found_response, strlen(not_found_response));
        close(client_socket);
        return;
    }

    // Get file size
    struct stat file_stat;
    if (fstat(file_fd, &file_stat) < 0) {
        perror("Failed to get file stats");
        close(file_fd);
        close(client_socket);
        return;
    }
    off_t file_size = file_stat.st_size;

    // Get MIME type
    const char *mime_type = get_mime_type(safe_path);

    // Send HTTP headers
    char header[256];
    int header_len = snprintf(header, sizeof(header),
                              "HTTP/1.1 200 OK\r\n"
                              "Content-Type: %s\r\n"
                              "Content-Length: %" PRIdMAX "\r\n"
                              "Connection: close\r\n"
                              "\r\n",
                              mime_type, (intmax_t)file_size);
    write(client_socket, header, header_len);

    // Send file content
    ssize_t sent_bytes, read_bytes;
    while ((read_bytes = read(file_fd, buffer, sizeof(buffer))) > 0) {
        sent_bytes = write(client_socket, buffer, read_bytes);
        if (sent_bytes < 0) {
            perror("Error sending file");
            break;
        }
    }

    close(file_fd);
    close(client_socket);
}

// Thread function to handle requests
void *thread_function(void *arg) {
    int client_socket = *(int *)arg;
    free(arg);
    handle_request(client_socket);
    return NULL;
}

int main(int argc, char *argv[]) {
    int server_fd, client_socket;
    struct sockaddr_in address;
    int opt = 1;
    socklen_t addrlen = sizeof(address);

    // Check arguments
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    root_directory = realpath(argv[1], NULL);
    if (!root_directory) {
        perror("Invalid root directory");
        exit(EXIT_FAILURE);
    }

    // Create the socket
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("Error creating socket");
        free(root_directory);
        exit(EXIT_FAILURE);
    }

    // Configure socket options
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
        perror("Error on setsockopt");
        free(root_directory);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Configure address and port
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Bind the socket to the specified port
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("Error on bind");
        free(root_directory);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Listen for incoming connections
    if (listen(server_fd, 10) < 0) {
        perror("Error on listen");
        free(root_directory);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    // Set up the signal handler
    struct sigaction sa;
    sa.sa_handler = sigchld_handler; // Reap all dead processes
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("Error setting up signal handler");
        free(root_directory);
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    printf("Static HTTP server, listening on port %d\n", PORT);
    printf("Root directory: %s\n", root_directory);

    while (1) {
        // Accept an incoming connection
        client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
        if (client_socket < 0) {
            perror("Error on accept");
            continue;
        }

        // Handle the request in a new thread
        int *new_sock = malloc(sizeof(int));
        if (!new_sock) {
            perror("Failed to allocate memory");
            close(client_socket);
            continue;
        }
        *new_sock = client_socket;

        pthread_t thread_id;
        if (pthread_create(&thread_id, NULL, thread_function, (void *)new_sock) != 0) {
            perror("Could not create thread");
            close(client_socket);
            free(new_sock);
            continue;
        }

        pthread_detach(thread_id); // Resources will be cleaned up when the thread exits
    }

    free(root_directory);
    close(server_fd);
    return 0;
}
