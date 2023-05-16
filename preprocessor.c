/* preprocessor.c
 * preprocessor functions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include "preprocessor.h"
#include "utils.h"
#include "errors.h"

macro_s *macros;  /* Global array of macros */
int num_macros = 0; /* Number of macros in the array */

#define HANDLE_REPORT if(report == ERR_MEM_ALLOC) return ERR_MEM_ALLOC; \
else if (report != NO_ERROR) error_flag = 1;

#define COUNT_SPACES(line_offset,line) while ((line)[line_offset] != '\0' && isspace((line)[line_offset])) \
(line_offset)++;

status assembler_preprocessor(file_context *src, file_context *dest) {
    char line[MAX_BUFFER_LENGTH];
    char *macro_name, *macro_body;
    unsigned int line_len;
    int found_macro = 0, error_flag = 0;
    status report;
    macro_name = macro_body = NULL;

    if (!src || !dest)
        return FAILURE; /* Unexpected error, probably unreachable */
    rewind(src->file_ptr); /* make sure we read from the beginning */

    while (fscanf(src->file_ptr, "%[^\n]%*c", line) == 1) {
        src->lc++;
        line_len = strlen(line);
        if (line_len > MAX_LINE_LENGTH) {
            error_flag = 1;
            handle_error(ERR_LINE_TOO_LONG, src);
        }
        report = handle_macro_start(src, line, &found_macro, &macro_name, &macro_body);
        HANDLE_REPORT;
        report = handle_macro_body(src, line, found_macro, &macro_body);
        HANDLE_REPORT;
        report = handle_macro_end(src, line, &found_macro, &macro_name, &macro_body);
        HANDLE_REPORT;
        report = write_to_file(src, dest, line, found_macro, report);
        HANDLE_REPORT;
    }
    /* Reset line counter and rewind files */
    src->lc = 0;
    dest->lc = 0;
    rewind(src->file_ptr);
    rewind(dest->file_ptr);
    free_unused_macros();
    return error_flag ? FAILURE : NO_ERROR;
}

status handle_macro_start(file_context *src, char *line, int *found_macro,
                          char **macro_name, char **macro_body) {
    char *ptr, *ptemp;
    size_t word_len;
    int i, line_offset = 0;
    status report = NO_ERROR;

    if (!*found_macro && (ptr = strstr(line, MACRO_START)) != NULL
        && strstr(line, MACRO_END) == NULL){
        *found_macro = 1;
        ptr += SKIP_MCRO;
        while (*ptr && (*ptr == ' ' || *ptr == '\t')) {
            ptr++;
        }
        word_len = get_word(&ptr);

        if (word_len >= MAX_MACRO_LENGTH) {
            handle_error(ERR_INVAL_MACRO_NAME, src);
            report = FAILURE;
        }

        for (i = 0; i < num_macros; i++) {
            if (strncmp(ptr, macros[i].name, strlen(macros[i].name)) == 0) {
                handle_error(ERR_DUP_MACRO, src);
                report =  FAILURE;
            }
        }

        ptemp = ptr;
        COUNT_SPACES(line_offset, ptemp);
        ptemp += line_offset + get_word(&ptemp);
        COUNT_SPACES(line_offset, ptemp);
        if (ptemp[line_offset] != '\0') {
            handle_error(ERR_EXTRA_TEXT, src); /* Extraneous text after macros name */
            report = FAILURE;
        }

        if (report == FAILURE) {
            *found_macro = 0;
            return FAILURE;
        }

        if (*macro_name != NULL) {
            free(*macro_name);
            *macro_name = NULL;
        }
        copy_n_string(macro_name, ptr, word_len);
        if (*macro_body != NULL) {
            handle_error(ERR_MISSING_ENDMACRO, src);
            *found_macro = 0;
            free(*macro_body);
            *macro_body = NULL;
        }

    }
    return report;
}


status handle_macro_body(file_context *src, char *line, int found_macro, char **macro_body) {
    static int macro_start = 0;
    int line_offset;
    char *new_macro_body = NULL;
    unsigned int body_len, line_length;

    if (!found_macro)
        return NO_ERROR;
    if (*macro_body != NULL) {
        macro_start = 0;
        /* Adding to the body of a macro */
        body_len = strlen(*macro_body);
        line_offset = 0;
        line_length = strlen(line);
        COUNT_SPACES(line_offset, line);

        if (strncmp(line + line_offset, "endmcro", SKIP_MCR0_END) == 0)
            return NO_ERROR;

        new_macro_body = realloc(*macro_body, body_len + line_length - line_offset + 2);
        if (new_macro_body == NULL) {
            handle_error(ERR_MEM_ALLOC, src);
            return ERR_MEM_ALLOC;
        }

        *macro_body = new_macro_body;
        strncat(*macro_body, line + line_offset, line_length - line_offset);
        strcat(*macro_body, "\n");
    } else {
        /* The beginning of a new macro's body */
        line_offset = 0;
        line_length = strlen(line);

        COUNT_SPACES(line_offset, line);
        if (macro_start == 0) {
            macro_start = 1;
            return NO_ERROR;
        }


        *macro_body = (char *)malloc(line_length - line_offset + 2);
        if (*macro_body == NULL) {
            handle_error(ERR_MEM_ALLOC, src);
            return ERR_MEM_ALLOC;
        }

        strncpy(*macro_body, line + line_offset, line_length - line_offset);
        (*macro_body)[line_length - line_offset] = '\0';
        strcat(*macro_body, "\n");
    }
    return NO_ERROR;
}


