/* first_pass.c
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "passes.h"
#include "utils.h"
#include "errors.h"
#include "data.h"


#define IS_ENTRY(word) is_dot_directive(word, "entry")
#define IS_EXTERN(word) is_dot_directive(word, "extern")
#define IS_DATA(word) is_dot_directive(word, "data")
#define IS_STRING(word) is_dot_directive(word, "string")

#define UPDATE_REPORT_STATUS(condition, file) if ((condition) != NO_ERROR) { \
cleanup(*(file)); \
return FAILURE; \
}

symbol **symbol_table = NULL;
data_image **data_img_obj = NULL;
data_image **data_img_ent = NULL;
data_image **data_img_ext = NULL;

size_t symbol_count = 0;
size_t data_img_obj_count = 0;
size_t data_img_ent_count = 0;
size_t data_img_ext_count = 0;
size_t DC = 0;
size_t IC = 0;

status assembler_first_pass(file_context **src) {
    char line[MAX_BUFFER_LENGTH];
    file_context *p_src = NULL;
    status report = NO_ERROR;

    p_src = *src;

    if (!p_src)
        return FAILURE;

    /* Checking for comment lines (;), invalid line start and handling too long lines
     * is taken care of at the preprocessor stage. */
    while (fscanf(p_src->file_ptr, "%[^\n]%*c", line) == 1) {
        report =  process_line(p_src,line);
        p_src->lc++;
    }

    /* Generate object output file only if no error occurred */
    if (report == NO_ERROR)
        report = assembler_second_pass(src);
    else /* Cleanup output files if an error occurred */
        cleanup(src);

    return report;
}

/**
 * Perform the second pass of the assembler, generating the object output.
 *
 * @param src Pointer to the file context for the input and output file information.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status assembler_second_pass(file_context **src) {
    status report;
    file_context *p_src = *src;

    if (!p_src)
        return FAILURE;

    report = generate_obj_output(p_src->file_name_wout_ext, IC, DC);
    UPDATE_REPORT_STATUS(report, &src);
    report = generate_directive_output(p_src->file_name_wout_ext, ENTRY_EXT, ENTRY);
    UPDATE_REPORT_STATUS(report, &src);
    report = generate_directive_output(p_src->file_name_wout_ext, EXTERNAL_EXT, EXTERN);
    UPDATE_REPORT_STATUS(report, &src);

    free_global_data_and_symbol();
    return report;
}


/*** NEED TO BE WRITTEN AGAIN ***/
status process_line(file_context *src, char *p_line) {
    char word[MAX_LABEL_LENGTH];
    char next_word[MAX_LABEL_LENGTH];
    char *first_char = p_line;
    status report = NO_ERROR;
    size_t word_len;

    (void)get_word(&p_line, word);
    word_len = get_word(&p_line, next_word); /* in order to unget */

    if (is_label(src, word)) {
        if((report = process_label(src, word)))
            return report;
        if (IS_DATA(next_word)) {
            process_data(src, word, p_line);
        } else if (IS_ENTRY(next_word)) {
            return ERR_DUP_LABEL; /* TODO: process_entry(); */
        } else if (IS_EXTERN(next_word)) {
            return ERR_FIRST_PASS; /* TODO: process_extern(); */
        } else if (IS_STRING(next_word))
            return NO_ERROR; /* TODO: process_string(); */
    }
    else {
        unget_word(&p_line, word_len, first_char);
        if (IS_DATA(word))
            report = process_data(src, NULL, p_line);
        else if (IS_ENTRY(word))
            return ERR_PRE; /* TODO: process_entry(); */
        else if (IS_EXTERN(word))
            return ERR_EXTRA_TEXT; /* TODO: process_extern(); */
        else if (IS_STRING(word))
            return ERR_LINE_TOO_LONG; /* TODO: process_string(); */
    }




    if (is_label(src, word)) {
        report = process_label(src, word);
        if (report != NO_ERROR)
            return report;
        (void)get_word(&p_line, word);
    }

    if (is_command(word)) {
        char first_param[MAX_LABEL_LENGTH], second_param[MAX_LABEL_LENGTH];
        report = process_command_word(src, p_line, first_param, second_param);
        if (report == NO_ERROR) {
            /* TODO: FILL WITH CODE HOPEFULLY */
        }
    }

    return report;
}

