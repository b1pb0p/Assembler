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

#define UPDATE_REPORT_STATUS(condition, file) if ((condition) != NO_ERROR) { \
cleanup(*(file)); \
return FAILURE; \
}

#define FREE_AND_NULL(p_mem) if ((p_mem)) { \
free ((p_mem)); \
(p_mem) = NULL;                             \
}

symbol **symbol_table = NULL;
data_image **data_img_obj = NULL;
data_image **data_img_ent = NULL;
data_image **data_img_ext = NULL;

size_t symbol_count = 0;
size_t data_arr_obj_index = 0;
size_t data_arr_ent_index = 0;
size_t data_arr_ext_index = 0;
int DC = 0;
int IC = 0;

status assembler_first_pass(file_context **src) {
    char line[MAX_BUFFER_LENGTH];
    file_context *p_src = NULL;
    status report = NO_ERROR;

    p_src = *src;

    if (!p_src)
        return FAILURE;

    /* Checking for comment lines (;), invalid line start and handling too long lines
     * is taken care of at the preprocessor stage. */
    while (fscanf(p_src->file_ptr, "%[^\n]%*c", line) == 1 && report != ERR_MEM_ALLOC) {
        report =  process_line(p_src,line);
        p_src->lc++;
    }

    /* Generate output file(s) only if no error has occurred */
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

void test_out(file_context *src) { /*TODO: REMOVE */
    size_t i;
    int error_flag = 0;
    data_image *runner = NULL;
    printf("\n***** TESTING *****\n");

    for (i = 0; i < data_arr_obj_index; i++) {
        runner = data_img_obj[i];
        if (!runner->value && runner->p_sym) {
            if (runner->p_sym->data)// && runner->p_sym->data->value)
                runner->value = runner->p_sym->data->value;
            else {
                error_flag = 1;
                handle_error(ERR_LABEL_DOES_NOT_EXIST, src, runner);
                continue;
            }
        }
        if (!error_flag) printf("Data_image: @%p, value: %d\n",data_img_obj[i], *(data_img_obj[i]->value));
    }

    if (error_flag) {
        //clean-up
        printf("Exiting ...\n");
        return;
    }

    printf("DC: %lu\t | Actual Memory allocation for data_image: %lu\nBase-64 Words:\n",
           (unsigned long)DC, (unsigned long)data_arr_obj_index);
    for (i = 0; i < data_arr_obj_index; i++) {
        create_base64_word(data_img_obj[i]);
        printf("Data_image: @%p, value: %s\n",data_img_obj[i], data_img_obj[i]->base64_word);
    }
}

status process_line(file_context *src, char *p_line) {
    char first_word[MAX_LABEL_LENGTH];
    int is_valid_label_exist;
    symbol *sym = NULL;
    status report = NO_ERROR;
    size_t word_len = get_word(&p_line, first_word, SPACE);

    is_valid_label_exist = is_label(src, first_word, &report)
                           && (sym = declare_label(src, first_word, word_len, &report)) && report != ERR_MEM_ALLOC;

    /* New label declaration following a Directive or a Command */
    if (is_valid_label_exist) {
        process_line_w_label(src, p_line, sym, &report);
    }
    else if (report != ERR_MEM_ALLOC){ /* Process line without any associated label */
        //process_line_wout_label(src, p_line, &report);
    }
    return report;
}

void process_line_w_label(file_context *src,char *line, symbol *sym, status *report) {
    char next_word[MAX_LABEL_LENGTH];
    data_image *p_data_image = NULL;
    Directive dir = 0;
    Command cmd = 0;
    size_t word_len;
    int condition;

    word_len =  get_word(&line, next_word, SPACE);

    condition = word_len && !is_label(src, next_word, NULL) &&
                ((dir = is_directive(next_word + 1) || (cmd = is_command(next_word))));

    if (!condition)  {
        *report = FAILURE;
        handle_error(ERR_INVALID_ACTION, src, "label" ,!word_len ? "[End of line]" : next_word);
        return;
    }

    if (cmd == RTS || cmd == STOP) {
        p_data_image->binary_opcode = decimal_to_binary12(cmd);
        if (!p_data_image->binary_opcode) *report = FAILURE;
    }
    else if (dir == ENTRY || dir == EXTERN) {
        handle_error(ERR_MEANINGLESS_LABEL, src);
        /* *report = dir == ENTRY ? process_entry() : process_extern(); */
    }
    else if (dir == STRING || dir == DATA) {
        DC++;
        // dir == STRING ? process_string() : process_data(src, sym->label, line, report);
    }
}

/**
 * Processes .data information from a given file context, label, and line.
 *
 * @param src The file_context pointer.
 * @param label The label associated with the data (optional - NULL).
 * @param line The line containing the data.
 * @param report Pointer to the status variable to store error reports.
 */
void process_data(file_context *src, const char *label, char *line, status *report) {
    char word[MAX_LABEL_LENGTH];
    int is_first_value = 0;
    int *value = NULL;
    status temp_report = NO_ERROR;
    Value val_type;
    data_image *p_data = NULL;

    while (*line != '\n' && *line != '\0' && get_word(&line,word,COMMA) != 0) {
        val_type = line_parser(src, DATA, &line, word, &temp_report);
        *report = temp_report == NO_ERROR ? *report : temp_report;

        if (temp_report == ERR_EXTRA_COMMA || temp_report == ERR_INVALID_DATA) {
            is_first_value = is_first_value ? is_first_value : 1;
            continue;
        }

        assert_data_img_by_label(src, label, &is_first_value, &value, &p_data, report);
        if (*report == ERR_MEM_ALLOC)
            return;

        is_first_value = is_first_value ? is_first_value : 1;
        if((temp_report = assert_value_to_data(src, DATA, val_type, word, &value, &p_data, report)) == TERMINATE)
            return;
        else if (temp_report == FAILURE)
            continue;

        DC++;
        p_data->value = value;
        p_data->concat = VALUE;
    }

    if (!is_first_value) /* Missing action after .data */
        handle_error(ERR_INVALID_DATA, src);
}

void process_string(file_context *src, const char *label, char *line, status *report) {
    char p_ch;
    char *word = NULL;
    char *p_word = NULL;
    data_image *p_data = NULL;
    int *value = NULL;
    int is_first_value = 0;
    int is_first_char;
    status temp_report = NO_ERROR;
    Value val_type;

    while (is_valid_string(&line, &word, report)) { /* Process each string */
        val_type = line_parser(src, STRING, &line, word, &temp_report);
        *report = temp_report == NO_ERROR ? *report : temp_report;

        if (temp_report == ERR_EXTRA_COMMA || temp_report == ERR_INVALID_STRING) {
            is_first_value = is_first_value ? is_first_value : 1;
            FREE_AND_NULL(word);
            continue;
        }
        is_first_char = 0;
        p_word = word;
        while (string_parser(src, &word, &p_ch, report) == NO_ERROR) { /* Process each character */
            assert_data_img_by_label(src, label, &is_first_value, &value, &p_data, report);
            if (*report == ERR_MEM_ALLOC)
                return;
            else if (val_type == LBL)
                assert_data_img_by_label(src, label, &is_first_value, &value, &p_data, report);
// todo : if valid label...
            *value = (int)p_ch;
            if (!is_first_char && !isalpha(*value))
                handle_error(ERR_ILLEGAL_CHARS, src, "label", word);

            is_first_char = is_first_value =  1;
            temp_report = assert_value_to_data(src, STRING, val_type, &p_ch, &value, &p_data, report);

            if (temp_report == TERMINATE) {
                FREE_AND_NULL(p_word);
                return;
            }
            else if (temp_report == FAILURE)
                continue;

            DC++;
            p_data->value = value;
            p_data->concat = VALUE;
        }
       FREE_AND_NULL(p_word);
    }
    if (!is_first_value) /* Missing action after .string */
        handle_error(ERR_INVALID_STRING, src);
    FREE_AND_NULL(p_word);

}

Value line_parser(file_context *src, Directive dir, char **line, char *word, status *report) {
    size_t length;

    if (dir != STRING && dir != DATA) {
        *report = TERMINATE;
        handle_error(TERMINATE, "line_parser()");
        return INV;
    }

    while (*word && isspace(*word))
        word++;

    length = strlen(word);

    while (*word == ',' && length >= 1) {
        word++;
        length--;
        *report = ERR_EXTRA_COMMA;
        handle_error(ERR_EXTRA_COMMA, src);
    }

    if (!length) {
        *report = dir == STRING ? ERR_INVALID_STRING : ERR_INVALID_DATA;
        return INV;
    }


    if (word[length - 1] != ',') {
        while (**line && isspace(**line))
            (*line)++;
        if (**line != ',' && **line != '\0') {
            *report = ERR_MISS_COMMA;
            handle_error(ERR_MISS_COMMA, src);
        }
        else if (**line == ',')
            (*line)++;
    }
    else {
        if (**line == '\0' || **line == '\n') {
            *report = ERR_EXTRA_COMMA;
            handle_error(ERR_EXTRA_COMMA, src);
        }
        word[length - 1] = '\0';
    }

    if (dir == DATA)
        return validate_data(src, word, length, report);
    else
        return validate_string(src, word, length, report);
}

status string_parser(file_context *src, char **word, char *ch, status *report) {
    static int is_first_qmark = 0;
    status ret_val = NO_ERROR;

    if (**word == '\"') {
        if (!is_first_qmark) {
            is_first_qmark = 1;
            (*word)++;
        }
        else {
            is_first_qmark = 0;
            (*word)++;
            ch = NULL;
            *report = (**word != '\0' && **word != '\n') ? ERR_EXTRA_TEXT : *report;
            return TERMINATE;
        }
    }
    else {
       if ((*word)[1] == '\0' || (*word)[1] == '\n') {
            is_first_qmark = 0;
            ret_val = TERMINATE;
            *report = ERR_MISSING_QMARK;
            handle_error(ERR_MISSING_QMARK, src);
        }  else if (!is_first_qmark) {
           is_first_qmark = 1;
           *report = ERR_MISSING_QMARK;
           handle_error(ERR_MISSING_QMARK, src);
       }
    }
    *ch = **word;
    (*word)++;
    return ret_val;
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

    for (i = 0; i < data_arr_obj_index; i++) {
        if (!(*data_img_obj[i]).is_word_complete)
            report = FAILURE;

        // (*data_img_obj[i]).base64_word = convert_bin_to_base64((*data_img_obj)->symbol_t->address_binary);
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
 * @param target The target Directive (ENTRY or EXTERN).
 * @return The status of the operation. Returns NO_ERROR on success, or FAILURE if an error occurred.
 */
status generate_directive_output(const char *file_name, char *ext, Directive target) {
    status report = NO_ERROR;
    size_t i, boundary;
    data_image **p_data = NULL;
    symbol *p_sym = NULL;
    file_context *dest = create_file_context(file_name, ext, FILE_EXT_LEN_OUT, FILE_MODE_WRITE, &report);

    boundary = target == EXTERN ? data_arr_ext_index : data_arr_ent_index;
    p_data = target == EXTERN ? data_img_ext : data_img_ent;

    if (!dest)
        return FAILURE;

    if (!p_data || !*p_data)
        return NO_ERROR;

    for (i = 0; i < boundary; i++) {
        //p_sym = p_data[i]->symbol_t;
        fprintf(dest->file_ptr, "%s\t%d\n", p_sym->label, p_sym->address_decimal);
    }
    free_file_context(&dest);
    return report;
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

    if (existing_symbol) {
        if (address == INVALID_ADDRESS)
            return existing_symbol;
        else if (existing_symbol->is_missing_info)
            return update_symbol_info(existing_symbol, address) == NO_ERROR ? existing_symbol : NULL;
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
        if (address == INVALID_ADDRESS) {
            new_symbol->is_missing_info = 1;
            new_symbol->address_decimal = address;
        }
        else {
            new_symbol->address_decimal = address;
            new_symbol->address_binary = decimal_to_binary12(address);
            new_symbol->is_missing_info = 0;
        }
        new_symbol->data = NULL;
        // new_symbol->data = create_data_image(src->lc);
        symbol_table = new_symbol_table;
        symbol_table[symbol_count++] = new_symbol;

        return new_symbol;
    }
}

/**
 * Create a new data image with default properties and add it to the data image object array.
 *
 * @param src A pointer to a file_context struct.
 * @param label The label associated with the data image. Pass NULL if no label is present.
 * @param address The address value of the data image. Pass INVALID_ADDRESS to create a new address.
 * @param report A pointer to a status report variable.
 * @return A pointer to the newly created data image, or NULL in case of errors.
 */
data_image* add_data_image_default(file_context *src, const char* label, status *report) {
    static size_t data_obj_cap = 0;
    size_t new_cap = data_obj_cap + DEFAULT_DATA_IMAGE_CAP;
    data_image **data_arr = NULL;
    data_image *new_image = create_data_image(src->lc); /* creates new data_image, setting the lc in which declared */

    if (!new_image || (data_obj_cap <= data_arr_obj_index &&
                       !(data_arr = realloc(data_img_obj, (new_cap) * sizeof(data_image *))))) {
        *report = ERR_MEM_ALLOC;
        return NULL;
    }

    if (label) {
        /* Label is always already declared at this point */
        if (!find_symbol(label) || !(find_symbol(label)->data = new_image)) {
            handle_error(TERMINATE, "add_data_image_default()");
            free_data_image(&new_image);
            return NULL;
        }
    }

    if (data_arr) {
        data_img_obj = data_arr;
        data_obj_cap = new_cap;
    }
    data_img_obj[data_arr_obj_index] = new_image;
    data_arr_obj_index++;

    return new_image;
}

symbol *declare_label(file_context *src, char *label, size_t label_len, status *report) {
    if (!strncpy(label, label, label_len - 1)) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    label[label_len - 1] = '\0';

    return add_symbol(src, label, (DC+IC) + ADDRESS_START, report);
}

/**
 * Checks if a string is a valid label.
 *
 * @param label The string to check.
 * @return NO_ERROR if the string is a valid label, an appropriate error status otherwise.
 *
 * @remarks The function checks if the label meets the following criteria:
 *   - The label is not NULL and has a length between 1 and MAX_LABEL_LENGTH characters.
 *   - The label does not match any reserved command or directive.
 *   - The label does not start with a digit.
 *   - The label consists only of alphanumeric characters.
 *   - The label ends with a colon (':') to indicate a label declaration.
 */
status is_valid_label(const char *label) {
    size_t length = strlen(label);
    int i;

    if (!label || length == 0  || length > MAX_LABEL_LENGTH ||
        is_command(label) || is_directive(label + 1))
        return ERR_INVALID_LABEL;

    if (isdigit(*label))
        return ERR_LABEL_START_DIGIT;

    if (!isalpha(*label))
        return ERR_ILLEGAL_CHARS;

    for (i = 1; i < length; i++)
        if (!isalnum(label[i]))
            return ERR_ILLEGAL_CHARS;

    if (label[length - 1] == ':')
        return NO_ERROR;

    return ERR_MISS_COLON;
}

int is_label(file_context *src, const char *label, status *report) {
    status ret_val = is_valid_label(label);

    if (!report)  /* Do not print error messages if report is NULL */
        return ret_val == ERR_INVALID_LABEL ? 0 : 1;
    if (ret_val == NO_ERROR)
        return 1;

    *report = ret_val;

    if (ret_val == ERR_INVALID_LABEL) {
        handle_error(ERR_INVALID_LABEL, src, label);
        return 0;
    }

    ret_val == ERR_MISS_COLON ? handle_error(ret_val,src) : handle_error(ret_val, src, label);
    return 1;
}

Value validate_data(file_context *src, char *word, size_t length, status *report) {
    char *p_word = NULL;

    if (*word == '+' || *word == '-')
        word++;

    if (isalpha(*word)) {
        if (word[length - 1] == ':') {
            word[length - 1] = '\0';
            handle_error(ERR_FORBIDDEN_LABEL_DECLARE, src);
            *report =  ERR_FORBIDDEN_LABEL_DECLARE;
        }
        return LBL;
    }
    else if (isdigit(*word)) {
        p_word = word;
        while (!isspace(*p_word) && *p_word != '\0' && *p_word != '\n') {
            if (!isdigit(*p_word)) {
                handle_error(ERR_INVALID_DATA, src);
                *report = ERR_INVALID_DATA;
                return INV;
            }
            p_word++;
        }
        return NUM;
    }
    return INV;
}

Value validate_string(file_context *src, char *word, size_t length, status *report) {
    if (*word == '\"') {
        if (word[length - 1] != '\"') {
            *report = ERR_MISSING_QMARK;
            handle_error(ERR_MISSING_QMARK, src);
        }
        return isalpha(word[1]) ? STR : INV;
    }
    else {
        if (word[length - 1] == '\"') {
            *report = ERR_MISSING_QMARK;
            handle_error(ERR_MISSING_QMARK, src);
            return isalpha(word[1]) ? STR : INV;
        }
        return LBL;
    }
}


int is_valid_string(char **line, char **word, status *report) {
    if (**line == '\0' || **line == '\n')
        return 0;

    while (**line && isspace(**line))
        (*line)++;

    *word = malloc(sizeof(char) * get_word_length(line) + 1);

    if (!*word || !get_word(line, *word, COMMA)) {
        *report = ERR_MEM_ALLOC;
        handle_error(ERR_MEM_ALLOC);
        return 0;
    }

    return 1;
}

void assert_data_img_by_label(file_context *src, const char *label, int *flag, int **value, data_image **p_data, status *report) {
    if (label && !*flag) {
        *flag = 1;
        if (!(*p_data = add_data_image_default(src, label, report)) && *report == ERR_MEM_ALLOC)
            return;
    }
    else {/* A label can only be associated with the initial value */
        if (!(*p_data = add_data_image_default(src, NULL, report)) && *report == ERR_MEM_ALLOC)
            return;
    }

    *value = malloc(sizeof (int));
    if (!*p_data || !*value) {
        handle_error(TERMINATE, "process_data");
        *report = TERMINATE;
        return;
    }
}

status assert_value_to_data(file_context *src, Directive dir, Value val_type ,
                            char *word, int **value, data_image **p_data, status *report) {
    symbol *sym = NULL;
    status temp_report = is_valid_label(word);

    if (dir == DATA && val_type == NUM)
        **value = atoi(word); // NOLINT(cert-err34-c)
    else if (dir == STRING && val_type == STR)
        **value = (int)*word;
    else if (temp_report == ERR_MISS_COLON && val_type == LBL) { /* A label (usage) within statement */
        sym = add_symbol(src, word, INVALID, report);
        if (sym && sym->data && sym->data->value)
            *value = sym->data->value;
        else if (sym) {
            (*p_data)->p_sym = sym;
            free(*value);
            *value = NULL;
        }
        else {
            free_data_image(&data_img_obj[data_arr_obj_index--]);
            return TERMINATE;
        }
    }
    else {
        if (temp_report == ERR_INVALID_LABEL || temp_report == ERR_ILLEGAL_CHARS)
           temp_report == ERR_INVALID_LABEL ? handle_error(temp_report, src, word) :
           handle_error(temp_report, src, "label" ,word);
        *report = dir == DATA ? ERR_INVALID_DATA : ERR_INVALID_STRING;
        free_data_image(&data_img_obj[data_arr_obj_index--]);
        return FAILURE;
    }
    return NO_ERROR;
}

int is_valid_register(const char* str) {
    if (str[0] == '@' && str[1] == 'r' &&
        str[2] >= '0' && str[2] <= '7' &&
        (str[3] == '\0' || isspace(str[3]) || str[3] == ','))
        return 1;
    else
        return 0;
}

/**
 * Frees the global data image arrays and symbol table.
 */
void free_global_data_and_symbol() {
    free_data_image_array(&data_img_obj, &data_arr_obj_index);
    free_data_image_array(&data_img_ent, &data_arr_ent_index);
    free_data_image_array(&data_img_ext, &data_arr_ext_index);
    free_symbol_table(&symbol_table, &symbol_count);
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
