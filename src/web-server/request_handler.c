#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../include/request_handler.h"
#include "../include/config_loader.h"
#include "../include/detect_epoch.h"
#include "../include/log.h"
#include "../include/orchestrator.h"
#include "../include/server_utils.h"
#include <arpa/inet.h>
#include <dirent.h>
#include <fcntl.h>
#include <inttypes.h> // For PRIdMAX
#include <limits.h>   // For PATH_MAX
#include <netinet/in.h>
#include <openssl/err.h>
#include <openssl/ssl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define BUFFER_SIZE 4096
#define MAX_PATH_LENGTH (PATH_MAX + 256)
#define MAX_PARAMS 10
#define MAX_PARAM_LENGTH 100

// Special route definitions
#define ROUTE_HOME "/"
#define ROUTE_BLOG "/blog"
#define ROUTE_BLOG_ENTRY "/blog/"
#define ROUTE_PAGE "/page"
#define ROUTE_PAGE_ENTRY "/page/"

//--------------------------------------------------------------------------
// Definición del contexto de conexión (unificado)
// Este contexto encapsula el descriptor del socket y, en caso de conexión SSL,
// el puntero a SSL.
typedef struct {
  int client_socket;
  SSL *ssl; // Si es distinto de NULL se usa SSL, de lo contrario conexión
            // "plana"
} connection_ctx_t;

//--------------------------------------------------------------------------
// Estructura para parámetros de query.
typedef struct {
  char key[MAX_PARAM_LENGTH];
  char value[MAX_PARAM_LENGTH];
} QueryParam;

// Separa la URL en la ruta y los parámetros de query.
void split_url(const char *url, char *route, QueryParam *params,
               int *param_count) {
  const char *query_start = strchr(url, '?');

  if (query_start == NULL) {
    strcpy(route, url);
    *param_count = 0;
  } else {
    size_t route_length = query_start - url;
    strncpy(route, url, route_length);
    route[route_length] = '\0';

    const char *param = query_start + 1;
    *param_count = 0;

    while (param && *param && *param_count < MAX_PARAMS) {
      const char *value = strchr(param, '=');
      const char *next_param = strchr(param, '&');

      if (value) {
        size_t key_length = value - param;
        strncpy(params[*param_count].key, param, key_length);
        params[*param_count].key[key_length] = '\0';

        value++; // Omitir el '='
        size_t value_length =
            next_param ? (size_t)(next_param - value) : strlen(value);
        strncpy(params[*param_count].value, value, value_length);
        params[*param_count].value[value_length] = '\0';

        (*param_count)++;
      }

      param = next_param ? next_param + 1 : NULL;
    }
  }
}

//--------------------------------------------------------------------------
// Construye la respuesta HTTP completa (headers + body) a partir del HTML.
char *build_full_response(const char *body, const char *headers) {
  if (body == NULL)
    return NULL;

  size_t body_len = strlen(body);
  size_t header_len = snprintf(NULL, 0,
                               "HTTP/1.1 200 OK\r\n"
                               "Content-Type: text/html\r\n"
                               "Content-Length: %zu\r\n"
                               "%s" // Headers adicionales (ej., cookies)
                               "Connection: close\r\n"
                               "\r\n",
                               body_len, headers);

  size_t total_size = header_len + body_len + 1; // +1 para '\0'
  char *response = malloc(total_size);
  if (response == NULL) {
    perror("Failed to allocate memory for HTTP response");
    return NULL;
  }

  snprintf(response, total_size,
           "HTTP/1.1 200 OK\r\n"
           "Content-Type: text/html\r\n"
           "Content-Length: %zu\r\n"
           "%s" // Insertar headers adicionales
           "Connection: close\r\n"
           "\r\n%s",
           body_len, headers, body);

  return response;
}

//--------------------------------------------------------------------------
// Función auxiliar para escribir en la conexión (usa SSL_write o write).
static ssize_t connection_write(void *ctx, const char *buf, size_t count) {
  connection_ctx_t *conn = (connection_ctx_t *)ctx;
  if (conn->ssl) {
    return SSL_write(conn->ssl, buf, count);
  } else {
    return write(conn->client_socket, buf, count);
  }
}

// Función auxiliar para cerrar la conexión.
static void connection_close(void *ctx) {
  connection_ctx_t *conn = (connection_ctx_t *)ctx;
  if (conn->ssl) {
    SSL_shutdown(conn->ssl);
    SSL_free(conn->ssl);
  }
  close(conn->client_socket);
}

