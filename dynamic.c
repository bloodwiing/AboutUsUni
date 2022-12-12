#include "dynamic.h"
#include <string.h>
#include <stdlib.h>

int compareDynaKey(const void *a, const void *b, void *udata) {
    const DynaKey *da = a;
    const DynaKey *db = b;
    return strcmp(da->key, db->key);
}

uint64_t hashDynaKey(const void *item, uint64_t seed0, uint64_t seed1) {
    const DynaKey *dyna_key = item;
    return hashmap_sip(dyna_key->key, strlen(dyna_key->key), seed0, seed1);
}

void addDynamicElementPlaceholder(DynaElement *element, char *placeholder, uint64_t placeholder_size, uint64_t position) {
    element->placeholders = realloc(element->placeholders, sizeof(DynaPlaceholder) * ++element->placeholder_count);

    DynaPlaceholder *item = &element->placeholders[element->placeholder_count - 1];
    item->name = calloc(placeholder_size + 1, 1);
    strncpy(item->name, placeholder, placeholder_size);

    item->pos = position;
}

void analyseDynamicElement(DynaElement *element) {
    char *cursor = element->original;
    size_t a = strlen(element->original) + 1;
    char *final = calloc(strlen(element->original) + 1, 1);

    char *placeholder_start = NULL, *placeholder_end = element->original;
    uint64_t placeholder_size = 0;

    uint64_t position = 0, last_placeholder = 0;
    uint64_t placeholder_offset = 0;
    while (*cursor) {
        // if the HTML is at a { now
        if (*cursor == '{') {
            // AND hasn't been marked as a PLACEHOLDER
            if (!placeholder_start) {
                // put the string up to the placeholder into the final and mark this as a placeholder
                strncat(final, placeholder_end, position - last_placeholder);
                placeholder_size = 0;
                placeholder_start = cursor + 1;
            // if it HAS been marked as a PLACEHOLDER already
            } else {
                // if it already was a PLACEHOLDER - then it must be an ESCAPE sequence ( "{{" )
                // so just put it in the final string and cancel the PLACEHOLDER flag
                placeholder_start = NULL;
                // set up the gap for other escape sequences
                placeholder_end = cursor + 1;
                last_placeholder = position + 1;
                // and finally the { itself
                strcat(final, "{");
                // increase offset
                ++placeholder_offset;
            }
        }

        // if the HTML is at a } now
        else if (*cursor == '}') {
            // AND the PLACEHOLDER has already been started
            if (placeholder_start) {
                // save the current PLACEHOLDER to the element
                // and record where the placeholder ended for later string concatenation
                addDynamicElementPlaceholder(element, placeholder_start, placeholder_size, position - placeholder_size - placeholder_offset - 1);
                placeholder_end = cursor + 1;
                last_placeholder = position + 1;
                placeholder_start = NULL;
                placeholder_offset += placeholder_size + 2;
            // if the PLACEHOLDER has NOT been marked
            } else {
                // if there's a DOUBLE bracket ( like an escape sequence "}}" )
                // then we skip ahead by one
                if (*(cursor+1) == '}')
                    ++cursor;
                // then we just put everything up to the } like normal
                strncat(final, placeholder_end, position - last_placeholder);
                placeholder_end = cursor + 1;
                last_placeholder = position + 1;
                // put only one "}" into the string
                strcat(final, "}");
            }
        }

        // ANY OTHER character
        else {
            // BUT still inside of a PLACEHOLDER
            if (placeholder_start) {
                // continue recording the size of the PLACEHOLDER name
                ++placeholder_size;
            }
        }

        // otherwise resume the loop and counter
        ++position;
        ++cursor;
    }

    // to finish the string do a FINAL CONCATENATION from the last PLACEHOLDER to the end of the string
    strncat(final, placeholder_end, position - last_placeholder);

    // save the element's new string
    element->original = final;
}

DynaElement *createDynamicDocument(FILE *file) {
    DynaElement *out = calloc(1, sizeof(DynaElement));

    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    out->original = calloc(size + 1, 1);
    fread(out->original, 1, size, file);

    analyseDynamicElement(out);

    out->key_map = hashmap_new(sizeof(DynaKey), 0, 0, 0, hashDynaKey, compareDynaKey, NULL, NULL);

    return out;
}

void putDynamicData(DynaElement *element, char *key, char *format, ...) {
    DynaKey *out = calloc(1, sizeof(DynaKey));

    out->key = malloc(strlen(key) + 1);
    strcpy(out->key, key);

    va_list va;
    va_start(va, format);
    int count = vsnprintf(NULL, 0, format, va) + 1;
    va_end(va);

    out->value = calloc(count, 1);
    va_start(va, format);
    vsnprintf(out->value, count, format, va);
    va_end(va);

    hashmap_set(element->key_map, out);
}

char *generateDynamicString(DynaElement *element) {
    uint64_t size = strlen(element->original) + 1;

    for (int i = 0; i < element->placeholder_count; i++) {
        DynaPlaceholder placeholder = element->placeholders[i];
        DynaKey *key = hashmap_get(element->key_map, &(DynaKey){.key = placeholder.name});
        if (key != NULL)
            size += strlen(key->value);
    }

    uint64_t pos = 0;
    char *result = calloc(size, 1);

    for (int i = 0; i < element->placeholder_count; i++) {
        DynaPlaceholder placeholder = element->placeholders[i];

        strncat(result, element->original + pos, placeholder.pos - pos);

        DynaKey *key = hashmap_get(element->key_map, &(DynaKey) {.key = placeholder.name});
        if (key != NULL)
            strcat(result, key->value);

        pos = placeholder.pos;
    }

    strncat(result, element->original + pos, size - pos);

    return result;
}
