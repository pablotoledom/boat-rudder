#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define RESPONSE_SIZE 2048

int main() {
    int server_fd, new_socket;
    struct sockaddr_in address;
    int opt = 1;
    int addrlen = sizeof(address);
    char buffer[BUFFER_SIZE] = {0};
    char response[RESPONSE_SIZE];

    const char *hello_response = "HTTP/1.1 200 OK\nContent-Type: text/html\nConnection: keep-alive\n\n<html><body><h1>Hello, World!</h1></body></html>";

    // Creating socket file descriptor
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

    // Forcefully attaching socket to the port 8080
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons(PORT);

    // Forcefully attaching socket to the port 8080
    if (bind(server_fd, (struct sockaddr *)&address, sizeof(address)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    if (listen(server_fd, 3) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    printf("Server listening on port %d...\n", PORT);

    while(1) {
        if ((new_socket = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0) {
            perror("accept");
            exit(EXIT_FAILURE);
        }

        printf("New connection established.\n");

        int bytes_received = recv(new_socket, buffer, BUFFER_SIZE, 0);
        if (bytes_received < 0) {
            perror("recv");
            exit(EXIT_FAILURE);
        } else if (bytes_received == 0) {
            printf("Connection closed by client.\n");
            close(new_socket);
            continue;
        }

        printf("Received request:\n%s\n", buffer);

        // Prepare the response
        snprintf(response, RESPONSE_SIZE, "%s", hello_response);

        // Send the response
        if (send(new_socket, response, strlen(response), 0) < 0) {
            perror("send");
            exit(EXIT_FAILURE);
        }

        printf("Response sent\n");

        // Clear the buffer and reset the response
        memset(buffer, 0, BUFFER_SIZE);
        memset(response, 0, RESPONSE_SIZE);

        // Close the connection
        close(new_socket);
        printf("Connection closed.\n");
    }

    return 0;
}