status process_data(file_context *src, char *label, char *line) {
    char word[MAX_LABEL_LENGTH];
    int is_first_value = 0;
    data_value value;
    status report;

    while (*line != '\n' && *line != '\0') {
        (void)get_word(&line, word);

        if (is_data_valid(word) != NO_ERROR) {
            handle_error(ERR_INVAL_DATA, src);
            return FAILURE;
        }

        value.decimal_value = atoi(word); // NOLINT(cert-err34-c)
        DC++;

        if (label && !is_first_value) {
            is_first_value = 1;
            if (!add_data_image_default(src, label, src->lc, &report)) report = FAILURE;
        }
        else /* A label can only be associated with the initial value */
            if (!add_data_image_default(src, NULL, INVALID, &report))
            report = FAILURE;

        if (report != NO_ERROR)
            return FAILURE;
        data_img_obj[data_img_obj_count]->value = value;
    }
    return NO_ERROR;
}

//status process_entry(file_context *src, char *label, char *line) {
//    char word[MAX_LABEL_LENGTH];
//    data_image *p_data = NULL;
//    status report;
//
//    IC++;
//    (void)get_word(&line, word);
//    /* p_data = add_data_image(src, word, ENTRY); */
//    return NO_ERROR;
//}

/**
 * Checks if a word is a dot directive, such as '.data', '.string', etc.
 *
 * @param word The word to check.
 * @param directive The specific dot directive string to compare with.
 * @return 1 if the word is a dot directive, 0 otherwise.
 */
int is_dot_directive(const char *word, const char *directive) {
    if (word && *word == '.' && strcmp(word + 1, directive) == 0 && *(word + 1 + strlen(directive)) == '\0')
        return 1;
    return 0;
}

/**
 * Checks if a string is a valid label.
 *
 * @param label The string to check.
 * @param src A file_context pointer.
 * @return 1 if the string is a valid label, 0 otherwise.
 */
int is_label(file_context *src, const char *label) {
    size_t length = strlen(label);
    if (length == 0 || length > MAX_LABEL_LENGTH)
        return 0;
    if (is_command(label) || is_directive(label) || isdigit(*label)) {
        handle_error(ERR_INVAL_LABEL, src, label);
        return 0;
    }
    if (label[length - 1] == ':') /* TODO: TEST! */
        return 1;

    handle_error(ERR_MISS_COLON,src);

    return 0;
}
/**
 * Checks if the given data value is valid.
 *
 * @param word The data value to be checked.
 * @return The status indicating if the data value is valid or not.
 *         It returns NO_ERROR if the data value is valid, FAILURE otherwise.
 */
status is_data_valid(const char *word) {
    int error_flag = 0;

    if (word[0] == '\0')
        error_flag = 1;

    if (!error_flag && word[0] == '+' || word[0] == '-')
        word++;

    while (!error_flag && *word) {
        if (!isdigit((unsigned char)*word))
            error_flag = 1;
        word++;
    }

    return error_flag ? FAILURE : NO_ERROR;
}

status process_label(file_context *src, const char *label) {
    status report = NO_ERROR;

    (void)add_symbol(src, label, src->ic + ADDRESS_START, &report);

    return report;
}

//status process_command_word(file_context *src, char *p_line, char *first_param, char *second_param) {
//    (void)get_word(&p_line, first_param);
//
//    if (*p_line == ',') {
//        p_line++;
//        (void)get_word(&p_line, second_param);
//
//        if (*p_line != '\n' && *p_line != '\0') {
//            handle_error(TERMINATE, "Invalid command format");
//            return FAILURE;
//        }
//    } else {
//        if (*p_line != '\n' && *p_line != '\0') {
//            handle_error(TERMINATE, "Invalid command format");
//            return FAILURE;
//        }
//    }
//
//    return NO_ERROR;
//}
/*** UP HERE BUDDY ***/

