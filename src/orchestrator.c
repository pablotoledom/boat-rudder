#define _XOPEN_SOURCE 700 // Define POSIX.1-2008 compliance level

#include "include/log.h"
#include "modules/container/container.h"
#include "modules/home_blog/home_blog.h"
#include "modules/menu/menu.h"
#include "modules/page/page.h"
#include "modules/mainbanner/mainbanner.h"
#include <stdio.h>   // For snprintf
#include <stdlib.h>  // For malloc and free
#include <string.h>  // For strlen

/**
 * Generates the HTML content for the home page.
 * Returns a dynamically allocated string with the HTML (without HTTP headers)
 * or NULL in case of error.
 */
const char *buildHomeWebSite(char *decoded_url, int epoch, char *lang, char *theme) {
  LOG_INFO("Building Home Website: decoded_url=%s, epoch=%i, Language=%s, Theme=%s", decoded_url, epoch, lang, theme);
  // Get the HTML content from each module
  const char *html_container = container(epoch);
  const char *html_menu = menu(decoded_url, epoch, lang, theme);
  const char *html_mainbanner = mainbanner(epoch);
  const char *html_home_blog = home_blog(epoch);
  const char *html_page = page("/", epoch);

  // Verify that none of the components are NULL
  if (!html_container || !html_menu || !html_mainbanner || !html_page || !html_home_blog) {
    LOG_DEBUG("Error in buildHomeWebSite: One or more HTML components are NULL.");
    if (html_container) free((void *)html_container);
    if (html_menu) free((void *)html_menu);
    if (html_mainbanner) free((void *)html_mainbanner);
    if (html_page) free((void *)html_page);
    if (html_home_blog) free((void *)html_home_blog);
    return NULL;
  }

  // Calculate the total length needed for the formatted string
  size_t total_length = snprintf(NULL, 0, html_container, html_menu, html_mainbanner, html_page, html_home_blog) + 1;
  LOG_DEBUG("Total length to allocate: %zu", total_length);

  // Allocate memory for the complete HTML
  char *buffer = malloc(total_length);
  if (buffer == NULL) {
    perror("Failed to allocate memory");
    free((void *)html_container);
    free((void *)html_menu);
    free((void *)html_mainbanner);
    free((void *)html_page);
    free((void *)html_home_blog);
    return NULL;
  }

  // Build the formatted HTML
  snprintf(buffer, total_length, html_container, html_menu, html_mainbanner, html_page, html_home_blog);

  // Free temporary strings
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_mainbanner);
  free((void *)html_page);
  free((void *)html_home_blog);

  LOG_DEBUG("Home page HTML generated successfully");
  return buffer;
}

/**
 * Generates the HTML content for the blog page.
 * Returns a dynamically allocated string with the HTML (without HTTP headers)
 * or NULL in case of error.
 */
const char *buildBlogWebSite(char *decoded_url, int epoch, char *lang, char *theme) {
  // Get the HTML content from each module
  const char *html_container = container(epoch);
  const char *html_menu = menu(decoded_url, epoch, lang, theme);
  const char *html_home_blog = home_blog(epoch);

  if (!html_container || !html_menu || !html_home_blog) {
    LOG_DEBUG("Error in buildBlogWebSite: One or more HTML components are NULL.");
    if (html_container) free((void *)html_container);
    if (html_menu) free((void *)html_menu);
    if (html_home_blog) free((void *)html_home_blog);
    return NULL;
  }

  // Format the HTML using empty strings for components that are not required
  size_t total_length = snprintf(NULL, 0, html_container, html_menu, "", html_home_blog, "") + 1;
  LOG_DEBUG("Total length to allocate: %zu", total_length);

  char *buffer = malloc(total_length);
  if (buffer == NULL) {
    perror("Failed to allocate memory");
    free((void *)html_container);
    free((void *)html_menu);
    free((void *)html_home_blog);
    return NULL;
  }

  snprintf(buffer, total_length, html_container, html_menu, "", html_home_blog, "");
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_home_blog);

  LOG_DEBUG("Blog page HTML generated successfully");
  return buffer;
}

/**
 * Generates the HTML content for a page or blog entry.
 * Returns a dynamically allocated string with the HTML (without HTTP headers)
 * or NULL in case of error.
 */
const char *buildPageEntryWebSite(const char *id, char *url, int epoch, char *lang, char *theme) {
  // Get the HTML content from each module
  const char *html_container = container(epoch);
  const char *html_menu = menu(url, epoch, lang, theme);
  const char *html_page = page(id, epoch);

  if (!html_container || !html_menu || !html_page) {
    LOG_DEBUG("Error in buildPageEntryWebSite: One or more HTML components are NULL.");
    if (html_container) free((void *)html_container);
    if (html_menu) free((void *)html_menu);
    if (html_page) free((void *)html_page);
    return NULL;
  }

  size_t total_length = snprintf(NULL, 0, html_container, html_menu, "", html_page, "") + 1;
  LOG_DEBUG("Total length to allocate: %zu", total_length);

  char *buffer = malloc(total_length);
  if (buffer == NULL) {
    perror("Failed to allocate memory");
    free((void *)html_container);
    free((void *)html_menu);
    free((void *)html_page);
    return NULL;
  }

  snprintf(buffer, total_length, html_container, html_menu, "", html_page, "");
  free((void *)html_container);
  free((void *)html_menu);
  free((void *)html_page);

  LOG_DEBUG("Page entry HTML generated successfully for id: %s", id);
  return buffer;
}
