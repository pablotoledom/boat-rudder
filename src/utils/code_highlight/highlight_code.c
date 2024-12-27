#include "../../include/code_highlight/highlight_code.h"
#include "../../include/code_highlight/highlight_basic.h"
#include "../../include/code_highlight/highlight_c.h"
#include "../../include/code_highlight/highlight_code.h"
#include "../../include/code_highlight/highlight_css.h"
#include "../../include/code_highlight/highlight_html.h"
#include "../../include/code_highlight/highlight_js.h"
#include "../../include/code_highlight/highlight_php.h"
#include "../../include/code_highlight/highlight_prolog.h"
#include "../../include/code_highlight/highlight_python.h"
#include "../../include/code_highlight/highlight_bash.h"
#include "../../include/code_highlight/highlight_bat.h"
#include <ctype.h>
#include <string.h>

// Enumeration for supported programming languages
typedef enum {
  LANG_UNKNOWN, // Unknown language
  LANG_JAVASCRIPT,
  LANG_HTML,
  LANG_C,
  LANG_PYTHON,
  LANG_CSS,
  LANG_PHP,
  LANG_BASIC,
  LANG_PROLOG,
  LANG_BASH,
  LANG_BAT
} Language;

// Function to detect the programming language from the code's first line
Language detect_language(const char *code) {
  const char *p = code;

  // Skip any blank lines at the start
  while (*p == '\n' || *p == '\r') {
    p++;
  }

  // Read the first non-blank line
  char line[256];
  int i = 0;
  while (*p && *p != '\n' && *p != '\r' && i < (int)(sizeof(line) - 1)) {
    line[i++] = *p++;
  }
  line[i] = '\0'; // Null-terminate the line

  // Convert the line to lowercase for comparison
  char lower_line[256];
  for (i = 0; line[i]; i++) {
    lower_line[i] = tolower(line[i]);
  }
  lower_line[i] = '\0'; // Null-terminate the lowercase line

  // Check for specific language identifiers in the line
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
  } else if (strstr(lower_line, "basic lang")) {
    return LANG_BASIC;
  } else if (strstr(lower_line, "prolog lang")) {
    return LANG_PROLOG;
  } else if (strstr(lower_line, "#!/bin/bash")) {
    return LANG_BASH;
  } else if (strstr(lower_line, "bat script")) {
    return LANG_BAT;
  } else {
    return LANG_UNKNOWN; // Return unknown if no match is found
  }
}

// Function to highlight code based on its detected language
char *highlight_code(const char *code) {
  // Detect the language from the code
  Language lang = detect_language(code);

  // Remove the first line (the language identifier line)
  const char *code_without_first_line = strchr(code, '\n');
  if (code_without_first_line) {
    code_without_first_line++; // Skip the newline character
  } else {
    code_without_first_line = code; // If no newline, use the original code
  }

  // Call the appropriate highlighting function based on the detected language
  switch (lang) {
  case LANG_JAVASCRIPT:
    return highlight_js(code_without_first_line); // Highlight JavaScript code
  case LANG_HTML:
    return highlight_html(code_without_first_line); // Highlight HTML code
  case LANG_C:
    return highlight_c(code_without_first_line); // Highlight C code
  case LANG_PYTHON:
    return highlight_python(code_without_first_line); // Highlight Python code
  case LANG_CSS:
    return highlight_css(code_without_first_line); // Highlight CSS code
  case LANG_PHP:
    return highlight_php(code_without_first_line); // Highlight PHP code
  case LANG_BASIC: // Added case for BASIC language
    return highlight_basic(code_without_first_line); // Highlight BASIC code
  case LANG_PROLOG: // Added case for Prolog language
    return highlight_prolog(code_without_first_line); // Highlight Prolog code
  case LANG_BASH: // Added case for Prolog language
    return highlight_bash(code_without_first_line); // Highlight Prolog code
  case LANG_BAT: // Added case for Prolog language
    return highlight_bat(code_without_first_line); // Highlight Prolog code
  default:
    // If the language is unknown, return the code without any changes
    return strdup(code_without_first_line);
  }
}
