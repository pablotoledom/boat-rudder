#include <arpa/inet.h>
#include <ctype.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 4096

// Root directory
char *root_directory;

// Function to get the file extension
const char *get_mime_type(const char *path) {
  const char *dot = strrchr(path, '.');
  if (!dot)
    return "text/plain";
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
  return "application/octet-stream";
}

// Function to URL encode
void url_encode(char *dst, const char *src, size_t dst_size) {
  char *dst_end = dst + dst_size - 1; // Reserve space for '\0'
  for (; *src && dst < dst_end; src++) {
    if (isalnum((unsigned char)*src) || strchr("-_.~/", *src)) {
      *dst++ = *src;
    } else {
      if (dst + 3 >= dst_end)
        break; // There is not enough space
      sprintf(dst, "%%%02X", (unsigned char)*src);
      dst += 3;
    }
  }
  *dst = '\0';
}

// Function to URL decode
void url_decode(char *dst, const char *src) {
  char a, b;
  while (*src) {
    if ((*src == '%') && ((a = src[1]) && (b = src[2])) &&
        (isxdigit(a) && isxdigit(b))) {
      if (a >= 'a')
        a -= 'a' - 'A';
      if (a >= 'A')
        a -= 'A' - 10;
      else
        a -= '0';
      if (b >= 'a')
        b -= 'a' - 'A';
      if (b >= 'A')
        b -= 'A' - 10;
      else
        b -= '0';
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

// Function to prevent directory traversal attacks (updated)
void sanitize_path(char *path) {
  char resolved_path[PATH_MAX];
  if (realpath(path, resolved_path) == NULL) {
    strcpy(path, ""); // Path is invalid, clear it.
  } else {
    strcpy(path, resolved_path);
  }
}

// Function to handle HTTP requests
void handle_request(int client_socket) {
  char buffer[BUFFER_SIZE], method[16], url[256], protocol[16];
  int bytes_read = read(client_socket, buffer, sizeof(buffer) - 1);

  if (bytes_read <= 0) {
    close(client_socket);
    return;
  }

  buffer[bytes_read] = '\0';

  // Parse the first line of the request
  sscanf(buffer, "%s %s %s", method, url, protocol);

  // We only handle the GET method
  if (strcmp(method, "GET") != 0) {
    close(client_socket);
    return;
  }

  // Decode the URL
  char decoded_url[256];
  url_decode(decoded_url, url);

  // Prevent directory traversal attacks
  sanitize_path(decoded_url);

  // If the URL ends with '/', remove it (except if it's just '/')
  size_t url_len = strlen(decoded_url);
  if (url_len > 1 && decoded_url[url_len - 1] == '/') {
    decoded_url[url_len - 1] = '\0';
  }

  // Build the path of the requested file
  char path[512];
  snprintf(path, sizeof(path), "%s%s", root_directory, decoded_url);

  // Check if it is a directory
  struct stat path_stat;
  if (stat(path, &path_stat) < 0) {
    // File not found
    const char *not_found_response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";
    write(client_socket, not_found_response, strlen(not_found_response));
    close(client_socket);
    return;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    // Search index.html o index.htm
    char index_path[512];
    snprintf(index_path, sizeof(index_path), "%s/index.html", path);
    if (stat(index_path, &path_stat) == 0) {
      strcpy(path, index_path);
    } else {
      snprintf(index_path, sizeof(index_path), "%s/index.htm", path);
      if (stat(index_path, &path_stat) == 0) {
        strcpy(path, index_path);
      } else {
        // List the content of the directory
        DIR *dir = opendir(path);
        if (!dir) {
          const char *forbidden_response =
              "HTTP/1.1 403 Forbidden\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: 53\r\n"
              "\r\n"
              "<html><body><h1>403 Forbidden</h1></body></html>";
          write(client_socket, forbidden_response, strlen(forbidden_response));
          close(client_socket);
          return;
        }

        char response[BUFFER_SIZE * 10];
        int len = snprintf(response, sizeof(response),
                           "HTTP/1.1 200 OK\r\n"
                           "Content-Type: text/html\r\n\r\n"
                           "<html><body><h1>Directory List</h1><ul>");

        struct dirent *entry;
        while ((entry = readdir(dir)) != NULL) {
          // Ignore "."
          if (strcmp(entry->d_name, ".") == 0)
            continue;

          // Build the route for the link
          char item_path[512];
          char encoded_item_path[512];

          if (strcmp(entry->d_name, "..") == 0) {
            // Handle the link for "Parent"
            if (strcmp(decoded_url, "") == 0 || strcmp(decoded_url, "/") == 0) {
              // We are already in the root directory, do not show link to
              // parent
              continue;
            } else {
              // Get the path of the parent directory
              char parent_url[256];
              strcpy(parent_url, decoded_url);
              char *last_slash = strrchr(parent_url, '/');
              if (last_slash != NULL) {
                *last_slash = '\0';
              } else {
                strcpy(parent_url, "/");
              }
              url_encode(encoded_item_path, parent_url,
                         sizeof(encoded_item_path));
            }
          } else {
            // Build the element path
            if (strcmp(decoded_url, "/") == 0 || strcmp(decoded_url, "") == 0) {
              snprintf(item_path, sizeof(item_path), "/%s", entry->d_name);
            } else {
              snprintf(item_path, sizeof(item_path), "%s/%s", decoded_url,
                       entry->d_name);
            }
            url_encode(encoded_item_path, item_path, sizeof(encoded_item_path));
          }

          // Add trailing slash if directory
          char display_name[256];
          snprintf(display_name, sizeof(display_name), "%s", entry->d_name);

          char entry_full_path[512];
          snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s", path,
                   entry->d_name);

          struct stat entry_stat;
          if (stat(entry_full_path, &entry_stat) == 0 &&
              S_ISDIR(entry_stat.st_mode)) {
            strcat(display_name, "/");
            strcat(encoded_item_path, "/");
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
  int file_fd = open(path, O_RDONLY);
  if (file_fd < 0) {
    const char *not_found_response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";
    write(client_socket, not_found_response, strlen(not_found_response));
    close(client_socket);
    return;
  }

  // Get file size
  off_t file_size = lseek(file_fd, 0, SEEK_END);
  lseek(file_fd, 0, SEEK_SET);

  // Get MIME type
  const char *mime_type = get_mime_type(path);

  // Send HTTP headers
  char header[256];
  int header_len = snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %ld\r\n"
                            "\r\n",
                            mime_type, file_size);
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

int main(int argc, char *argv[]) {
  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  int addrlen = sizeof(address);

  // Check arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  root_directory = argv[1];

  // Create the socket
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("Error creating socket");
    exit(EXIT_FAILURE);
  }

  // Configure socket options
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt))) {
    perror("Error on setsockopt");
    exit(EXIT_FAILURE);
  }

  // Configure address and port
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

  printf("Static HTTP server, listening port %d\n", PORT);
  printf("Root directory: %s\n", root_directory);

  while (1) {
    // Accept an incoming connection
    if ((client_socket = accept(server_fd, (struct sockaddr *)&address,
                                (socklen_t *)&addrlen)) < 0) {
      perror("Error en accept");
      continue;
    }

    // Handle the request in a child process
    if (fork() == 0) {
      close(server_fd);
      handle_request(client_socket);
      exit(0);
    } else {
      close(client_socket);
    }
  }

  return 0;
}
