#ifndef VERSION_H
#define VERSION_H

// The running version: CMakeLists.txt defines BOAT_RUDDER_VERSION from the
// VERSION file at the root of the source. `boat-rudder --version`, the
// startup log, Site settings and /dashboard/settings/updates print it.
#ifndef BOAT_RUDDER_VERSION
#define BOAT_RUDDER_VERSION "unknown"
#endif

#endif // VERSION_H
