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

macro_node* macro_head = NULL; /* Head of the macros linked list */
macro_node* macro_tail = NULL; /* Tail of the macros linked list */

#define HANDLE_REPORT if(report == ERR_MEM_ALLOC || report == TERMINATE) return TERMINATE; \
else if (report != NO_ERROR) found_error = 1;

#define COUNT_SPACES(line_offset,line) while ((line)[line_offset] != '\0' && isspace((line)[line_offset])) \
(line_offset)++;

#define IS_EMPTY() (macro_head == NULL)

status assembler_preprocessor(file_context *src, file_context *dest) {
    char line[MAX_BUFFER_LENGTH];
    char *macro_name, *macro_body;
    unsigned int line_len;
    int found_macro = 0, found_error = 0, ch;
    status report;
    macro_name = macro_body = NULL;

    if (!src || !dest)
        return FAILURE; /* Unexpected error, probably unreachable */
    rewind(src->file_ptr); /* make sure we read from the beginning */

    while (fscanf(src->file_ptr, "%[^\n]%*c", line) == 1
            || (ch = fgetc(src->file_ptr)) == '\n') {
        src->lc++;

        if (*line == ';')
            continue;
        if (ch == '\n') {
            fprintf(dest->file_ptr, "\n");
            ch = -1;
            continue;
        }

        line_len = strlen(line);
        if (line_len > MAX_LINE_LENGTH) {
            found_error = 1;
            handle_error(ERR_LINE_TOO_LONG, src);
        }
        report = handle_macro_start(src, line, &found_macro, &macro_name, &macro_body);
        HANDLE_REPORT;
        report = handle_macro_body(src, line, found_macro, &macro_body);
        HANDLE_REPORT;
        report = handle_macro_end(line, &found_macro, &macro_name, &macro_body);
        HANDLE_REPORT;
        report = write_to_file(src, dest, line, found_macro, found_error);
        HANDLE_REPORT;
    }
    /* Reset line counter and rewind files */
    dest->lc = 0;
    rewind(dest->file_ptr);

    if (found_error) { /* Error found, output file should be removed */
        fclose(dest->file_ptr);
        remove(dest->file_name);
    }

    free_macros();

    return found_error ? FAILURE : NO_ERROR;
}

