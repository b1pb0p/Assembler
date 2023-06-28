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

const char *directives[DIRECTIVE_LEN] = {
    "data",
    "string",
    "entry",
    "extern"
};

const char *commands[COMMANDS_LEN] = {
    "mov",
    "cmp",
    "add",
    "sub",
    "not",
    "clr",
    "lea",
    "inc",
    "dec",
    "jmp",
    "bne",
    "red",
    "prn",
    "jsr",
    "rts",
    "stop"
};

/**
 * Creates a file context object, add extension to file name,
 * and opens the file in the specified mode.
 *
 * @param file_name The name of the file.
 * @param ext The extension to append to the file name.
 * @param mode The file mode for opening the file (e.g., "r" for read, "w" for write).
 * @param report Pointer to the status report variable.
 * @return A pointer to the created file context object if successful, NULL otherwise.
 */
file_context* create_file_context(const char* file_name, char* ext, size_t ext_len, char* mode, status *report) {
    file_context* fc;
    FILE* file = NULL;
    char *file_name_w_ext = NULL;
    size_t len;

    fc = malloc(sizeof(file_context));
    if (!fc) {
        *report = ERR_MEM_ALLOC;
        return fc;
    }

    len = strlen(file_name) + ext_len + 1;
    file_name_w_ext = malloc(len * sizeof(char));

    if (!file_name_w_ext) {
        *report = ERR_MEM_ALLOC;
        free_file_context(&fc);
        return NULL;
    }


    strcpy(file_name_w_ext, file_name);
    strcat(file_name_w_ext, ext);
    fc->file_name_wout_ext = strdup( file_name);

    if (!(fc->file_name_wout_ext)) {
        *report = ERR_MEM_ALLOC;
        free_file_context(&fc);
        return NULL;
    }

    fc->file_name = NULL;
    fc->file_ptr = NULL;

    copy_n_string(&fc->file_name, file_name_w_ext, len);
    free(file_name_w_ext);
    file = fopen(fc->file_name, mode);

    if (!file) {
        handle_error(ERR_OPEN_FILE, fc);
        *report = ERR_OPEN_FILE;
        free_file_context(&fc);
        return NULL;
    }

    fc->file_ptr = file;
    fc->lc = 1; /* line starts from 1 */
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
size_t get_word_length(char **ptr) {
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
 * Extracts the next word from the input string pointed to by 'ptr'.
 * The word is delimited by whitespace or a specific delimiter character.
 * It will update 'ptr' to point to the next character after the extracted word.
 *
 * @param ptr The pointer to the input string.
 * @param word The buffer to store the extracted word.
 * @param delimiter The delimiter character to use for word extraction (COMMA, COLON, or SPACE for whitespace delimiter).
 * @return The length of the extracted word.
 */
size_t get_word(char **ptr, char *word, Delimiter delimiter) {
    size_t length = 0;

    if (!word) return 0;

    while (**ptr && isspace((int)**ptr))
        (*ptr)++;

    char targetDelimiter;
    if (delimiter == COMMA)
        targetDelimiter = ',';
    else if (delimiter == COLON)
        targetDelimiter = ':';
    else
        targetDelimiter = ' ';

    while (**ptr && **ptr != targetDelimiter && !isspace((int)**ptr)) {
        word[length] = **ptr;
        (*ptr)++;
        length++;
    }

    if (delimiter != SPACE && **ptr == targetDelimiter) {
        word[length] = **ptr;
        (*ptr)++;
        length++;
    }

    word[length] = '\0';

    return length;
}


/**
 * Pushes the pointer back to the beginning of the previous word, accounting for unknown spaces or tabs.
 * If the previous word contains spaces or tabs, it goes back to the first character of the word.
 *
 * @param ptr The pointer to the input string. It will be updated to point to the previous word.
 * @param word_length The length of the previous word.
 * @param line The input line as a string.
 */
void unget_word(char **ptr, size_t word_length, char *line) {
    if (word_length > 0) {
        *ptr -= word_length;
        while (word_length > 0 && (isspace((int)(*ptr)[-1]) || (*ptr)[-1] == '\t')) {
            (*ptr)--;
            word_length--;
        }
        while (word_length > 0 && (*ptr)[-1] != '\0' && (*ptr)[-1] != '\n') {
            (*ptr)--;
            word_length--;
        }
    }
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
    char* temp = NULL;
    if (!source) {
        handle_error(TERMINATE, "copy_string()");
        return TERMINATE;
    }

    temp = malloc(strlen(source) + 1);
    if (!temp) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }

    strcpy(temp, source);
    temp[strlen(source)] = '\0';

    if (!*target) *target = NULL;

    if (!*target) free(*target);

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
    char* temp = NULL;
    if (!source) {
        handle_error(TERMINATE, "copy_n_string()");
        return TERMINATE;
    }

    temp = malloc(count + 1);
    if (!temp) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }
    strncpy(temp, source, count);
    temp[count] = '\0';

    if (!*target)*target = NULL;

    if (!*target) free(*target);

    *target = temp;
    return NO_ERROR;
}

