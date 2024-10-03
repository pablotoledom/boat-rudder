#ifndef HOME_BLOG_ITEMS_H
#define HOME_BLOG_ITEMS_H

#define MAX_ID_LENGTH 64
#define MAX_URL_LENGTH 256
#define MAX_IMAGE_URL_LENGTH 256
#define MAX_TITLE_LENGTH 128
#define MAX_SUMMARY_LENGTH 512

typedef struct {
    char id[MAX_ID_LENGTH];
    char url[MAX_URL_LENGTH];
    char image_url[MAX_IMAGE_URL_LENGTH];
    char tittle[MAX_TITLE_LENGTH];
    char summary[MAX_SUMMARY_LENGTH];
} HomeBlogItems;

// Function prototype
HomeBlogItems *getBlogItems(int *routeCount);

#endif // HOME_BLOG_ITEMS_H
