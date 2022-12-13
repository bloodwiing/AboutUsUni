/// DYNAMIC HTML GENERATOR
/// BLOODWIING © 2022
/// GITHUB: https://github.com/bloodwiing

#ifndef HTML_DYNAMIC_H
#define HTML_DYNAMIC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "hashmap.h"


// ---- STRUCTS -----

/// A Placeholder that remembers where value could be inserted to
typedef struct DynaPlaceholder {
    char *name; // The name or key of the placeholder
    uint64_t pos; // The position in the de-formatted string where the Placeholder is
} DynaPlaceholder;

typedef struct DynaElement {
    char *original; // The original or de-formatted string (with placeholders removed)

    struct hashmap *key_map; // The HashMap that contains all replacement values given via `putDynamicData()`

    int placeholder_count; // The total count of all Placeholders found
    DynaPlaceholder *placeholders; // The dynamic array of Placeholders

    char *result; // The generated result string, created by using `generateDynamicString()`
} DynaElement;


// ----- FUNCTIONS -----

/// Calculates the size of the format with all the data applied
/// \param format The format or string
/// \param arg_list The Variable Argument List that gets inserted into the format
/// \return The size of the calculated string
size_t getFormattedStringSize(char *format, va_list arg_list);

/// Creates a new Dynamic HTML Document
/// \param file The open file that will be read
/// \return The pointer to a Dynamic Element that will be used for everything
DynaElement * createDynamicDocument(FILE *file);

/// Provides a replacement value for a single Placeholder that was found within the Dynamic Element
/// \param element The Dynamic Element where to replace a Placeholder
/// \param key The name or key of the Placeholder
/// \param format The format of the string which will replace the Placeholder
/// \param ... Extra data to fill the format
void putDynamicData(DynaElement *element, char *key, char *format, ...);

/// Create the final string with all Placeholders replaced
/// \param element The Dynamic Element to generate the string from
/// \return The generated final string
char * generateDynamicString(DynaElement *element);

/// De-allocates the memory used up by the Dynamic Element
/// \param element The Element to free up
void freeDynamicElement(DynaElement *element);

#endif //HTML_DYNAMIC_H
