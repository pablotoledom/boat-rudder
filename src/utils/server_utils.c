#define _XOPEN_SOURCE 700  // Define POSIX.1-2008 compliance level

#include "../include/server_utils.h"
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <limits.h>
#include <stdlib.h>
#include <unistd.h>

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

int sanitize_path(const char *url_path, char *safe_path, size_t size, const char *root_directory) {
    char path[PATH_MAX];
    if (snprintf(path, sizeof(path), "%s%s", root_directory, url_path) >= (int)sizeof(path)) {
        // Path too long
        return 0;
    }

    char resolved_path[PATH_MAX];
    if (realpath(path, resolved_path) == NULL) {
        // Invalid path
        return 0;
    } else {
        // Ensure the resolved path starts with the root directory
        size_t root_len = strlen(root_directory);
        if (strncmp(resolved_path, root_directory, root_len) == 0) {
            if (strlen(resolved_path) >= size) {
                // Safe path buffer too small
                return 0;
            }
            strcpy(safe_path, resolved_path);
            return 1;
        } else {
            // Attempted access outside root directory
            return 0;
        }
    }
}
