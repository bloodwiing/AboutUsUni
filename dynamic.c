/// DYNAMIC HTML GENERATOR
/// BLOODWIING © 2022
/// GITHUB: https://github.com/bloodwiing

#include "dynamic.h"
#include <string.h>
#include <stdlib.h>


// ----- TECHNICAL STRUCTS -----
// These structs are not meant to be modified by hand, they are autofilled and used by the inner workings of the library

/// User-entered value pair for Placeholder replacement
typedef struct DynaKey {
    char *key; // The name of the placeholder to replace
    char *value; // The value to replace with
} DynaKey;


// ----- HASHMAP FUNCTIONS -----
// They allow for comparison and correct hashing of the DynaKey struct that is stored in buckets

/// Compares two pointers (DynaKey *) if their key matches
/// \param a Object pointer A
/// \param b Object pointer B
/// \return 1 if the keys of the objects match, 0 if not
int compareDynaKey(const void *a, const void *b, void *udata) {
    const DynaKey *da = a;
    const DynaKey *db = b;
    return strcmp(da->key, db->key);
}

/// Generates the hash of the object (DynaKey) pointed to
/// \param item Object pointer
/// \return The generated 64 bit Hash
uint64_t hashDynaKey(const void *item, uint64_t seed0, uint64_t seed1) {
    const DynaKey *dyna_key = item;
    return hashmap_sip(dyna_key->key, strlen(dyna_key->key), seed0, seed1);
}


// ----- PRIVATE FUNCTIONS -----
// These functions handle the inner workings of the library
// Not for manual use

/// Inserts a new Placeholder into the dynamic array stored in the element
/// \param element The element where to store the Placeholder
/// \param placeholder The character where the Placeholder name starts
/// \param placeholder_size The length of the Placeholder name
/// \param position The relative position where the Placeholder in reference to the de-formatted element string
void addDynamicElementPlaceholder(DynaElement *element, char *placeholder, uint64_t placeholder_size, uint64_t position) {
    // Dynamic array
    element->placeholders = realloc(element->placeholders, sizeof(DynaPlaceholder) * ++element->placeholder_count);

    // Get item and set its name
    DynaPlaceholder *item = &element->placeholders[element->placeholder_count - 1];
    item->name = calloc(placeholder_size + 1, 1);
    strncpy(item->name, placeholder, placeholder_size);

    // Update its position too
    item->pos = position;
}

/// Analyses the element's string and processes it.<br>
/// It cuts out every single Placeholder and adds them, escapes any double brackets and saves the new string without
/// escapes and placeholders into the element
/// \param element The element to process
void analyseDynamicElement(DynaElement *element) {
    // The pointer to the first character of the element string
    char *cursor = element->original;
    // The de-formatted string builder
    char *final = calloc(strlen(element->original) + 1, 1);

    // Pointer to the char where the Placeholder name starts
    char *placeholder_start = NULL;
    // Pointer to the char where the Placeholder name has ended
    // By default it's the very first character as it's used to simply paste text into the final string unchanged up
    // from this point
    char *placeholder_end = element->original;
    // The size of the Placeholder name
    uint64_t placeholder_size = 0;

    // The current position of the string, used for calculating where the Placeholder belongs in reference to the
    // edited string
    uint64_t position = 0;
    // The position of the last placeholder, used for calculating how many characters have passed since the last
    // placeholder for concatenation
    uint64_t last_placeholder = 0;
    // Since escapes and Placeholders themselves appear differently in the final string, this helps remember how much
    // to actually offset the position
    uint64_t placeholder_offset = 0;

    // Loop while the string has not ended yet (aka while the cursor doesn't point to a '\0')
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
    free(element->original);
    element->original = final;
}


// ---- FUNCTION IMPLEMENTATIONS -----
// This is where the functions are implemented
// Function comments are missing here as they are described in the header

size_t getFormattedStringSize(char *format, va_list arg_list) {
    // The return value of `vsnprintf()` gives the total string length that was generated after formatting
    return vsnprintf(NULL, 0, format, arg_list) + 1;
}