status handle_macro_start(file_context *src, char *line, int *found_macro,
                           char **macro_name, char **macro_body) {
    char *mcro, *endmcro, ch_m;
    size_t word_len;
    int line_offset = 0;
    status report = NO_ERROR;

    mcro = strstr(line, MACRO_START);
    endmcro = strstr(line, MACRO_END);

    if (*found_macro) { /* previously found 'mcro' */
        if ((mcro && !endmcro) || (endmcro && mcro < endmcro)) { /* 'mcro' detected */
            handle_error(ERR_MISSING_ENDMACRO, src);
            if (**macro_body) free(*macro_body);
            *macro_body = NULL;
        }
    }
    else {
        if ((mcro && !endmcro) || (endmcro && (mcro < endmcro))) { /* 'mcro' detected */
            *found_macro = 1;
            mcro += SKIP_MCRO;
            if (!isspace(*mcro)) {
                *found_macro = 0;
                return NO_ERROR;
            }
            while (*mcro && (*mcro == ' ' || *mcro == '\t')) {
                mcro++;
            }
            word_len = get_word(&mcro);

            if (word_len >= MAX_MACRO_NAME_LENGTH) {
                handle_error(ERR_INVAL_MACRO_NAME, src);
                report = FAILURE;
            }

            if (is_macro_exists(mcro)) {
                    handle_error(ERR_DUP_MACRO, src);
                    report = FAILURE;
            }

            endmcro = mcro;
            COUNT_SPACES(line_offset, endmcro);
            endmcro += line_offset + get_word(&endmcro);
            COUNT_SPACES(line_offset, endmcro);
            if (endmcro[line_offset] != '\0') {
                handle_error(ERR_EXTRA_TEXT, src); /* Extraneous text after macros name */
                report = FAILURE;
            }


            if (*macro_name) {
                free(*macro_name);
                *macro_name = NULL;
            }
            if(copy_n_string(macro_name, mcro, word_len) != NO_ERROR) return FAILURE;
        }
        else if (endmcro && (isspace(*(endmcro + 1)) )) {
            handle_error(ERR_MISSING_MACRO, src);
            return FAILURE; /* 'endmcro' detected, without a matching opening 'mcro.' */
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


status handle_macro_end(char *line, int *found_macro,
                        char **macro_name, char **macro_body) {
    char *ptr;
    status report = NO_ERROR;

    if (*found_macro && (ptr = strstr(line, MACRO_END)) != NULL) {
        *found_macro = 0;
        ptr += SKIP_MCR0_END;

        while (*ptr && isspace(*ptr)) {
            ptr++;
        }

        report = add_macro(*macro_name, *macro_body);

        if (*macro_name) free(*macro_name);
        if (*macro_body) free(*macro_body);
        *macro_name = NULL;
        *macro_body = NULL;
        }
    return report;
}

status write_to_file(file_context *src, file_context *dest, char *line, int found_macro, int found_error) {
    int line_offset;
    char *ptr, *word = NULL;
    macro_node *matched_macro;
    size_t word_len;

    if (found_error)
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
        if (copy_n_string(&word, ptr, word_len) != NO_ERROR) {
            free(word);
            return TERMINATE;
        }
        found_macro = 0;

        if ((matched_macro = is_macro_exists(word))) {
                /* Replace the macro name with the macro body */
                found_macro = 1;
                fprintf(dest->file_ptr, "%s", matched_macro->body);
        }
        if (strncmp(word, MACRO_END,SKIP_MCRO) == 0) {
            ptr += SKIP_MCR0_END;
            line_offset = 0;
            COUNT_SPACES(line_offset, ptr);
            if (ptr[line_offset] != '\0') {
                handle_error(ERR_EXTRA_TEXT, src); /* Extraneous text after end of macros */
                return FAILURE;
            }
            else
                return NO_ERROR;
    }
        else if (strcmp(word, MACRO_START) == 0) {
            handle_error(ERR_EXTRA_TEXT, src); /* Extraneous text after macro call */
            return FAILURE;
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
* Adds a new macro with the given name and body to the global linked list of macros.
*
* @param name The name of the macro to add.
* @param body The body of the macro to add.
*
* @return status, NO_ERROR in case of no error otherwise else the error status.
 */
status add_macro(char* name, char* body) {
    status s_name , s_body;
    macro_node* new_macro = (macro_node*)malloc(sizeof(macro_node));

    if (!new_macro) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }

    s_name = copy_string(&new_macro->name, name);
    s_body = copy_string(&new_macro->body, body);

    if (s_name != NO_ERROR || s_body != NO_ERROR) {
       if (new_macro->name) free(new_macro->name);
       if (new_macro->body) free(new_macro->body);
        free(new_macro);
        return TERMINATE;
    }

    new_macro->next = NULL;

    if (IS_EMPTY()) {
        macro_head = new_macro;
        macro_tail = new_macro;
    } else {
        macro_tail->next = new_macro;
        macro_tail = new_macro;
    }
    return NO_ERROR;
}

/**
 * Checks if a macro with the given name exists.
 *
 * @param name The name of the macro to check.
 *
 * @return A pointer to the matching macro if found, or NULL otherwise.
 */
macro_node* is_macro_exists(char* name) {
    macro_node* current = macro_head;

    while (current && !IS_EMPTY()) {
        if (strncmp(current->name, name, strlen(current->name)) == 0)
            return current; /* matching macro found */
        current = current->next;
    }

    return NULL; /* no matching macro found */
}


/**
 * Frees the memory allocated for the linked list of macros,
 * including the memory allocated for macro names and bodies.
 * After freeing the memory, the macro list is empty.
 */
void free_macros() {
    macro_node* current = macro_head;
    macro_node* next;

    while (current && !IS_EMPTY()) {
        next = current->next;
        free(current->name);
        free(current->body);
        free(current);
        current = next;
    }

    macro_head = NULL;
    macro_tail = NULL;
}


