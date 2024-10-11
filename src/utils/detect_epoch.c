#include <string.h>
#include <stdlib.h>

// Define the epochs
#define EPOCH_EARLY 1
#define EPOCH_MIDDLE 2
#define EPOCH_MODERN 1

// Function to detect the browser epoch based on the User-Agent
int detect_epoch(const char *user_agent) {
    // Detect Internet Explorer
    if (strstr(user_agent, "MSIE")) {
        // Extract MSIE version
        char *version_str = strstr(user_agent, "MSIE");
        int version = atoi(version_str + 5); // Skip "MSIE "
        
        if (version <= 4) {
            return EPOCH_EARLY; // Internet Explorer 4 or earlier
        } else if (version >= 5 && version <= 8) {
            return EPOCH_MIDDLE; // Internet Explorer 5 to 8
        } else {
            return EPOCH_MODERN; // Internet Explorer 9 or later
        }
    }
    
    // Detect Trident-based browsers (IE 11 and newer IE versions)
    if (strstr(user_agent, "Trident")) {
        return EPOCH_MODERN; // Trident is the rendering engine for IE 11+
    }
    
    // Detect Gecko-based browsers (like Firefox)
    if (strstr(user_agent, "Firefox")) {
        char *version_str = strstr(user_agent, "Firefox/");
        int version = atoi(version_str + 8); // Skip "Firefox/"
        
        if (version < 4) {
            return EPOCH_MIDDLE; // Old Firefox (before Firefox 4)
        } else {
            return EPOCH_MODERN; // Firefox 4 or later
        }
    }
    
    // Detect WebKit-based browsers (like Chrome and Safari)
    if (strstr(user_agent, "Chrome")) {
        char *version_str = strstr(user_agent, "Chrome/");
        int version = atoi(version_str + 7); // Skip "Chrome/"
        
        if (version < 10) {
            return EPOCH_MIDDLE; // Old Chrome (before Chrome 10)
        } else {
            return EPOCH_MODERN; // Chrome 10 or later
        }
    }
    
    if (strstr(user_agent, "Safari") && !strstr(user_agent, "Chrome")) {
        // Safari without Chrome (because Chrome also has "Safari" in its User-Agent)
        char *version_str = strstr(user_agent, "Version/");
        int version = atoi(version_str + 8); // Skip "Version/"
        
        if (version < 5) {
            return EPOCH_MIDDLE; // Old Safari
        } else {
            return EPOCH_MODERN; // Safari 5 or later
        }
    }
    
    // If no known browser is found, assume modern epoch as fallback
    return EPOCH_EARLY;
}
