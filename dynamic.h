#ifndef HTML_DYNAMIC_H
#define HTML_DYNAMIC_H

#include <stdarg.h>
#include <stdio.h>
#include <stdint.h>
#include "hashmap.h"

typedef struct DynaKey {
    char *key;
    char *value;
} DynaKey;

typedef struct DynaPlaceholder {
    char *name;
    uint64_t pos;
} DynaPlaceholder;

typedef struct DynaElement {
    char *original;

    struct hashmap *key_map;
    struct hashmap *elem_map;

    int placeholder_count;
    DynaPlaceholder *placeholders;
} DynaElement;

DynaElement * createDynamicDocument(FILE *file);
void putDynamicData(DynaElement *element, char *key, char *format, ...);
char * generateDynamicString(DynaElement *element);

#endif //HTML_DYNAMIC_H
