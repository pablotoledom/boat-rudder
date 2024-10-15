#include "../../include/code_formatter/highlight_code.h"
#include "../../include/code_formatter/highlight_basic.h" // Agregamos esta línea
#include "../../include/code_formatter/highlight_c.h"
#include "../../include/code_formatter/highlight_code.h"
#include "../../include/code_formatter/highlight_css.h"
#include "../../include/code_formatter/highlight_html.h"
#include "../../include/code_formatter/highlight_js.h"
#include "../../include/code_formatter/highlight_php.h"
#include "../../include/code_formatter/highlight_prolog.h" // Agregamos esta línea
#include "../../include/code_formatter/highlight_python.h"
#include <ctype.h>
#include <string.h>

typedef enum {
  LANG_UNKNOWN,
  LANG_JAVASCRIPT,
  LANG_HTML,
  LANG_C,
  LANG_PYTHON,
  LANG_CSS,
  LANG_PHP,
  LANG_BASIC, // Agregamos esta línea
  LANG_PROLOG // Agregamos esta línea
} Language;

Language detect_language(const char *code) {
  const char *p = code;
  // Saltar líneas en blanco
  while (*p == '\n' || *p == '\r') {
    p++;
  }
  // Leer la primera línea
  char line[256];
  int i = 0;
  while (*p && *p != '\n' && *p != '\r' && i < sizeof(line) - 1) {
    line[i++] = *p++;
  }
  line[i] = '\0';

  // Convertir a minúsculas para comparación
  char lower_line[256];
  for (i = 0; line[i]; i++) {
    lower_line[i] = tolower(line[i]);
  }
  lower_line[i] = '\0';

  if (strstr(lower_line, "javascript lang")) {
    return LANG_JAVASCRIPT;
  } else if (strstr(lower_line, "html lang")) {
    return LANG_HTML;
  } else if (strstr(lower_line, "c lang")) {
    return LANG_C;
  } else if (strstr(lower_line, "python lang")) {
    return LANG_PYTHON;
  } else if (strstr(lower_line, "css lang")) {
    return LANG_CSS;
  } else if (strstr(lower_line, "php lang")) {
    return LANG_PHP;
  } else if (strstr(lower_line, "basic lang")) { // Agregamos esta condición
    return LANG_BASIC;
  } else if (strstr(lower_line, "prolog lang")) { // Agregamos esta condición
    return LANG_PROLOG;
  } else {
    return LANG_UNKNOWN;
  }
}

char *highlight_code(const char *code) {
  // Detectar el lenguaje
  Language lang = detect_language(code);

  // Eliminar la primera línea (la línea de lenguaje)
  const char *code_without_first_line = strchr(code, '\n');
  if (code_without_first_line) {
    code_without_first_line++; // Saltar el salto de línea
  } else {
    code_without_first_line =
        code; // Si no hay salto de línea, usar el código original
  }

  // Llamar a la función de resaltado correspondiente
  switch (lang) {
  case LANG_JAVASCRIPT:
    return highlight_js(code_without_first_line);
  case LANG_HTML:
    return highlight_html(code_without_first_line);
  case LANG_C:
    return highlight_c(code_without_first_line);
  case LANG_PYTHON:
    return highlight_python(code_without_first_line);
  case LANG_CSS:
    return highlight_css(code_without_first_line);
  case LANG_PHP:
    return highlight_php(code_without_first_line);
  case LANG_BASIC: // Agregamos esta línea
    return highlight_basic(code_without_first_line);
  case LANG_PROLOG: // Agregamos esta línea
    return highlight_prolog(code_without_first_line);
  default:
    // Si el lenguaje es desconocido, devolver el código sin cambios
    return strdup(code_without_first_line);
  }
}