/**
 * Add a new data image to the appropriate global data image array based on the directive.
 *
 * @param src The file context containing input and output file information.
 * @param label The label associated with the data image (optional).
 * @param dir The directive indicating the type of data image (DEFAULT, EXTERN, or ENTRY).
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
//data_image* add_data_image(file_context *src, const char* label, directive dir) {
//    symbol* sym = NULL;
//    data_image* new_image = create_data_image(src->lc);
//
//    if (!new_image)
//        return NULL;
//
//    if (label) {
//        sym = find_symbol(label);
//        if (sym) {
//            new_image->symbol_t = sym;
//        }
//        if (dir == DEFAULT)
//            add_symbol(src, label, src->lc + STARTING_ADDRESS);
//        else
//            add_symbol(src, label, );
//    }
//
//
//    if (dir == DEFAULT) {
//        data_img_obj[data_img_obj_count] = new_image;
//        data_img_obj_count++;
//    } else if (dir == EXTERN) {
//        data_img_ext[data_img_ext_count] = new_image;
//        data_img_ext_count++;
//    } else if (dir == ENTRY) {
//        data_img_ent[data_img_ent_count] = new_image;
//        data_img_ent_count++;
//    }
//    else {
//        handle_error(TERMINATE, "add_data_image()");
//        free_data_image(&new_image);
//        return NULL;
//    }
//    return new_image;
//}


data_image* add_data_image_default(file_context *src, const char* label, int address, status *report) {
    symbol* sym = NULL;
    char *dummy_label = NULL;
    data_image* new_image = create_data_image(src->lc);

    if (!new_image) {
        *report = ERR_MEM_ALLOC;
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    if (label && isdigit(*label)) {
        *report = FAILURE;
        handle_error(ERR_LABEL_START_DIGIT, src, label);
        return NULL;
    }

    if (label) {
        sym = find_symbol(label);
        if (sym)
            new_image->symbol_t = add_symbol(src, label, address, report);
        else
            (void)add_symbol(src, label, address, report);
    }
    else if ((dummy_label = create_dummy_label()))
        (void)add_symbol(src, dummy_label, address, report);

    data_img_obj[data_img_obj_count] = new_image;
    data_img_obj_count++;
    return new_image;
}








/**
* Generates the output file for the object code.
*
* Generate the object output file based on the data image and symbol table.
*
* @param file_name The name of the output file.
* @param ic The instruction count (IC).
* @param dc The data count (DC).
* @return The status of the operation. Returns NO_ERROR on success, or FAILURE if an error occurred.
*/
status generate_obj_output(const char *file_name, size_t ic, size_t dc) {
    int i;
    status report = NO_ERROR;
    file_context *obj_file = create_file_context(file_name, OBJECT_EXT, FILE_EXT_LEN, FILE_MODE_WRITE, &report);

    if (!obj_file)
        return FAILURE;

    fprintf(obj_file->file_ptr, "%lu %lu\n", (unsigned long)ic, (unsigned long)dc);

    for (i = 0; i < data_img_obj_count; i++) {
        if ((*data_img_obj[i]).missing_info)
            report = FAILURE;

        (*data_img_obj[i]).base64_word = convert_bin_to_base64((*data_img_obj)->symbol_t->address_binary);
        if (!(*data_img_obj[i]).base64_word)
            report = FAILURE;

        fprintf(obj_file->file_ptr, "%s\n", (*data_img_obj[i]).base64_word);
    }
    free_file_context(&obj_file);
    return report;
}

/**
 * Generates the output file for entry or extern directives.
 *
 * This function generates the output file for entry or extern directives, containing
 * the corresponding symbol labels and their decimal addresses.
 *
 * @param file_name The name of the output file.
 * @param ext The file extension for the output file.
 * @param target The target directive (ENTRY or EXTERN).
 * @return The status of the operation. Returns NO_ERROR on success, or FAILURE if an error occurred.
 */
status generate_directive_output(const char *file_name, char *ext, directive target) {
    status report = NO_ERROR;
    size_t i, boundary;
    data_image **p_data = NULL;
    symbol *p_sym = NULL;
    file_context *dest = create_file_context(file_name, ext, FILE_EXT_LEN_OUT, FILE_MODE_WRITE, &report);

    boundary = target == EXTERN ? data_img_ext_count : data_img_ent_count;
    p_data = target == EXTERN ? data_img_ext : data_img_ent;

    if (!dest)
        return FAILURE;

    if (!p_data || !*p_data)
        return NO_ERROR;

    for (i = 0; i < boundary; i++) {
        p_sym = p_data[i]->symbol_t;
        fprintf(dest->file_ptr, "%s\t%d\n", p_sym->label, p_sym->address_decimal);
    }
    free_file_context(&dest);
    return report;
}

/**
 * Frees the global data image arrays and symbol table.
 */
void free_global_data_and_symbol() {
    free_data_image_array(&data_img_obj, &data_img_obj_count);
    free_data_image_array(&data_img_ent, &data_img_ent_count);
    free_data_image_array(&data_img_ext, &data_img_ext_count);
    free_symbol_table(&symbol_table, &symbol_count);
}

