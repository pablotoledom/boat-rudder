#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "include/config_loader.h"
#include "include/request_handler.h"
#include <arpa/inet.h>
#include <errno.h>
#include <limits.h> // For PATH_MAX
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h> // For realpath, malloc, free
#include <string.h>
#include <sys/wait.h> // For waitpid
#include <unistd.h>

// Structure to pass arguments to the thread function
struct thread_args {
  int client_socket;
  const char *root_directory;
};

// Signal handler to reap zombie processes
void sigchld_handler(int s) {
  (void)s; // Unused parameter
  int saved_errno = errno;
  while (waitpid(-1, NULL, WNOHANG) > 0)
    ;
  errno = saved_errno;
}

// Thread function to handle requests
void *thread_function(void *arg) {
  struct thread_args *args = arg;
  int client_socket = args->client_socket;
  const char *root_directory = args->root_directory;
  free(args);

  handle_request(client_socket, root_directory);
  return NULL;
}

int main(int argc, char *argv[]) {

  // Load the configuration file
  if (load_config("./configs/config.txt") == 0) {
    if (verbose) {
      // Show loaded configurations
      printf("Verbose mode is on.\n\n");

      printf("Verbose: %s\n", verbose ? "true" : "false");
      printf("Server Port: %d\n", server_port);
      printf("Spreadsheet ID: %s\n", spreadsheet_id);
      printf("API Key: %s\n\n", api_key);
    } else {
      printf("Verbose mode is off.\n\n");
    }
  } else {
    printf("Error loading configuration.\n");
  }

  int server_fd, client_socket;
  struct sockaddr_in address;
  int opt = 1;
  socklen_t addrlen = sizeof(address);

  // Check arguments
  if (argc != 2) {
    fprintf(stderr, "Usage: %s <root_directory>\n", argv[0]);
    exit(EXIT_FAILURE);
  }

  // Resolve the root directory
  char resolved_path[PATH_MAX];
  if (realpath(argv[1], resolved_path) == NULL) {
    perror("Invalid root directory");
    exit(EXIT_FAILURE);
  }
  char *root_directory = strdup(resolved_path);
  if (root_directory == NULL) {
    perror("Failed to allocate memory for root_directory");
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
  address.sin_port = htons(server_port);

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

  printf("Static HTTP server, listening on port %d\n", server_port);
  printf("Root directory: %s\n", root_directory);

  while (1) {
    // Accept an incoming connection
    client_socket = accept(server_fd, (struct sockaddr *)&address, &addrlen);
    if (client_socket < 0) {
      perror("Error on accept");
      continue;
    }

    // Handle the request in a new thread
    struct thread_args *args = malloc(sizeof(struct thread_args));
    if (!args) {
      perror("Failed to allocate memory");
      close(client_socket);
      continue;
    }
    args->client_socket = client_socket;
    args->root_directory = root_directory;

    pthread_t thread_id;
    if (pthread_create(&thread_id, NULL, thread_function, (void *)args) != 0) {
      perror("Could not create thread");
      close(client_socket);
      free(args);
      continue;
    }

    pthread_detach(
        thread_id); // Resources will be cleaned up when the thread exits
  }

  free(root_directory);
  close(server_fd);
  return 0;
}