/**
 * Skips leading white spaces (spaces, tabs, and newlines) in a line and updates the line pointer.
 *
 * @param line The line to skip white spaces from.
 * @return Status indicating the result of the operation: NO_ERROR if successful, or EOF if end of line reached.
 */
status skip_white_spaces(char *line) {
    int i;

    for (i = 0; line[i] == ' ' || line[i] == '\t' || line[i] == '\n'; i++)
        ;
    if (line[i] == '\0' || line[i] == '\n')
        return EOF;

    strcpy(line, line + i);
    return NO_ERROR;
}

/**
 * Checks if a given string is a valid Directive.
 *
 * @param src The string to check.
 * @return The corresponding Directive index if the string is a valid Directive, otherwise DEFAULT.
 */
Directive is_directive(const char* src) {
    int i;
    if (src)
        for (i = 0; i < DIRECTIVE_LEN; i++)
            if (strcmp(src, directives[i]) == 0)
                return i + 1; /* Corresponding Directive*/
    return 0;
}

/**
 * Checks if a given string is a valid Command.
 *
 * @param src The string to check.
 * @return The corresponding Command index if the string is a valid Command, otherwise 0.
 */
Command is_command(const char* src) {
    int i;
    if (src)
        for (i = 0; i < COMMANDS_LEN; i++)
            if (strcmp(src, commands[i]) == 0)
                return i; /* Corresponding Command*/
    return 0;
}

/**
 * Duplicates a string by allocating memory and copying the contents of the original string.
 *
 * @param s The original string to be duplicated.
 * @return A pointer to the newly allocated duplicated string, or NULL if memory allocation fails.
 */
char *strdup(const char *s) {
    size_t len = strlen(s) + 1;
    char *dup = malloc(len);
    if (dup) {
        memcpy(dup, s, len);
    }
    return dup;
}

/**
 * Frees the memory occupied by a file_context structure.
 * Closes the file pointer if it's open and frees the dynamically allocated file name.
 *
 * @param context The file_context structure to be freed.
 */
void free_file_context(file_context** context) {
    if (*context != NULL) {
        if ((*context)->file_ptr != NULL)
            fclose((*context)->file_ptr);

        if ((*context)->file_name != NULL)
            free((*context)->file_name);

        if ((*context)->file_name_wout_ext != NULL)
            free((*context)->file_name_wout_ext);

        free(*context);
        *context = NULL;
    }
}

/**
 * Frees the memory occupied by an array of file_context structures.
 * Calls the free_file_context function to free each individual file_context structure in the array.
 *
 * @param contexts The array of file_context pointers to be freed.
 * @param size The size of the array.
 */
void free_file_context_array(file_context** contexts, int size) {
    int i;

    if (!contexts)
        return;

    for (i = 0; i < size; i++)
        if(contexts[i]) free_file_context(&contexts[i]);
}