//--------------------------------------------------------------------------
// Función que maneja la solicitud HTTP del cliente.
// Ahora la firma es la nueva, usando el callback de lectura y un contexto
// unificado.
void handle_request(read_func_t read_func, void *ctx,
                    const char *root_directory) {
  // connection_ctx_t *conn = (connection_ctx_t *)ctx;
  // int client_socket = conn->client_socket; // Para uso local si se requiere
  LOG_DEBUG("handle_request() called");

  if (!ctx) {
    LOG_ERROR("handle_request: ctx is NULL");
    return;
  }

  char buffer[BUFFER_SIZE];
  char method[16], url[256], protocol[16];

  // Leer la solicitud usando el callback (ya sea read() o SSL_read())
  LOG_DEBUG("Reading request from client...");
  int bytes_read = read_func(ctx, buffer, sizeof(buffer) - 1);
  if (bytes_read <= 0) {
    LOG_WARN("No bytes read from client socket");
    connection_close(ctx);
    return;
  }
  buffer[bytes_read] = '\0';
  LOG_DEBUG("Received request (%d bytes):\n%s", bytes_read, buffer);

  // Analizar la primera línea de la solicitud (método, URL, protocolo)
  if (sscanf(buffer, "%15s %255s %15s", method, url, protocol) != 3) {
    LOG_ERROR("Malformed request: %s", buffer);
    connection_close(ctx);
    return;
  }

  LOG_INFO("Parsed Request: Method=%s, URL=%s, Protocol=%s", method, url,
           protocol);

  // Buscar headers: User-Agent, Accept-Language y Cookie
  const char *user_agent_key = "User-Agent:";
  const char *accept_language_key = "Accept-Language:";
  const char *cookie_key = "Cookie:";

  size_t user_agent_key_len = strlen(user_agent_key);
  size_t accept_language_key_len = strlen(accept_language_key);
  size_t cookie_key_len = strlen(cookie_key);

  char *user_agent = NULL;
  char *accept_language = NULL;
  char *cookie_header = NULL;

  char *line = strtok(buffer, "\r\n");
  while (line != NULL) {
    if (strncmp(line, user_agent_key, user_agent_key_len) == 0) {
      user_agent = line + user_agent_key_len;
    } else if (strncmp(line, accept_language_key, accept_language_key_len) ==
               0) {
      accept_language = line + accept_language_key_len;
    } else if (strncmp(line, cookie_key, cookie_key_len) == 0) {
      cookie_header = line + cookie_key_len;
    }
    if (user_agent && accept_language && cookie_header)
      break;
    line = strtok(NULL, "\r\n");
  }

  LOG_DEBUG("Headers parsed successfully");

  if (user_agent) {
    LOG_INFO("User-Agent: %s", user_agent);
  } else {
    LOG_INFO("User-Agent not found in the request");
  }
  if (accept_language) {
    LOG_INFO("Accept-Language: %s", accept_language);
  } else {
    LOG_INFO("Accept-Language header not found");
  }

  // Detectar la época del navegador (usando detect_epoch)
  int epoch = EPOCH_EARLY; // Valor por defecto
  if (user_agent) {
    epoch = detect_epoch(user_agent);
    LOG_DEBUG("Detected epoch: %d", epoch);
  } else {
    LOG_DEBUG("User-Agent not found, using default epoch");
  }
  if (!accept_language) {
    LOG_DEBUG("Accept-Language header not found, using default language");
    accept_language = "en";
  }

  // Procesar la primera línea de la solicitud
  int ret = sscanf(buffer, "%15s %255s %15s", method, url, protocol);
  if (ret != 3) {
    const char *bad_request_response =
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 51\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>400 Bad Request</h1></body></html>";
    LOG_ERROR("Bad request: could not parse the request line");
    connection_write(ctx, bad_request_response, strlen(bad_request_response));
    connection_close(ctx);
    return;
  }
  if (strncmp(protocol, "HTTP/", 5) != 0) {
    const char *bad_request_response =
        "HTTP/1.1 400 Bad Request\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 51\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>400 Bad Request</h1></body></html>";
    LOG_ERROR("Bad request: invalid protocol");
    connection_write(ctx, bad_request_response, strlen(bad_request_response));
    connection_close(ctx);
    return;
  }

  char route[256];
  QueryParam params[MAX_PARAMS];
  int param_count;
  split_url(url, route, params, &param_count);
  LOG_INFO("Request URL: %s", route);

  // Variables locales para opciones de tema e idioma
  strcpy(theme, "dark");
  strcpy(lang, "Eng");

  int query_theme_set = 0, query_lang_set = 0;

  for (int i = 0; i < param_count; i++) {
    LOG_INFO("Query parameter: %s: %s", params[i].key, params[i].value);
    if (strcmp(params[i].key, "theme") == 0) {
      query_theme_set = 1;
      if (strcmp(params[i].value, "light") == 0) {
        strcpy(theme, "light");
      } else if (strcmp(params[i].value, "dark") == 0) {
        strcpy(theme, "dark");
      } else {
        strcpy(theme, params[i].value);
      }
    }
    if (strcmp(params[i].key, "lang") == 0) {
      query_lang_set = 1;
      if (strcmp(params[i].value, "english") == 0) {
        strcpy(lang, "Eng");
      } else if (strcmp(params[i].value, "spanish") == 0) {
        strcpy(lang, "Esp");
      } else {
        strcpy(lang, params[i].value);
      }
    }
  }

  // Procesar header Cookie (si existe)
  char cookie_lang_val[32] = "";
  char cookie_theme_val[32] = "";
  if (cookie_header != NULL) {
    char cookie_copy[256];
    strncpy(cookie_copy, cookie_header, sizeof(cookie_copy) - 1);
    cookie_copy[sizeof(cookie_copy) - 1] = '\0';

    char *token = strtok(cookie_copy, ";");
    while (token != NULL) {
      while (*token == ' ')
        token++;
      if (strncmp(token, "lang=", 5) == 0) {
        strncpy(cookie_lang_val, token + 5, sizeof(cookie_lang_val) - 1);
        cookie_lang_val[sizeof(cookie_lang_val) - 1] = '\0';
      } else if (strncmp(token, "theme=", 6) == 0) {
        strncpy(cookie_theme_val, token + 6, sizeof(cookie_theme_val) - 1);
        cookie_theme_val[sizeof(cookie_theme_val) - 1] = '\0';
      }
      token = strtok(NULL, ";");
    }
    LOG_DEBUG("Parsed cookies - lang: %s, theme: %s", cookie_lang_val,
              cookie_theme_val);
  }

  if (!query_lang_set && strlen(cookie_lang_val) > 0) {
    if (strcmp(cookie_lang_val, "en") == 0)
      strcpy(lang, "Eng");
    else if (strcmp(cookie_lang_val, "es") == 0)
      strcpy(lang, "Esp");
    else
      strcpy(lang, cookie_lang_val);
  }
  if (!query_theme_set && strlen(cookie_theme_val) > 0) {
    strcpy(theme, cookie_theme_val);
  }

  LOG_INFO("Selected theme: %s", theme);
  LOG_INFO("Selected language: %s", lang);

  // Solo se acepta el método GET
  if (strcmp(method, "GET") != 0) {
    const char *method_not_allowed_response =
        "HTTP/1.1 405 Method Not Allowed\r\n"
        "Allow: GET\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 54\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>405 Method Not Allowed</h1></body></html>";
    LOG_WARN("Method not allowed: %s", method);
    connection_write(ctx, method_not_allowed_response,
                     strlen(method_not_allowed_response));
    connection_close(ctx);
    return;
  }

  // Decodificar la URL
  char decoded_url[256];
  url_decode(decoded_url, route);
  LOG_DEBUG("Decoded URL: %s", decoded_url);

  // Rutas especiales: Home, Blog, Blog Entry, Page Entry
  if (strcmp(decoded_url, ROUTE_HOME) == 0 || strcmp(decoded_url, "") == 0) {
    const char *html_body = buildHomeWebSite(ROUTE_HOME, epoch, lang, theme);
    if (html_body == NULL) {
      const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 21\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Internal Server Error";
      LOG_ERROR("buildHomeWebSite returned NULL");
      connection_write(ctx, error_response, strlen(error_response));
    } else {
      char header[256];
      snprintf(header, sizeof(header),
               "Set-Cookie: lang=%s; path=/\r\n"
               "Set-Cookie: theme=%s; path=/\r\n",
               lang, theme);
      char *full_response = build_full_response(html_body, header);
      if (full_response != NULL) {
        LOG_DEBUG("Sending Home page response with cookies");
        connection_write(ctx, full_response, strlen(full_response));
        free(full_response);
      }
      free((void *)html_body);
    }
    connection_close(ctx);
    return;
  } else if (strcmp(decoded_url, ROUTE_BLOG) == 0) {
    const char *html_body = buildBlogWebSite(ROUTE_BLOG, epoch, lang, theme);
    if (html_body == NULL) {
      const char *error_response = "HTTP/1.1 500 Internal Server Error\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 21\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Internal Server Error";
      LOG_ERROR("buildBlogWebSite returned NULL");
      connection_write(ctx, error_response, strlen(error_response));
    } else {
      char header[256];
      snprintf(header, sizeof(header),
               "Set-Cookie: lang=%s; path=/\r\n"
               "Set-Cookie: theme=%s; path=/\r\n",
               lang, theme);
      char *full_response = build_full_response(html_body, header);
      if (full_response != NULL) {
        LOG_DEBUG("Sending Blog page response with cookies");
        connection_write(ctx, full_response, strlen(full_response));
        free(full_response);
      }
      free((void *)html_body);
    }
    connection_close(ctx);
    return;
  } else if (strncmp(decoded_url, ROUTE_BLOG_ENTRY, strlen(ROUTE_BLOG_ENTRY)) ==
             0) {
    const char *id = decoded_url + strlen(ROUTE_BLOG_ENTRY);
    if (strlen(id) == 0) {
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      LOG_WARN("Blog entry ID is empty");
      connection_write(ctx, error_response, strlen(error_response));
      connection_close(ctx);
      return;
    }
    const char *html_body =
        buildPageEntryWebSite(id, ROUTE_BLOG, epoch, lang, theme);
    if (html_body == NULL) {
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      LOG_WARN("Blog entry page not found for id: %s", id);
      connection_write(ctx, error_response, strlen(error_response));
    } else {
      char header[256];
      snprintf(header, sizeof(header),
               "Set-Cookie: lang=%s; path=/\r\n"
               "Set-Cookie: theme=%s; path=/\r\n",
               lang, theme);
      char *full_response = build_full_response(html_body, header);
      if (full_response != NULL) {
        LOG_DEBUG("Sending Blog entry response with cookies");
        connection_write(ctx, full_response, strlen(full_response));
        free(full_response);
      }
      free((void *)html_body);
    }
    connection_close(ctx);
    return;
  } else if (strncmp(decoded_url, ROUTE_PAGE_ENTRY, strlen(ROUTE_PAGE_ENTRY)) ==
             0) {
    const char *id = decoded_url + strlen(ROUTE_PAGE_ENTRY);
    if (strlen(id) == 0) {
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      LOG_WARN("Page entry ID is empty");
      connection_write(ctx, error_response, strlen(error_response));
      connection_close(ctx);
      return;
    }
    const char *html_body =
        buildPageEntryWebSite(id, decoded_url, epoch, lang, theme);
    if (html_body == NULL) {
      const char *error_response = "HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/plain\r\n"
                                   "Content-Length: 13\r\n"
                                   "Connection: close\r\n"
                                   "\r\n"
                                   "Not Found";
      LOG_WARN("Page entry not found for id: %s", id);
      connection_write(ctx, error_response, strlen(error_response));
    } else {
      char header[256];
      snprintf(header, sizeof(header),
               "Set-Cookie: lang=%s; path=/\r\n"
               "Set-Cookie: theme=%s; path=/\r\n",
               lang, theme);
      char *full_response = build_full_response(html_body, header);
      if (full_response != NULL) {
        LOG_DEBUG("Sending Page entry response with cookies");
        connection_write(ctx, full_response, strlen(full_response));
        free(full_response);
      }
      free((void *)html_body);
    }
    connection_close(ctx);
    return;
  }

  // Si la ruta no coincide con ninguna especial, se asume que es un recurso en
  // disco.
  char safe_path[MAX_PATH_LENGTH];
  if (!sanitize_path(decoded_url, safe_path, sizeof(safe_path),
                     root_directory)) {
    const char *forbidden_response =
        "HTTP/1.1 403 Forbidden\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 53\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>403 Forbidden</h1></body></html>";
    LOG_WARN("sanitize_path failed for URL: %s", decoded_url);
    connection_write(ctx, forbidden_response, strlen(forbidden_response));
    connection_close(ctx);
    return;
  }

  struct stat path_stat;
  if (stat(safe_path, &path_stat) < 0) {
    const char *not_found_response =
        "HTTP/1.1 404 Not Found\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 48\r\n"
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>404 Not Found</h1></body></html>";
    LOG_WARN("File not found: %s", safe_path);
    connection_write(ctx, not_found_response, strlen(not_found_response));
    connection_close(ctx);
    return;
  }

  if (S_ISDIR(path_stat.st_mode)) {
    char index_path[MAX_PATH_LENGTH];
    if (snprintf(index_path, sizeof(index_path), "%s/index.html", safe_path) >=
        (int)sizeof(index_path)) {
      LOG_ERROR("Path too long when building index_path");
      connection_close(ctx);
      return;
    }
    if (stat(index_path, &path_stat) == 0) {
      strncpy(safe_path, index_path, sizeof(safe_path) - 1);
      safe_path[sizeof(safe_path) - 1] = '\0';
    } else {
      if (snprintf(index_path, sizeof(index_path), "%s/index.htm", safe_path) >=
          (int)sizeof(index_path)) {
        LOG_ERROR("Path too long when building index_path");
        connection_close(ctx);
        return;
      }
      if (stat(index_path, &path_stat) == 0) {
        strncpy(safe_path, index_path, sizeof(safe_path) - 1);
        safe_path[sizeof(safe_path) - 1] = '\0';
      } else {
        // Si no se encuentra un index, listar el contenido del directorio.
        DIR *dir = opendir(safe_path);
        if (!dir) {
          const char *forbidden_response =
              "HTTP/1.1 403 Forbidden\r\n"
              "Content-Type: text/html\r\n"
              "Content-Length: 53\r\n"
              "Connection: close\r\n"
              "\r\n"
              "<html><body><h1>403 Forbidden</h1></body></html>";
          LOG_WARN("Failed to open directory: %s", safe_path);
          connection_write(ctx, forbidden_response, strlen(forbidden_response));
          connection_close(ctx);
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
          if (strcmp(entry->d_name, ".") == 0)
            continue;

          char item_path[MAX_PATH_LENGTH];
          char encoded_item_path[MAX_PATH_LENGTH];

          size_t decoded_len = strlen(decoded_url);
          if (strcmp(decoded_url, "/") == 0 || strcmp(decoded_url, "") == 0) {
            snprintf(item_path, sizeof(item_path), "/%s", entry->d_name);
          } else {
            if (decoded_url[decoded_len - 1] == '/')
              snprintf(item_path, sizeof(item_path), "%s%s", decoded_url,
                       entry->d_name);
            else
              snprintf(item_path, sizeof(item_path), "%s/%s", decoded_url,
                       entry->d_name);
          }

          url_encode(encoded_item_path, item_path, sizeof(encoded_item_path));

          char display_name[256];
          html_encode(display_name, entry->d_name, sizeof(display_name));

          char entry_full_path[MAX_PATH_LENGTH];
          if (snprintf(entry_full_path, sizeof(entry_full_path), "%s/%s",
                       safe_path,
                       entry->d_name) >= (int)sizeof(entry_full_path)) {
            LOG_ERROR("Path too long when building entry_full_path");
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
            LOG_ERROR("Response buffer full or error");
            break;
          }
          len += n;
        }
        closedir(dir);
        len += snprintf(response + len, sizeof(response) - len,
                        "</ul></body></html>");
        LOG_DEBUG("Sending directory listing for: %s", safe_path);
        connection_write(ctx, response, len);
        connection_close(ctx);
        return;
      }
    }
  }

  // Si el recurso es un archivo, se procede a leerlo y enviarlo.
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
    LOG_WARN("Failed to open file: %s", safe_path);
    connection_write(ctx, not_found_response, strlen(not_found_response));
    connection_close(ctx);
    return;
  }
  struct stat file_stat;
  if (fstat(file_fd, &file_stat) < 0) {
    perror("Failed to get file stats");
    close(file_fd);
    connection_close(ctx);
    return;
  }
  off_t file_size = file_stat.st_size;
  const char *mime_type = get_mime_type(safe_path);

  char header[256];
  int header_len = snprintf(header, sizeof(header),
                            "HTTP/1.1 200 OK\r\n"
                            "Content-Type: %s\r\n"
                            "Content-Length: %" PRIdMAX "\r\n"
                            "Connection: close\r\n"
                            "\r\n",
                            mime_type, (intmax_t)file_size);
  LOG_DEBUG("Sending file: %s, size: %jd bytes", safe_path,
            (intmax_t)file_size);
  connection_write(ctx, header, header_len);

  ssize_t sent_bytes, read_bytes;
  while ((read_bytes = read(file_fd, buffer, sizeof(buffer))) > 0) {
    sent_bytes = connection_write(ctx, buffer, read_bytes);
    if (sent_bytes < 0) {
      perror("Error sending file");
      break;
    }
  }

  close(file_fd);
  connection_close(ctx);
}