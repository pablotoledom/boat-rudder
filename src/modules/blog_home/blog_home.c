#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"

const char* blog_home() {
    const char *blog_home_response =  read_file_to_string("./html/blog-home.html");
    
    return blog_home_response;
}