DynaElement *createDynamicDocument(FILE *file) {
    // Check for if file does not exist
    if (file == NULL)
        return NULL;

    // Allocate the element
    DynaElement *out = calloc(1, sizeof(DynaElement));

    // Calculate the total size of the file
    fseek(file, 0L, SEEK_END);
    long size = ftell(file);
    fseek(file, 0L, SEEK_SET);

    // Allocate the string by how big the file is and put all file contents to it
    out->original = calloc(size + 1, 1);
    fread(out->original, 1, size, file);

    // Begin analysing for Placeholders and generate a de-formatted string
    analyseDynamicElement(out);

    // Create a new HashMap that supports hashing and comparisons for DynaKey elements
    out->key_map = hashmap_new(sizeof(DynaKey), 0, 0, 0, hashDynaKey, compareDynaKey, NULL, NULL);

    return out;
}

void putDynamicData(DynaElement *element, char *key, char *format, ...) {
    // Init the key on this scope (its value gets copied, so this may as well be discarded after the function)
    DynaKey out = {};

    // Allocate the name of the key based on how long the `key` param is and set it
    out.key = malloc(strlen(key) + 1);
    strcpy(out.key, key);

    // Using a Variable Argument list calculate the total size of the final string
    va_list va;
    va_start(va, format);
    size_t count = getFormattedStringSize(format, va);
    va_end(va);

    // Use that size as the size of the allocated string
    out.value = calloc(count, 1);
    // Reset the Variable Argument List and save the final string
    va_start(va, format);
    vsnprintf(out.value, count, format, va);
    va_end(va);

    // Put the item into the hashmap for generation
    hashmap_set(element->key_map, &out);
}

char *generateDynamicString(DynaElement *element) {
    // NULL Check
    if (element == NULL)
        return NULL;

    // If a result was previously generated, free the previous result then (memory leak prevention)
    if (element->result != NULL) {
        free(element->result);
        element->result = NULL;
    }

    // Calculate the final string size of the element when generated
    // Basic logic: total size = de-formatted string size + each placeholder's value string size
    uint64_t size = strlen(element->original) + 1;

    // Loop over each Placeholder
    for (int i = 0; i < element->placeholder_count; i++) {
        DynaPlaceholder placeholder = element->placeholders[i];
        // Get the Placeholder's replacement value (which was set by the user via `putDynamicData()`
        DynaKey *key = hashmap_get(element->key_map, &(DynaKey){.key = placeholder.name});
        // And if it was set, add it to the final size
        if (key != NULL)
            size += strlen(key->value);
    }

    // Tracking the last position of the placeholder
    // Used to add the de-formatted string before and after every Placeholder position
    // By default it's 0 in case there are no Placeholders, then right below the loop the whole de-formatted string
    // just gets added to the final string
    uint64_t pos = 0;
    // Create the string of the final size
    element->result = calloc(size, 1);

    // Loop over every Placeholder
    for (int i = 0; i < element->placeholder_count; i++) {
        DynaPlaceholder placeholder = element->placeholders[i];

        // Add the de-formatted string UP TO the Placeholder's position
        strncat(element->result, element->original + pos, placeholder.pos - pos);

        // Get the Placeholder's replacement value (which was set by the user via `putDynamicData()`
        DynaKey *key = hashmap_get(element->key_map, &(DynaKey) {.key = placeholder.name});
        // And if it was set, add it to the final string
        if (key != NULL)
            strcat(element->result, key->value);

        // Continue keeping track of where the last Placeholder ended
        pos = placeholder.pos;
    }

    // After all placeholders ended, add the remaining de-formatted string since the last Placeholder
    // Remember to save it to the element for de-allocation when the Element gets discarded
    strncat(element->result, element->original + pos, size - pos);

    // Return the string
    return element->result;
}

void freeDynamicElement(DynaElement *element) {
    // Free the de-formatted string
    free(element->original);

    // HashMap.c pointer-based loop
    size_t iter = 0;
    void *item;
    while (hashmap_iter(element->key_map, &iter, &item)) {
        // Get each key
        const DynaKey *key = item;
        // Free its name
        free(key->key);
        // Free its replacement value
        free(key->value);
    }
    // And use the provided function to de-allocate the whole HashMap now
    hashmap_free(element->key_map);

    // For every Placeholder
    for (int i = 0; i < element->placeholder_count; i++)
        // Free up the Placeholder's name
        free(element->placeholders[i].name);
    // Then de-allocate the dynamic array
    free(element->placeholders);

    // Finally de-allocate the generated final string
    free(element->result);
    // And the element itself
    free(element);
}