/**
 * Add a symbol to the symbol table.
 * If the symbol already exists and has missing information, its information is updated with the provided address.
 * If the symbol already exists and has complete information, NULL is returned.
 * If the symbol is new, it is added to the symbol table with the provided label and address.
 *
 * @param src A pointer to a file_context.
 * @param label The label of the symbol to add.
 * @param address The address of the symbol.
 * @param report A pointer to a status report variable.
 * @return A pointer to the added symbol if it is new or has missing information, or NULL if the symbol already exists with complete information.
 *         Returns NULL in case of memory allocation errors during symbol creation or table expansion.
 */
symbol* add_symbol(file_context *src, const char* label, int address, status *report) {
    symbol** new_symbol_table = NULL;
    symbol* new_symbol = NULL;
    symbol* existing_symbol = NULL;
    existing_symbol = find_symbol(label);

    *report = NO_ERROR;

    if (existing_symbol) {
        if (existing_symbol->is_missing_info && address != INVALID_ADDRESS)
            update_symbol_info(existing_symbol, address);
        else {
            handle_error(ERR_DUP_LABEL, src);
            *report = ERR_DUP_LABEL;
            return NULL;
        }
    } else {
        new_symbol_table = realloc(symbol_table, (symbol_count + 1) * sizeof(symbol*));
        new_symbol = malloc(sizeof(symbol));

        if (!new_symbol_table || !new_symbol || copy_string(&(new_symbol->label), label) != NO_ERROR)    {
            handle_error(ERR_MEM_ALLOC);
            free_symbol(&new_symbol);
            free_symbol_table(&symbol_table, &symbol_count);
            *report = ERR_MEM_ALLOC;
            return NULL;
        }
        if (address == INVALID_ADDRESS)
            new_symbol->is_missing_info = 1;
        else {
            new_symbol->address_decimal = address;
            new_symbol->address_binary = decimal_to_binary12(address);
            new_symbol->is_missing_info = 0;
        }

        symbol_table = new_symbol_table;
        symbol_table[symbol_count++] = new_symbol;

        return new_symbol;
    }

    return existing_symbol;
}

/**
 * Find a symbol in the symbol table based on its label.
 *
 * @param label The label to search for in the symbol table.
 * @return A pointer to the symbol if found, or NULL if the label is not found.
 */
symbol* find_symbol(const char* label) {
    size_t i;

    if (!label) return NULL;

    for (i = 0; i < symbol_count; ++i)
        if (symbol_table[i] && strcmp(symbol_table[i]->label, label) == 0)
            return symbol_table[i];
    return NULL;
}

/**
 * Updates the information of an existing symbol with a new address.
 *
 * @param existing_symbol The existing symbol to update.
 * @param address The new address to assign to the symbol.
 * @return The status of the update operation.
 *         Returns NO_ERROR if the update was successful,
 *         or FAILURE if the conversion to binary failed.
 */
status update_symbol_info(symbol* existing_symbol, int address) {
    existing_symbol->address_decimal = address;
    existing_symbol->address_binary = decimal_to_binary12(address);

    if (!existing_symbol->address_binary)
        return FAILURE;

    existing_symbol->is_missing_info = 0;
    return NO_ERROR;
}

/**
 * Create a dummy label using a static counter and "unnamed_label".
 * The label is dynamically allocated and should be freed by the caller.
 *
 * @return Pointer to the dynamically allocated label string.
 *         Returns NULL if memory allocation fails.
 */
char* create_dummy_label() {
    static int counter = 0;
    char buffer[50];
    char* label;

    snprintf(buffer, sizeof(buffer), "%d_unnamed_label", counter++);

    label = (char*)malloc((strlen(buffer) + 1) * sizeof(char));
    if (!label) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    return copy_string(&label, buffer) == NO_ERROR ? label : NULL;
}

/**
 * Cleans up a file_context and associated resources.
 * Closes the file, removes the file from the filesystem,
 * frees the file_context memory, and clears the pointer.
 *
 * @param src A pointer to the file_context pointer.
 *            The pointer will be set to NULL after cleanup.
 */
void cleanup(file_context **src) {
    file_context *p_src = *src;

    fclose(p_src->file_ptr);
    remove(p_src->file_name);
    free_file_context(&p_src);
    *src = NULL;
    free_global_data_and_symbol();
}
