#include <stdlib.h>

#include "parser.tab.h"

int main(void)
{
    SourceLocation location = source_location_make(7);

    if (location.line != 7
        || KW_INT == IDENTIFIER
        || IDENTIFIER == SEMICOLON) {
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
