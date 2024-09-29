#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"

const char* menu() {
    const char *menu_response =  read_file_to_string("./html/menu.html");
    
    return menu_response;
}