status handle_macro_end(file_context *src, char *line, int *found_macro,
                        char **macro_name, char **macro_body) {
    char *ptr;
    status add_macro_status;

    if (*found_macro && (ptr = strstr(line, MACRO_END)) != NULL) {
        *found_macro = 0;
        ptr += SKIP_MCR0_END;

        while (*ptr && isspace(*ptr)) {
            ptr++;
        }
            add_macro_status = add_macro(*macro_name, *macro_body);
            if (add_macro_status == ERR_MEM_ALLOC) {
                handle_error(ERR_MEM_ALLOC, src);
                return ERR_MEM_ALLOC;
            }
            free(*macro_name);
            free(*macro_body);
            *macro_name = NULL;
            *macro_body = NULL;
        }
    return NO_ERROR;
}

status write_to_file(file_context *src, file_context *dest, char *line, int found_macro, status report) {
    int line_offset, i;
    char *ptr, *word = NULL;
    size_t word_len;

    if (report != NO_ERROR)
        return FAILURE;
    if (found_macro) /* In the middle of processing a macro, no need to write the line */
        return NO_ERROR;

    line_offset = 0;
    while (isspace(line[line_offset])) {
        line_offset++;
    }
    ptr = line + line_offset;
    while (*ptr != '\0') {
        while (isspace(*ptr)) {
            fprintf(dest->file_ptr, "%c", *ptr);
            ptr++;
        }
        if (*ptr == '\0')
            break;
        word_len = get_word(&ptr);
        if (copy_n_string(&word, ptr, word_len) == ERR_MEM_ALLOC) {
            handle_error(ERR_MEM_ALLOC);
            free(word);
            return ERR_MEM_ALLOC;
        }
        found_macro = 0;
        for (i = 0; i < num_macros; i++) {
            if (strcmp(word, macros[i].name) == 0) {
                /* Replace the macro name with the macro body */
                found_macro = 1;
                fprintf(dest->file_ptr, "%s", macros[i].body);
                break;
            }
        }
        if (strcmp(word, MACRO_END) == 0) {
            ptr += SKIP_MCR0_END;
            line_offset = 0;
            COUNT_SPACES(line_offset, ptr);
            if (ptr[line_offset] != '\0') {
                handle_error(ERR_EXTRA_TEXT, src); /* Extraneous text after end of macros */
                return FAILURE;
            } else
                return NO_ERROR;
    }
        if (!found_macro)
            fprintf(dest->file_ptr, "%s", word);
        free(word);

        /* Move the pointer to the next word */
        ptr += word_len;
    }
    if (!found_macro)
        fprintf(dest->file_ptr, "\n");
    return NO_ERROR;
}




/**
* Adds a new macro with the given name and body to the global array of macros.
* If the array is full, it will be resized to double its current size.
*
* @param name The name of the macro to add.
* @param body The body of the macro to add.
*
* @return status, NO_ERROR in case of no error otherwise else the error status.
 */
status add_macro(char* name, char* body) {
    static int macro_cap = DEFAULT_MACROS; /* macro_s capacity */
    macro_s *temp_macro= NULL;
    status s_name , s_body;

    if (num_macros == 0)
        temp_macro = realloc(macros, DEFAULT_MACROS * sizeof(macro_s));
    else if (num_macros >= macro_cap) {
        temp_macro = realloc(macros, (macro_cap * 2) * sizeof(macro_s));
        macro_cap *= 2;
    }
    else temp_macro = macros; /* make sure we won't raise an unwanted error */
    if (!temp_macro)
        return ERR_MEM_ALLOC;
    macros = temp_macro;
    s_name = copy_string(&macros[num_macros].name, name);
    s_body = copy_string(&macros[num_macros].body, body);
    if (s_name != NO_ERROR || s_body != NO_ERROR)
        return ERR_MEM_ALLOC;
    num_macros++;
    return NO_ERROR;
}

/**
 * Frees the memory allocated for the global array of macros,
 * and resets the counter to 0.
 */
void free_macros() {
    int i;
    if (!macros) return;
    for (i = 0; i < num_macros; i++) {
        free(macros[i].name);
        free(macros[i].body);
    }
    free(macros);
    macros = NULL;
    num_macros = 0;
}

/**
 * Frees any unused memory allocated for the macros array by resizing it to the exact size
 * of the used memory. If the array is empty, returns immediately without performing any action.
 *
 * @throws status_type ERR_MEM_ALLOC when there's a memory allocation error during reallocation.
 */
void free_unused_macros() {
    macro_s *temp_macro = NULL;
    if (num_macros == 0)
        return;
    temp_macro = realloc(macros, num_macros * sizeof(macro_s));
    if (!temp_macro)
        handle_error(ERR_MEM_ALLOC);
    macros = temp_macro;
}
