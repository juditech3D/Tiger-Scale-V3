// pod_reader.h -- one of the scale's readers as Tiger Studio sees it, for the
// TigerPOD tool (§9 of the .ino).
//
// A header rather than a struct in the .ino: the Arduino converter writes the
// prototypes of the .ino's functions above its first line, and the functions
// that take a PodReader would then name a type that does not exist yet.
#pragma once
#include <Arduino.h>

class PN532Reader;

struct PodReader {
    PN532Reader *r;
    const char  *id;            // "1" / "2", what Studio names it by
    bool         present;
    uint8_t      uid[10];
    uint8_t      uidLen;
    uint8_t      misses;
    String       pagesHex;      // pages 4-39 as last read, "" if unreadable
};
