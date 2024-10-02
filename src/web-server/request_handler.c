#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/request_handler.h"
#include "../include/orchestrator.h"
#include "../include/server_utils.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h> // For PRIdMAX
#include <limits.h>   // For PATH_MAX
#include <netinet/in.h>
#include <stdint.h> // For intmax_t
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH (PATH_MAX + 256)

void handle_request(int client_socket, const char *root_directory) {
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
    write(client_socket, method_not_allowed_response,
          strlen(method_not_allowed_response));
    close(client_socket);
    return;
  }

  // Decode the URL
  char decoded_url[256];
  url_decode(decoded_url, url);

  // Check if the client is accessing the home directory
  //   if (strcmp(decoded_url, "/") == 0 || strcmp(decoded_url, "") == 0) {

  //     // Respond with the welcome page using the function from orchestrator.c
  //     const char *response = buildHomeWebSite();

  //     write(client_socket, response, strlen(response));
  //     close(client_socket);
  //     return;
  //   }

  //   // Check if the client is accessing the home directory
  //   if (strcmp(decoded_url, "/blog") == 0 || strcmp(decoded_url, "") == 0) {

  //     // Respond with the welcome page using the function from orchestrator.c
  //     const char *response = buildBlogWebSite();

  //     write(client_socket, response, strlen(response));
  //     close(client_socket);
  //     return;
  //   }

  // Verificar si la URL es exactamente "/"
  if (strcmp(decoded_url, "/") == 0 ||
      strcmp(decoded_url, "") == 0) {
    // Responder con la página principal del blog
    const char *response = buildHomeWebSite();

    if (response == NULL) {
      // Manejar el error si la respuesta es NULL
      const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 21\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Internal Server Error";
      write(client_socket, error_response, strlen(error_response));
    } else {
      write(client_socket, response, strlen(response));
      free((void *)response); // Liberar la memoria si es necesario
    }
    close(client_socket);
    return;
  }
  // Verificar si la URL comienza con "/blog"
  else if (strcmp(decoded_url, "/blog") == 0 || strcmp(decoded_url, "") == 0) {
    // Responder con la página principal del blog
    const char *response = buildBlogWebSite();

    if (response == NULL) {
      // Manejar el error si la respuesta es NULL
      const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 21\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Internal Server Error";
      write(client_socket, error_response, strlen(error_response));
    } else {
      write(client_socket, response, strlen(response));
      free((void *)response); // Liberar la memoria si es necesario
    }
    close(client_socket);
    return;
  }
  // Verificar si la URL comienza con "/blog/"
  else if (strncmp(decoded_url, "/blog/", 6) == 0) {
    // Extraer el identificador después de "/blog/"
    const char *id = decoded_url + 6; // Apunta al carácter después de "/blog/"

    // Opcional: Verificar que el ID no esté vacío
    if (strlen(id) == 0) {
      // Responder con un error 404 si el ID está vacío
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      write(client_socket, error_response, strlen(error_response));
      close(client_socket);
      return;
    }

    // Llamar a la función para generar la página del blog específico
    const char *response = buildBlogEntryWebSite(id);

    if (response == NULL) {
      // Manejar el error si la respuesta es NULL
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      write(client_socket, error_response, strlen(error_response));
    } else {
      write(client_socket, response, strlen(response));
      free((void *)response); // Liberar la memoria si es necesario
    }
    close(client_socket);
    return;
  }

  // Prevent directory traversal attacks and resolve the safe path
  char safe_path[MAX_PATH_LENGTH];
  if (!sanitize_path(decoded_url, safe_path, sizeof(safe_path),
                     root_directory)) {
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
    char index_path[MAX_PATH_LENGTH];
    if (snprintf(index_path, sizeof(index_path), "%s/index.html", safe_path) >=
        (int)sizeof(index_path)) {
      // Path too long
      fprintf(stderr, "Path too long when building index_path\n");
      close(client_socket);
      return;
    }

    if (stat(index_path, &path_stat) == 0) {
      strncpy(safe_path, index_path, sizeof(safe_path) - 1);
      safe_path[sizeof(safe_path) - 1] = '\0';
    } else {
      if (snprintf(index_path, sizeof(index_path), "%s/index.htm", safe_path) >=
          (int)sizeof(index_path)) {
        // Path too long
        fprintf(stderr, "Path too long when building index_path\n");
        close(client_socket);
        return;
      }
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

          char item_path[MAX_PATH_LENGTH];
          char encoded_item_path[MAX_PATH_LENGTH];

          // Build the element path
          size_t decoded_len = strlen(decoded_url);
          if (strcmp(decoded_url, "/") == 0 || strcmp(decoded_url, "") == 0) {
            // We are at the root directory
            snprintf(item_path, sizeof(item_path), "/%s", entry->d_name);
          } else {
            // Check if decoded_url ends with '/'
            if (decoded_url[decoded_len - 1] == '/') {
              // Do not add extra '/'
              snprintf(item_path, sizeof(item_path), "%s%s", decoded_url,
                       entry->d_name);
            } else {
              snprintf(item_path, sizeof(item_path), "%s/%s", decoded_url,
                       entry->d_name);
            }
          }

          url_encode(encoded_item_path, item_path, sizeof(encoded_item_path));

          // Add trailing slash if directory
          char display_name[256];
          html_encode(display_name, entry->d_name, sizeof(display_name));

          char entry_full_path[MAX_PATH_LENGTH];
          if (snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s",
                       safe_path,
                       entry->d_name) >= (int)sizeof(entry_full_path)) {
            fprintf(stderr, "Path too long when building entry_full_path\n");
            continue;
          }

          struct stat entry_stat;
          if (stat(entry_full_path, &entry_stat) == 0 &&
              S_ISDIR(entry_stat.st_mode)) {
            strncat(display_name, "/",
                    sizeof(display_name) - strlen(display_name) - 1);
            strncat(encoded_item_path, "/",
                    sizeof(encoded_item_path) - strlen(encoded_item_path) - 1);
          }

          int n = snprintf(response + len, sizeof(response) - len,
                           "<li><a href=\"%s\">%s</a></li>", encoded_item_path,
                           display_name);
          if (n < 0 || n >= (int)(sizeof(response) - len)) {
            // Response buffer full or error
            fprintf(stderr, "Response buffer full or error\n");
            break;
          }
          len += n;
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
