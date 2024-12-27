#ifndef ORCHESTRATOR_H
#define ORCHESTRATOR_H

const char* buildHomeWebSite(const char *base_url, int epoch);
const char* buildBlogWebSite(char *base_url, int epoch);
const char* buildBlogEntryWebSite(const char *id, char *base_url, int epoch);

#endif // ORCHESTRATOR_H
