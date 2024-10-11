#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"
#include "../../include/generate_url_theme.h"
#include <stdlib.h>

const char *slider(int epoch) {
    char *filename_slider_html = generate_url_theme("slider/slider_epoch%d.html", epoch);
    const char *slider_response =  read_file_to_string(filename_slider_html);
    free(filename_slider_html);
    
    return slider_response;
}