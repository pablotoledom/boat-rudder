#ifndef UTILS_H
#define UTILS_H

#include <stddef.h>

const char *get_mime_type(const char *path);
void url_encode(char *dst, const char *src, size_t dst_size);
void url_decode(char *dst, const char *src);
void html_encode(char *dst, const char *src, size_t dst_size);
int sanitize_path(const char *url_path, char *safe_path, size_t size, const char *root_directory);

#endif // UTILS_H
