#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"

const char* container() {
    char *container_content = read_file_to_string("./html/container.html");

    return container_content;
}
