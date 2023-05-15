/* utils.c
 * declaration for utilities functions, handling errors and managing input
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "errors.h"
#include "utils.h"


file_context* create_file_context(const char* file_name, char* ext, char* mode, status *report) {
    file_context* fc;
    FILE* file = NULL;
    char* file_name_w_ext;
    size_t len;

    fc = malloc(sizeof(file_context));
    if (!fc) {
        *report = ERR_MEM_ALLOC;
        return fc;
    }
    len = strlen(file_name) + FILENAME_EXT_LEN + 1;
    file_name_w_ext = malloc(len * sizeof(char));
    if (!file_name_w_ext) {
        *report = ERR_MEM_ALLOC;
        free(fc);
        return NULL;
    }
    strcpy(file_name_w_ext, file_name);
    strcat(file_name_w_ext, ext);
    copy_n_string(&fc->file_name, file_name_w_ext, len);
    free(file_name_w_ext);
    fc->file_ptr = NULL;
    file = fopen(fc->file_name, mode);

    if (!file) {
        handle_error(ERR_OPEN_FILE, fc);
        *report = ERR_OPEN_FILE;
        return NULL;
    }

    fc->file_ptr = file;
    fc->ic = 0;
    fc->dc = 0;
    fc->lc = 0;
    *report = NO_ERROR;
    return fc;
}

/**
 * Finds the length of the consecutive characters in a word, skipping leading white spaces.
 * Updates the pointer to point to the start of the word.
 *
 * @param ptr Pointer to the input string. Updated to point to the start of the word.
 * @return Length of the word.
 */
size_t get_word(char **ptr) {
    char *start;
    size_t length = 0;

    while (**ptr && isspace((int)**ptr)) {
        (*ptr)++;
    }
    start = *ptr;
    while (**ptr && !isspace((int)**ptr)) {
        (*ptr)++;
        length++;
    }

    *ptr = start;
    return length;
}

/**
 * Copy a string from source to target and allocate memory for the target
 * according to the source.
 *
 * @param target  Pointer to the target string.
 * @param source  Pointer to the source string.
 * @return        Status: NO_ERROR if successful, otherwise the error status.
 */
status copy_string(char** target, const char* source) {
    char* temp = malloc(strlen(source) + 1);
    if (!temp) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }
    strcpy(temp, source);
    *target = temp;
    return NO_ERROR;
}

/**
 * Copy the first count characters from a source string to target string,
 * and allocate memory to target according to source.
 *
 * @param target  Target string.
 * @param source  Source string.
 * @param count number of characters to copy
 *
 * @return status, NO_ERROR in case of no error otherwise else the error status.
 */
status copy_n_string(char** target, const char* source, size_t count) {
    char* temp = malloc(count + 1);
    if (!temp) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }
    strncpy(temp, source, count);
    temp[count] = '\0';
    *target = temp;
    return NO_ERROR;
}

/**
 * Frees the memory occupied by a file_context structure.
 * Closes the file pointer if it's open and frees the dynamically allocated file name.
 *
 * @param context The file_context structure to be freed.
 */
void free_file_context(file_context* context) {
    if (context != NULL) {
        if (context->file_ptr != NULL)
            fclose(context->file_ptr);

        if (context->file_name != NULL)
            free(context->file_name);

        free(context);
    }
}
