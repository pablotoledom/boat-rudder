#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

const char* buildWebSite() {
    const char *root_response =
        "HTTP/1.1 200 OK\r\n"
        "Content-Type: text/html\r\n"
        "Content-Length: 54\r\n" // Length of the HTML content below
        "Connection: close\r\n"
        "\r\n"
        "<html><body><h1>Welcome to my  site</h1></body></html>";

    return root_response;
}
