#ifndef SOURCE_LOCATION_H
#define SOURCE_LOCATION_H

#include <stddef.h>

typedef struct {
    size_t line;
} SourceLocation;

static inline SourceLocation source_location_make(size_t line)
{
    SourceLocation location = {line};
    return location;
}

#endif
