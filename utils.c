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
 * @param ext_len The length of the extension.
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
    fc->tc = 0;
    fc->tc = 0;

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

    if (!**ptr)
        return 0;

    while (**ptr && isspace((int)**ptr))
        (*ptr)++;

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
    char target_delimiter;

    if (!word) return 0;

    while (**ptr && isspace((int)**ptr))
        (*ptr)++;

    if (delimiter == COMMA)
        target_delimiter = ',';
    else if (delimiter == COLON)
        target_delimiter = ':';
    else if (delimiter == QUOTE)
        target_delimiter = '\"';
    else
        target_delimiter = ' ';

    while (**ptr && **ptr != target_delimiter && (!isspace((int)**ptr) || delimiter == QUOTE)){
        word[length] = **ptr;
        (*ptr)++;
        length++;
    }

    if (delimiter != SPACE && **ptr == target_delimiter) {
        word[length] = **ptr;
        (*ptr)++;
        length++;
    }

    word[length] = '\0';

    return length;
}

/**
 * Safely converts a string to an integer.
 * Does the same as atoi() but safer.
 *
 * @param str The string to convert to an integer.
 * @return The converted integer value, or 0 if the conversion fails.
 */
int safe_atoi(const char *str) {
    int result = 0;
    int sign = 1;

    if (!str)
        return 0;

    while (isspace(*str))
        str++;

    /* Check for sign */
    if (*str == '-') {
        sign = -1;
        str++;
    } else if (*str == '+')
        str++;

    while (isdigit(*str)) {
        result = result * 10 + (*str - '0');
        str++;
    }

    while (*str) {
        if (!isspace(*str))
            return 0;
        str++;
    }

    return result * sign;
}

/**
 * Validates if the given string is a valid register.
 *
 * @param src Pointer to file context for error handling.
 * @param str The string to validate as a register.
 * @param report Pointer to status report for error indication.
 * @return The register number if valid, or 0 if invalid.
 */
int is_valid_register(file_context *src, const char* str, status *report) {
    int missing_at_key = 0;
    if (*str != '@')
        missing_at_key = 1;

    if (str[1] == 'r' && str[2] >= '0' && str[2] <= '7' && (str[3] == '\0' || isspace(str[3]))) {
        if (missing_at_key) {
            handle_error(ERR_MISS_ADDRESS_MARK, src);
            *report = ERR_MISS_ADDRESS_MARK;
        }
        return 1;
    }
    else
        return 0;
}

/**
 * Checks if the current line contains a valid string and extracts it as a word.
 *
 * @param line The current line being processed. It will be updated to skip leading whitespace and the extracted word.
 * @param word Pointer to store the extracted word. Memory for the word should be allocated by the caller.
 * @param report Pointer to the status report to indicate any errors.
 * @return The length of the extracted word if a valid string was found and extracted, 0 otherwise.
 */
size_t is_valid_string(char **line, char **word, status *report) {
    size_t length;

    if (**line == '\0' || **line == '\n')
        return 0;

    while (**line && isspace(**line))
        (*line)++;

    if (*word) free(*word);
    *word = malloc(sizeof(char) * (length = get_word_length(line)) + 1);

    if (!*word || !get_word(line, *word, COMMA)){
        *report = ERR_MEM_ALLOC;
        handle_error(ERR_MEM_ALLOC);
    }
    return *report == ERR_MEM_ALLOC ? 0 : length;
}

/**
 * Checks if a string is a valid label.
 *
 * @param label The string to check.
 * @return NO_ERROR if the string is a valid label, an appropriate error status otherwise.
 *
 * @remarks The function checks if the label meets the following criteria:
 *   - The label is not NULL and has a length between 1 and MAX_LABEL_LENGTH characters.
 *   - The label does not match any reserved command or sym_dir.
 *   - The label does not start with a digit.
 *   - The label consists only of alphanumeric characters.
 *   - The label ends with a colon (':') to indicate a label declaration.
 */
status is_valid_label(const char *label) {
    size_t length = strlen(label);
    int i;

    if (!label || length == 0  || length > MAX_LABEL_LENGTH ||
        is_command(label) != INV_CMD || is_directive(label + 1) ||
        is_directive(label))
        return ERR_INVALID_LABEL;

    if (isdigit(*label))
        return ERR_LABEL_START_DIGIT;

    if (!isalpha(*label))
        return ERR_ILLEGAL_CHARS;

    for (i = 1; i < length - 1; i++)
        if (!isalnum(label[i]) || isspace(label[i]))
            return ERR_ILLEGAL_CHARS;

    if (label[length - 1] != ':' && !isalnum(label[length - 1]))
        return ERR_ILLEGAL_CHARS;

    if (label[length - 1] == ':')
        return NO_ERROR;

    return ERR_MISSING_COLON;
}

/**
 * Validates if the given string is a valid data value.
 *
 * @param src Pointer to the file context.
 * @param word The string to validate as a data value.
 * @param length The length of the word.
 * @param report Pointer to the status report.
 * @return The type of the data value (LBL, NUM, STR, or INV) if valid, or INV if invalid.
 */
