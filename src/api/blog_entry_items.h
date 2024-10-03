#ifndef BLOG_ENTRY_ITEMS_H
#define BLOG_ENTRY_ITEMS_H

#define MAX_ID_LENGTH 50
#define MAX_CONTENT_ID_LENGTH 50
#define MAX_TYPE_LENGTH 30
#define MAX_CONTENT_LENGTH 50000
#define MAX_EXTRA_DATA_LENGTH 50000

typedef struct {
    char entry_id[MAX_ID_LENGTH];
    char content_id[MAX_CONTENT_ID_LENGTH];
    char type[MAX_TYPE_LENGTH];
    char content[MAX_CONTENT_LENGTH];
    char extra_data[MAX_EXTRA_DATA_LENGTH];
} BlogEntryItems;

// Function prototype const char *EntryId,
BlogEntryItems *getBlogEntryItems(const char *EntryId, int *filteredCount);

#endif // BLOG_ENTRY_ITEMS_H