#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "../../include/generate_url_theme.h"
#include "../../include/read_file.h"
#include <stdlib.h>

const char *container(int epoch) {
  char *filename_container_html =
      generate_url_theme("container/container_epoch%d.html", epoch);
  char *container_content = read_file_to_string(filename_container_html);
  free(filename_container_html);

  return container_content;
}
