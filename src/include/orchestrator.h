#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

const char* buildHomeWebSite(const char *base_url, int epoch, char *lang, char *theme);
const char* buildBlogWebSite(char *base_url, int epoch, char *lang, char *theme);
const char* buildPageEntryWebSite(const char *id, char *base_url, int epoch, char *lang, char *theme);

#endif // ORCHESTRATOR_H
