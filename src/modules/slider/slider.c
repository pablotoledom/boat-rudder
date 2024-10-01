#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"

const char* slider() {
    const char *slider_response =  read_file_to_string("./html/slider/slider_std2.html");
    
    return slider_response;
}