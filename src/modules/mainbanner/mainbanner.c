#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/read_file.h"
#include "../../include/generate_url_theme.h"
#include <stdlib.h>

const char *mainbanner(int epoch) {
    char *filename_mainbanner_html = generate_url_theme("mainbanner/mainbanner_epoch%d.html", epoch);
    const char *mainbanner_response =  read_file_to_string(filename_mainbanner_html);
    free(filename_mainbanner_html);
    
    return mainbanner_response;
}