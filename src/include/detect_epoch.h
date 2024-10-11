#ifndef DETECT_EPOCH_H
#define DETECT_EPOCH_H

// Define the epochs
#define EPOCH_EARLY 1
#define EPOCH_MIDDLE 2
#define EPOCH_MODERN 3

// Function declaration to detect browser epoch based on User-Agent
int detect_epoch(const char *user_agent);

#endif // DETECT_EPOCH_H