Value validate_data(file_context *src, char *word, size_t length, status *report) {
    char *p_word = NULL;
    status temp_report;

    if (*word == '+' || *word == '-')
        word++;

    if (isalpha(*word)) {
        if (word[length - 1] == ':') {
            word[length - 1] = '\0';
            handle_error(ERR_FORBIDDEN_LABEL_DECLARE, src, word);
            *report =  ERR_FORBIDDEN_LABEL_DECLARE;
        }
        temp_report = is_valid_label(word);
        if (temp_report != NO_ERROR && temp_report != ERR_MISSING_COLON) {
            *report =  ERR_INVALID_SYNTAX;
            return INV;
        }
        return LBL;
    }
    else if (isdigit(*word)) {
        p_word = word;
        while (!isspace(*p_word) && *p_word != '\0' && *p_word != '\n') {
            if (!isdigit(*p_word)) {
                handle_error(ERR_INVALID_SYNTAX, src, "label", p_word);
                *report = ERR_INVALID_SYNTAX;
                return INV;
            }
            p_word++;
        }
        return NUM;
    }
    return INV;
}

/**
 * Extracts the next word from the line, accounting for spaces within a string.
 *
 * Extracts the next word from the line, considering spaces within a string.
 * It allocates memory for the word and returns it. Updates the word length and report status accordingly.
 *
 * @param line The current line being processed. Will be updated to skip leading whitespace and the extracted word.
 * @param word_len Pointer to store the length of the extracted word.
 * @param report Pointer to the status report to indicate any errors.
 * @return The extracted word as a dynamically allocated string, or NULL if an error occurred.
 */
char* has_spaces_string(char **line, size_t *word_len, status *report) {
    char *next_word = NULL;
    next_word = malloc(sizeof(char) * get_word_length(line) + 1);

    if (!next_word) {
        handle_error(ERR_MEM_ALLOC);
        *report = ERR_MEM_ALLOC;
        return NULL;
    } else if (!(*word_len = get_word(line, next_word, COMMA))) {
        free (next_word);
        return NULL;
    }
    return next_word;
}

/**
 * Concatenates and validates a string by appending space characters between words.
 *
 * This function is responsible for concatenating a string by adding space characters
 * between words. It scans the input line and extracts each word, ensuring proper spacing
 * between them. It returns the concatenated string and validates its format.
 *
 * @param line - A pointer to the input line string.
 * @param word - A pointer to the string being concatenated and validated.
 * @param length - A pointer to the length of the string.
 * @param report - A pointer to the status report variable.
 * @return The value indicating the type of the concatenated and validated string (STR or INV).
 */
Value concat_and_validate_string(char **line, char **word, size_t *length ,status *report) {
    char *next_word = NULL;
    char white_spaces_str[MAX_LABEL_LENGTH];
    char *p_word = *word;
    size_t word_len = 0, white_spaces_amt = 0;
    status temp_report = NO_ERROR;

    while (p_word[*length - 1] != '\"') {
        **word = *p_word;

        while(**line && isspace(**line)) {
            white_spaces_str[white_spaces_amt++] = **line;
            (*line)++;
        }
        white_spaces_str[white_spaces_amt] = '\0';
        next_word = has_spaces_string(line, &word_len, &temp_report);
        if (!next_word && temp_report == NO_ERROR) break;
        else if (!next_word || ((p_word = realloc(p_word, (*length += word_len + white_spaces_amt) + 1)) &&
                (!strcat(p_word, white_spaces_str) || !strcat(p_word, next_word)))) {
            *report = ERR_MEM_ALLOC;
            if (next_word) free(next_word);
            return INV;
        }
        free(next_word);
    }
    *word = p_word;
    return STR;
}

/**
 * Validates if the given string is a valid string value.
 *
 * @param line Pointer to address of the line.
 * @param word Pointer to address of string to validate as a string value.
 * @param length Pointer to the length of the word.
 * @param report Pointer to the status report.
 * @return The type of the string value (LBL, STR, or INV) if valid, or INV if invalid.
 */
Value validate_string(char **line ,char **p_word, size_t length, status *report) {
    char *word = NULL;
    status temp_report;
    size_t word_len = length;
    word = *p_word;

    if (*word == '\"') {
        if (concat_and_validate_string(line, p_word, &word_len, report) == INV) return INV;
        word = *p_word;
        if (word[word_len - 1] != '\"')
            *report = ERR_MISSING_QMARK;
        return isalpha(word[1]) ? STR : INV;
    }
    else {
        temp_report = is_valid_label(word);
        if (temp_report != NO_ERROR && temp_report != ERR_MISSING_COLON) {
            *report = ERR_INVALID_SYNTAX;
            return INV;
        }
        return LBL;
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

    if (!*target) *target = NULL;

    if (!*target) free(*target);

    temp = malloc(strlen(source) + 1);
    if (!temp) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }

    strcpy(temp, source);
    temp[strlen(source)] = '\0';

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
 * Checks if a given string is a valid Directive.
 *
 * @param src The string to check.
 * @return The corresponding Directive index if the string is a valid Directive, otherwise DEFAULT.
 */
Directive is_directive(const char* src) {
    int i;
    if (src)
        for (i = 0; i < DIRECTIVE_LEN; i++)
            if (strncmp(src, directives[i], strlen(directives[i])) == 0)
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
    return INV_CMD;
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