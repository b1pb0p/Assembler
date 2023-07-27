/* first_pass.c
 * Assembler's first and second passes process.
 * Author: Bar Toplian
 * ID: 323869065
 * Email: bar.toplian@gmail.com
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

/** Global variables are reset to zero during cleanup **/
symbol **symbol_table = NULL;
data_image **data_img_obj = NULL;

size_t symbol_count = 0;
size_t data_arr_obj_index = 0;

int DC = 0;
int IC = 0;
int next_free_address = ADDRESS_START;

/**
 * Perform the first pass of the assembler, processing each line and generating symbol table entries.
 *
 * @param src Pointer to the file context for the input file information.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status assembler_first_pass(file_context **src) {
    char line[MAX_BUFFER_LENGTH];
    file_context *p_src = NULL;
    status report = NO_ERROR;
    int has_error = 0, ch = -1;;

    p_src = *src;

    if (!p_src)
        return FAILURE;

    /* The check for comment lines (;), invalid line start, and handling too long lines
     * is taken care of at the preprocessor stage. */
    while ((fscanf(p_src->file_ptr, "%[^\n]%*c", line) == 1  || (ch = fgetc(p_src->file_ptr)) == '\n')
        && report != ERR_MEM_ALLOC) {
        if (ch == '\n') {
            ch = -1;
            continue; /* empty line */
        }

        report =  process_line(p_src,line);
        p_src->lc++;
        has_error = report != NO_ERROR ? 1 : has_error;
        report = report == ERR_MEM_ALLOC ? ERR_MEM_ALLOC : NO_ERROR; /* resetting for next line processing */
    }

    /* Generate output file(s) only if no error has occurred */
    if (!has_error) {
        handle_progress(FIRST_PASS_OK, (*src)->fc, (*src)->tc, (*src)->file_name_wout_ext);
        report = assembler_second_pass(src);
    }
    else {  /* Cleanup output files if an error occurred */
        handle_error(ERR_FIRST_PASS, (*src)->fc, (*src)->tc, (*src)->file_name_wout_ext);
        fclose(p_src->file_ptr);
        p_src->file_ptr = NULL;
        remove(p_src->file_name);
        cleanup(src);
    }

    return has_error ? FAILURE : report;
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

    report = generate_output_by_dest(p_src,  EXTERN); /* .ext output */
    UPDATE_REPORT_STATUS(report, &src);
    report = generate_output_by_dest(p_src,  ENTRY); /* .ent output */
    UPDATE_REPORT_STATUS(report, &src);
    report = generate_output_by_dest(p_src, DEFAULT); /* .obj output */
    UPDATE_REPORT_STATUS(report, &src);

    free_file_context(src);
    free_global_data_and_symbol();
    return report;
}

/**
 * Process a line of assembly code, identifying directives, commands, and labels.
 *
 * @param src Pointer to the file context for the input file information.
 * @param p_line The line of code to process.
 * @return The status of the line processing (NO_ERROR or an error status).
 */
status process_line(file_context *src, char *p_line) {
    char first_word[MAX_LABEL_LENGTH];
    char *p_ch = NULL;
    symbol *sym = NULL;
    int has_directive, has_command;
    status report = NO_ERROR;
    size_t word_len;

    p_ch = p_line;
    word_len = get_word(&p_line, first_word, COLON);
    has_directive = word_len && (is_directive(first_word) || (is_directive(first_word + 1)));
    has_command = word_len && (is_command(first_word)) != INV_CMD;

    if (!has_directive && !has_command && (word_len && (sym = declare_label(src, first_word, word_len, &report)))
        && report != ERR_MEM_ALLOC) /* Label declaration following a Directive or a Command */
        handle_processing_line(src, &p_line, sym, &report);
    else if (!has_directive && !has_command)  /* Faulty label declaration */
        return report;
    else if (report != ERR_MEM_ALLOC) /* Process line without any associated label */
        handle_processing_line(src, &p_ch, NULL, &report);
    else {
        report = TERMINATE;
        handle_error(TERMINATE, "process_line()");
    }
    return report;
}

/**
 * Handle the processing of a line, including labels, directives, and commands.
 *
 * @param src Pointer to the file context for the input file information.
 * @param line The line to process.
 * @param sym The symbol associated with the line (optional - NULL).
 * @param report Pointer to the status variable to store error reports.
 */
void handle_processing_line(file_context *src, char **line, symbol *sym, status *report) {
    char next_word[MAX_LABEL_LENGTH];
    char *p_label = NULL;
    size_t word_len;
    Directive dir = 0;
    Command cmd = INV_CMD;
    int has_directive, has_command;

    word_len =  get_word(line, next_word, SPACE);
    if (sym) p_label = sym->label;
    if (next_word[word_len - 1] == ',') {
        next_word[word_len - 1] = '\0';
        word_len--;
        *report = ERR_EXTRA_COMMA;
        handle_error(ERR_EXTRA_COMMA, src);
    }

    has_directive = word_len && ((dir = is_directive(next_word)) || (dir = is_directive(next_word + 1)));
    has_command = word_len && ((cmd = is_command(next_word)) != INV_CMD);

    if (!has_directive && !has_command)  {
        *report = FAILURE;
        handle_error(ERR_INVALID_ACTION, src, "label" ,!word_len ? "[End of line]" : next_word);
        return;
    }

    if (cmd != INV_CMD && !dir)
        process_command(src, cmd, p_label, *line, report);
    else if (dir == ENTRY || dir == EXTERN)
        process_directive(src, dir, p_label, *line, report);
    else if (dir == STRING || dir == DATA) {
        if (*next_word != '.') {
            handle_error(ERR_MISSING_DOT, src);
            *report = ERR_MISSING_DOT;
        }
        dir == STRING ? process_string(src, p_label, *line, report) : process_data(src, p_label, *line, report);
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
    int is_first_value = 0;
    int *value = NULL;
    status temp_report;
    Value val_type;
    data_image *p_data = NULL;
    char *p_word = NULL;
    char *word = malloc(MAX_LABEL_LENGTH);
    p_word = word;

    while (*line != '\n' && *line != '\0' && get_word(&line,word,COMMA) != 0) {
        temp_report = NO_ERROR;
        val_type = line_parser(src, DATA, &line, &word, &temp_report);
        *report = temp_report == NO_ERROR ? *report : temp_report;

        if (temp_report == ERR_EXTRA_COMMA || temp_report == ERR_INVALID_SYNTAX) {
            if (temp_report == ERR_INVALID_SYNTAX) handle_error(ERR_INVALID_SYNTAX, src, "data", word);
            is_first_value = 1;
            continue;
        }

        if (label && !strncmp(label, word, strlen(label))) {
            *report = ERR_FORBIDDEN_LABEL_DECLARE;
            handle_error(ERR_FORBIDDEN_LABEL_DECLARE, src, label);
            is_first_value = 1;
            continue;
        }

        assert_data_img_by_label(src, label, &is_first_value, &value, &p_data, report);
        if (*report == ERR_MEM_ALLOC) {
            FREE_AND_NULL(p_word);
            return;
        }

        is_first_value = 1;
        if((temp_report = assert_value_to_data(src, DATA, val_type, word, &value, &p_data, report)) == TERMINATE) {
            FREE_AND_NULL(p_word);
            return;
        }
        else if (temp_report == FAILURE)
            continue;

        p_data->value = value;
        p_data->concat = VALUE;
        DC++;
    }
    FREE_AND_NULL(p_word);

    if (!is_first_value) {/* Missing action after .data */
        handle_error(WARN_EMPTY_DIR, src, DATA);
    }
}

/**
 * Processes .string information from a given file context, label, and line.
 *
 * @param src The file_context pointer.
 * @param label The label associated with the string (optional - NULL).
 * @param line The line containing the string.
 * @param report Pointer to the status variable to store error reports.
 */
void process_string(file_context *src, const char *label, char *line, status *report) {
    char p_ch, *word = NULL, *p_word = NULL;
    int is_first_char, is_first_value = 0, *value = NULL;
    data_image *p_data = NULL;
    status temp_report;
    Value val_type;

    while (is_valid_string(&line, &word, report)) { /* Process each string */
        temp_report = NO_ERROR;
        val_type = line_parser(src, STRING, &line, &word, &temp_report);
        *report = temp_report == NO_ERROR ? *report : temp_report;

        if (temp_report == ERR_EXTRA_COMMA || temp_report == ERR_INVALID_SYNTAX) {
            is_first_value = is_first_value ? is_first_value : 1;
            if (temp_report == ERR_INVALID_SYNTAX) handle_error(ERR_INVALID_SYNTAX, src, "string", word);
            free(p_word);
            word = NULL;
            continue;
        }

        if (label && !strncmp(label, word, strlen(label))) {
            *report = ERR_FORBIDDEN_LABEL_DECLARE;
            handle_error(ERR_FORBIDDEN_LABEL_DECLARE, src, label);
            is_first_value = 1;
            continue;
        }

        is_first_char = 0;
        p_word = word;
        while (val_type == LBL || (string_parser(src, &word, &p_ch, report) == NO_ERROR)) { /* Process each character */
            assert_data_img_by_label(src, label, &is_first_value, &value, &p_data, report);
            if (*report == ERR_MEM_ALLOC)
                return;
            *value = (int)p_ch;
            if (val_type != LBL && !is_first_char && !isalpha(*value))
                handle_error(ERR_ILLEGAL_CHARS, src, "string", word);
            else if (!is_first_char)

                is_first_char = is_first_value =  1;
            temp_report = assert_value_to_data(src, STRING, val_type,
                                               val_type == LBL ? p_word: &p_ch, &value, &p_data, report);

            if (temp_report == TERMINATE) {
                FREE_AND_NULL(p_word);
                return;
            } else if (temp_report == FAILURE) {
                if (val_type == LBL) break;
                continue;
            }

            DC++;
            p_data->value = value;
            p_data->concat = VALUE;
            if (val_type == LBL) break;
        }
        FREE_AND_NULL(p_word);
        word = NULL;
    }
    if (!is_first_value) /* Missing action after .string */
        handle_error(WARN_EMPTY_DIR, src, STRING);
    FREE_AND_NULL(p_word);
}

/**
 * process_directive - Process a directive
 *
 * Processes a directive by adding symbols to the symbol table and performing error handling.
 * It handles directives with multiple operands and checks for duplicate or invalid directives.
 *
 * @param src - Pointer to the source file context.
 * @param dir - The directive being processed.
 * @param label - The label associated with the directive (optional - can be NULL).
 * @param line - The line containing the directive.
 * @param report - A pointer to the status report.
 */
void process_directive(file_context *src, Directive dir, const char *label, char *line, status *report) {
    symbol *sym = NULL;
    int has_extern = 0;
    char *word = malloc(MAX_LABEL_LENGTH);

    if (label)
        handle_error(WARN_MEANINGLESS_LABEL, src, label, dir);

    while (*line != '\n' && *line != '\0' && get_word(&line,word,COMMA) != 0) {
        (void) line_parser(src, dir, &line, &word, report);
        sym = add_symbol(src, word, INVALID_ADDRESS, report);
        has_extern = 1; /* Flag for non-empty extern command */

        if (!sym) /* Error handling is taken care of within add_symbol() */
            continue;

        if (sym->sym_dir != DEFAULT && sym->sym_dir != dir) {
            handle_error(ERR_BOTH_DIR, src, sym->label);
            *report = ERR_BOTH_DIR;
            continue;
        }
        else if ((dir == EXTERN && !sym->is_missing_info) || word[strlen(word) - 1] == ':') {
            handle_error(ERR_FORBIDDEN_LABEL_DECLARE, src, sym->label);
            *report = ERR_FORBIDDEN_LABEL_DECLARE;
            continue;
        }
        else if (sym->sym_dir == dir) {
            handle_error(ERR_DUPLICATE_DIR, src, sym->label, dir);
            *report = ERR_DUPLICATE_DIR;
            continue;
        }

        sym->sym_dir = dir;
        if (sym->data) sym->data->directive = dir;
    }
    FREE_AND_NULL(word);
    if (!has_extern) {
        handle_error(WARN_EMPTY_DIR, src, dir);
    }
}

/**
 * process_command - Process a command
 *
 * Processes a command by calling specific handler functions based on the type of command.
 * It handles commands with no operands, one operand, or two operands.
 *
 * @param src - Pointer to the source file context.
 * @param cmd - The command being processed.
 * @param label - The label associated with the command (optional - can be NULL).
 * @param line - The line containing the command.
 * @param report - A pointer to the status report.
 */
void process_command(file_context *src,  Command cmd, const char *label, char *line, status *report) {
    status temp_report = NO_ERROR;

    if (cmd == RTS || cmd == STOP)
        handle_no_operands(src, cmd, label, line, &temp_report);
    else if ((cmd >= INC && cmd <= JSR) || (cmd >= NOT && cmd <= CLR)) {
        handle_one_operand(src, cmd, label, line, &temp_report);
    } else if (cmd <= SUB || cmd == LEA)
        handle_two_operands(src, cmd, label, line, &temp_report);
    else {
        *report = TERMINATE;
        handle_error(TERMINATE, "process_command()");
    }
    *report = *report == ERR_MEM_ALLOC || temp_report == NO_ERROR ? *report : temp_report;

}

/**
 * handle_no_operands - Handle commands with no operands
 *
 * Handles commands that take no operands, such as `rts` and `stop`.
 * It adds data images to the source file context and updates symbol information.
 *
 * @param src - Pointer to the source file context.
 * @param cmd - The command being processed.
 * @param label - The label associated with the data (optional - can be NULL).
 * @param line - The line containing the command.
 * @param report - A pointer to the status report.
 */
void handle_no_operands(file_context *src, Command cmd, const char *label, char *line, status *report) {
    status temp_report;
    symbol *sym = NULL;
    data_image *p_data = add_data_image(src, label, report);

    if (!p_data)
        return;

    sym = find_symbol(label);

    if (get_word_length(&line)) {
        *report = ERR_EXTRA_TEXT;
        handle_error(ERR_EXTRA_TEXT, src);
    }

    temp_report = process_data_img_dec(p_data, INVALID_MD, cmd, INVALID_MD, ABSOLUTE);
    if (temp_report != NO_ERROR) {
        *report = temp_report;
        return;
    }

    if (sym) sym->data = p_data;
    p_data->is_word_complete = 1;
    IC++;
}

/**
 * handle_one_operand - Handle commands with one operand
 *
 * Handles commands that take one operand, such as `not`, `clr`, and `jmp`.
 * It processes the operand, assembles the data image, and updates symbol information.
 *
 * @param src - Pointer to the source file context.
 * @param cmd - The command being processed.
 * @param label - The label associated with the data (optional - can be NULL).
 * @param line - The line containing the command.
 * @param report - A pointer to the status report.
 */
void handle_one_operand(file_context *src, Command cmd, const char *label, char *line, status *report) {
    char *word = NULL;
    data_image *p_data_word = NULL;
    data_image *p_data_op = NULL;
    Adrs_mod op_mode;
    status temp_report;
    size_t word_len;
    Concat_mode concat;

    word = malloc(sizeof(char) * get_word_length(&line) + 1);
    word_len = get_word(&line, word, COMMA);

    if (!word_len || !word) {
        *report = *report == ERR_MEM_ALLOC ? ERR_MEM_ALLOC : ERR_MISS_OPERAND;
        handle_error(ERR_MISS_OPERAND, src);
        if (word) free(word);
        return;
    } else if (get_word_length(&line)) {
        *report = (word[word_len - 1] == ',') ? ERR_TOO_MANY_OPERANDS : ERR_EXTRA_TEXT;
        handle_error(*report, src);
    } if (word_len > MAX_LABEL_LENGTH) {
        *report = ERR_OPERAND_TOO_LONG;
        handle_error(ERR_OPERAND_TOO_LONG);
    } if (word[word_len - 1] == ',') {
        word[word_len - 1] = '\0';
        word_len--;
        *report = ERR_EXTRA_COMMA;
        handle_error(ERR_EXTRA_COMMA, src);
    }

    op_mode = get_addressing_mode(src, word, word_len, report);
    if (!is_legal_addressing(src, cmd, INVALID_MD, op_mode, report) ||
        (concat = get_concat_mode_one_op(INVALID_MD, op_mode)) == -1) {
        free(word);
        return;
    }

    p_data_word = add_data_image(src, label, report);
    p_data_op = assemble_operand_data_img(src, concat, op_mode, word);
    temp_report = process_data_img_dec(p_data_word, INVALID_MD, cmd, op_mode, ABSOLUTE);

    if (!p_data_word || !p_data_op || temp_report != NO_ERROR ) {
        free(word);
        *report = ERR_MEM_ALLOC;
        return;
    }

    free(word);
    p_data_word->is_word_complete = 1;
    IC += 2;
}

/**
 * handle_two_operands - Handle commands with two operands
 *
 * Handles commands that take two operands, such as `mov`, `add`, and `sub`.
 * It assembles the data image for both operands and performs necessary error handling.
 *
 * @param src - Pointer to the source file context.
 * @param cmd - The command being processed.
 * @param label - The label associated with the data (optional - can be NULL).
 * @param line - The line containing the command.
 * @param report - A pointer to the status report.
 */
void handle_two_operands(file_context *src, Command cmd, const char *label, char *line, status *report) {
    char *word = NULL;
    char *next_word = NULL;
    data_image *p_data_word = NULL;
    data_image *p_data_op = NULL;
    data_image *p_data_sec_op = NULL;
    Adrs_mod op_mode, sec_op_mode;
    status temp_report = NO_ERROR, secondary_temp;
    Concat_mode concat_1 = ILLEGAL_CONCAT, concat_2 = ILLEGAL_CONCAT;
    size_t word_len, word_len_sec;

    word_len =  is_valid_string(&line, &word, report);
    (void) line_parser(src, DEFAULT, &line, &word, &temp_report);
    word_len_sec =  is_valid_string(&line, &next_word, report);
    (void) line_parser(src, DEFAULT, &line, &next_word, &temp_report);

    if (!word || !next_word) {
        *report = *report == ERR_MEM_ALLOC ? ERR_MEM_ALLOC : ERR_MISS_OPERAND;
        handle_error(ERR_MISS_OPERAND, src);
        if (word) free(word);
        if (next_word) free(next_word);
        return;
    } else if (get_word_length(&line)) {
        *report = ERR_EXTRA_TEXT;
        handle_error(ERR_EXTRA_TEXT, src);
    } if (word_len > MAX_LABEL_LENGTH || word_len_sec > MAX_LABEL_LENGTH) {
        *report = ERR_OPERAND_TOO_LONG;
        handle_error(ERR_OPERAND_TOO_LONG);
    }

    op_mode = get_addressing_mode(src, word, word_len, report);
    sec_op_mode = get_addressing_mode(src, next_word, word_len_sec, report);

    if (!is_legal_addressing(src, cmd, op_mode, sec_op_mode, report) ||
        get_concat_mode(op_mode, sec_op_mode, &concat_1, &concat_2) != NO_ERROR) {
        if (word) free(word);
        if (next_word) free(next_word);
        return;
    }

    p_data_word = add_data_image(src, label, report);
    if (p_data_word) {
        if (concat_1 == concat_2 && concat_1 == REG_REG)
            p_data_op = assemble_operand_data_img(src, concat_1, op_mode, word, next_word);
        else {
            p_data_op = assemble_operand_data_img(src, concat_1, op_mode, word);
            p_data_sec_op = assemble_operand_data_img(src, concat_2, sec_op_mode, next_word);
        }
        secondary_temp = process_data_img_dec(p_data_word, op_mode, cmd, sec_op_mode, ABSOLUTE);
    }

    if (!p_data_word || (!p_data_op || (concat_1 != REG_REG && !p_data_sec_op)) ||
        secondary_temp != NO_ERROR || temp_report != NO_ERROR) {
        if (word) free(word);
        if (next_word) free(next_word);
        *report = ERR_MEM_ALLOC;
        return;
    }

    p_data_word->is_word_complete = 1;
    if (word) free(word);
    if (next_word) free(next_word);

    if (concat_1 == REG_REG) {
        p_data_op->is_word_complete = 1;
        IC += 2;
        return;
    }
    if (op_mode == IMMEDIATE || op_mode == REGISTER) p_data_op-> is_word_complete = 1;
    if (sec_op_mode == IMMEDIATE || sec_op_mode == REGISTER) p_data_sec_op-> is_word_complete = 1;

    IC += 3; /* Instruction + 2 operands words */
}

/**
 * Parses a line and extracts a word based on the specified delimiter.
 * It also handles the removal of commas and checks for extra text or missing comma errors.
 *
 * @param src The pointer to the source file context.
 * @param dir The directive being processed.
 * @param line A pointer to the line string.
 * @param word A pointer to store the extracted word.
 * @param report A pointer to the status report.
 * @return The value indicating the type of the parsed word (LBL, INV, etc.).
 */
Value line_parser(file_context *src, Directive dir, char **line, char **word, status *report) {
    size_t length;
    char *p_line = *line;
    Value ret_val;

    if (!(*word)) {
        if (dir != DEFAULT) /* in case of 'DEFAULT' we handle missing operands within "handle_two_operands()" */
            handle_error(TERMINATE, "line_parser()");
        return INV;
    }

    while (**word && isspace(**word))
        (*word)++;

    length = strlen(*word);

    while (**word == ',' && length >= 1) {
        (*word)++;
        length--;
        *report = ERR_EXTRA_COMMA;
        handle_error(ERR_EXTRA_COMMA, src);
    }

    if (!length) {
        *report = ERR_INVALID_SYNTAX;
        return INV;
    }


    if ((*word)[length - 1] != ',') {
        while (**line && isspace(**line))
            (*line)++;
        if (**line != ',' && **line != '\0' && **word != '\"') {
            *report = ERR_MISSING_COMMA;
            handle_error(ERR_MISSING_COMMA, src);
        }
        else if (**line == ',')
            (*line)++;
    }
    else {
        if (**line == '\0' || **line == '\n') {
            *report = ERR_EXTRA_COMMA;
            handle_error(ERR_EXTRA_COMMA, src);
        }
        (*word)[length - 1] = '\0';
        length--;
    }

    if (dir == DATA)
        return validate_data(src, *word, length, report);
    else if (dir == STRING && (*line = p_line)) {
        ret_val = validate_string(line , word, length, report);
        while (**line && isspace(**line)) (*line)++;
        p_line = *line;
        while (**line && isspace(**line)) (*line)++;
        if (*p_line != ',' && (**line != '\0' && **line != '\n')) {
            *report = ERR_MISSING_COMMA;
            handle_error(ERR_MISSING_COMMA, src);
        }
        return ret_val;
    }
    else
        return LBL;
}

/**
 * Parses a string and extracts characters one by one.
 * It handles the opening and closing quotation marks of the string and reports any errors.
 *
 * @param src The pointer to the source file context.
 * @param word A pointer to the string being parsed.
 * @param ch A pointer to store the extracted character.
 * @param report A pointer to the status report.
 * @return The status of the parsing operation.
 *         Returns TERMINATE if the end of the string is reached,
 *         or NO_ERROR if the parsing is successful.
 */
status string_parser(file_context *src, char **word, char *ch, status *report) {
    static int is_first_qmark = 0;
    static status reached_end = 0;
    status ret_val = NO_ERROR;

    if (!src && !word && !ch && !report) { /* resetting static variable */
        is_first_qmark = 0;
        reached_end = 0;
        return NO_ERROR;
    }

    if (reached_end) {
        reached_end = 0;
        return TERMINATE;
    }

    if (**word == '\"') {
        if (!is_first_qmark) {
            is_first_qmark = 1;
            (*word)++;
        }
        else {
            is_first_qmark = 0;
            (*word)++;
            *ch = '\0';
            *report = (**word != '\0' && **word != '\n') ? ERR_EXTRA_TEXT : *report;
            if (*report == ERR_EXTRA_TEXT) handle_error(ERR_EXTRA_TEXT, src);
            ret_val = NO_ERROR;
            reached_end = 1;
        }
    }
    else {
        if ((*word)[1] == '\0' || (*word)[1] == '\n') {
            is_first_qmark = 0;
            reached_end = 1;
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
 * Updates the information of an existing symbol with a new data_address.
 *
 * @param sym The existing symbol to update.
 * @param address The new data_address to assign to the symbol.
 * @return The status of the update operation.
 *         Returns NO_ERROR if the update was successful,
 *         or FAILURE if the conversion to binary failed.
 */
status update_symbol_info(symbol *sym, int address) {
    sym->address_decimal = address;
    sym->address_binary = decimal_to_binary12(address);

    if (!sym->address_binary)
        return FAILURE;

    sym->is_missing_info = 0;
    return NO_ERROR;
}

/**
 * Add a symbol to the symbol table.
 * If the symbol already exists and has missing information, its information is updated with the provided data_address.
 * If the symbol already exists and has complete information, NULL is returned.
 * If the symbol is new, it is added to the symbol table with the provided label and data_address.
 *
 * @param src A pointer to a file_context.
 * @param label The label of the symbol to add.
 * @param address The data_address of the symbol.
 * @param report A pointer to a status report variable.
 * @return A pointer to the added symbol if it is new or has missing information, or NULL if the symbol already exists with complete information.
 *         Returns NULL in case of memory allocation errors during symbol creation or table expansion.
 */
symbol *add_symbol(file_context *src, const char *label, int address, status *report) {
    symbol **new_symbol_table = NULL;
    symbol *new_symbol = NULL;
    symbol *existing_symbol = NULL;
    status temp_report;
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
        temp_report = is_valid_label(label);
        if (temp_report != NO_ERROR && temp_report != ERR_MISSING_COLON) {
            handle_error(ERR_INVALID_LABEL, src, label);
            *report = temp_report;
            return NULL;
        }
        new_symbol_table = realloc(symbol_table, (symbol_count + 1) * sizeof(symbol*));
        new_symbol = malloc(sizeof(symbol));

        if (new_symbol) new_symbol->label = new_symbol->address_binary = NULL;
        if (!label || !new_symbol_table || !new_symbol || copy_string(&(new_symbol->label), label) != NO_ERROR)    {
            handle_error(ERR_MEM_ALLOC);
            if (new_symbol) free_symbol(&new_symbol);
            symbol_table = new_symbol_table ? new_symbol_table : symbol_table;
            *report = ERR_MEM_ALLOC;
            return NULL;
        }
        if (address == INVALID_ADDRESS) {
            new_symbol->is_missing_info = 1;
            new_symbol->address_decimal = address;
            new_symbol->address_binary = NULL;
        }
        else {
            new_symbol->address_decimal = address;
            new_symbol->address_binary = decimal_to_binary12(address);
            new_symbol->is_missing_info = 0;
        }

        new_symbol->lc = src->lc;
        new_symbol->sym_dir = DEFAULT;
        new_symbol->data = NULL;
        symbol_table = new_symbol_table;
        symbol_table[symbol_count++] = new_symbol;

        return new_symbol;
    }
}

/**
 * Adds a data image entry to the data image array.
 *
 * Adds a new data image entry to the data image array based on the provided
 * source and label information. Updates the report status accordingly.
 *
 * @param src The source file_context pointer.
 * @param label The label associated with the data image entry (optional).
 * @param report Pointer to the status report variable.
 * @return Pointer to the newly added data image entry, or NULL if an error occurred.
 */
data_image* add_data_image(file_context *src, const char* label, status *report) {
    static size_t data_obj_cap = 0;
    size_t new_cap = data_obj_cap + DEFAULT_DATA_IMAGE_CAP;
    data_image **data_arr = NULL;
    data_image *new_image = NULL;

    if (!src && !label && !report) { /* resetting static variable */
        data_obj_cap = 0;
        return NULL;
    }

    new_image = create_data_image(src->lc, &next_free_address);

    if (next_free_address == MAX_MEMORY_SIZE) {
        handle_error(TERMINATE, "The input file requires too much memory.");
        if (new_image) free_data_image(&new_image);
        *report = TERMINATE;
        return NULL;
    }

    if (!new_image || (data_obj_cap <= data_arr_obj_index &&
                       !(data_arr = realloc(data_img_obj, (new_cap) * sizeof(data_image *))))) {
        if (new_image) free_data_image(&new_image);
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
        next_free_address--; /* updated within create_data_image() */
        new_image->data_address = next_free_address;
    }

    if (data_arr) {
        data_img_obj = data_arr;
        data_obj_cap = new_cap;
    }
    data_img_obj[data_arr_obj_index++] = new_image;

    return new_image;
}

/**
 * Declares a label symbol and adds it to the symbol table.
 *
 * Declares a label symbol based on the provided source, label, and label length.
 * Updates the report status accordingly.
 *
 * @param src The source file_context pointer.
 * @param label The label to declare.
 * @param label_len The length of the label.
 * @param report Pointer to the status report variable.
 * @return Pointer to the declared label symbol, or NULL if an error occurred.
 */
symbol* declare_label(file_context *src, char *label, size_t label_len, status *report) {
    symbol *sym = NULL;
    if (!label_len) {
        handle_error(TERMINATE, "declare_label()");
        return NULL;
    }

    if (is_valid_label(label) == ERR_INVALID_LABEL)
        return NULL;

    if (label[label_len - 1] == ':')
        label[label_len - 1] = '\0';
    else {
        *report = ERR_MISSING_COLON;
        handle_error(ERR_MISSING_COLON, src);
    }

    sym = add_symbol(src, label, next_free_address, report);
    next_free_address = sym ? next_free_address + 1 : next_free_address;

    return sym;
}

/**
 * Asserts the data image entry by label and handles associated resources.
 *
 * Asserts the data image entry based on the provided label and updates the flag,
 * value, data image, and report status accordingly.
 *
 * @param src The source file_context pointer.
 * @param label The label associated with the data image entry (optional).
 * @param flag Pointer to the flag variable.
 * @param value Pointer to the value variable.
 * @param p_data Pointer to the data image pointer.
 * @param report Pointer to the status report variable.
 */
void assert_data_img_by_label(file_context *src, const char *label, int *flag,
                              int **value, data_image **p_data, status *report) {
    if (label && !*flag) {
        *flag = 1;
        if (!(*p_data = add_data_image(src, label, report)) && (*report == ERR_MEM_ALLOC || *report == TERMINATE))
            return;
    }
    else {/* A label can only be associated with the initial value */
        if (!(*p_data = add_data_image(src, NULL, report)) && (*report == ERR_MEM_ALLOC || *report == TERMINATE))
            return;
    }

    *value = malloc(sizeof (int));
    if (!*p_data || !*value) {
        handle_error(TERMINATE, "process_data");
        *report = TERMINATE;
        return;
    }
}

/**
 * Asserts a value to the data image and handles associated resources.
 *
 * Asserts a value to the data image based on the provided directive, value type,
 * word, and updates the value, data image, and report status accordingly.
 *
 * @param src The source file_context pointer.
 * @param dir The directive type.
 * @param val_type The value type.
 * @param word The word value.
 * @param value Pointer to the value variable.
 * @param p_data Pointer to the data image pointer.
 * @param report Pointer to the status report variable.
 * @return The status of the assertion: NO_ERROR on success, FAILURE or TERMINATE if an error occurred.
 */
status assert_value_to_data(file_context *src, Directive dir, Value val_type ,
                            char *word, int **value, data_image **p_data, status *report) {
    symbol *sym = NULL;
    status temp_report = is_valid_label(word);

    if (dir == DATA && val_type == NUM)
        **value = safe_atoi(word);
    else if (dir == STRING && val_type == STR)
        **value = (int)*word;
    else if (temp_report == ERR_MISSING_COLON && val_type == LBL) { /* A label (usage) within statement */
        sym = add_symbol(src, word, INVALID_ADDRESS, report);
        if (sym && sym->data && sym->data->value)
            *value = sym->data->value;
        else if (sym) {
            (*p_data)->p_sym = sym;
            free(*value);
            *value = NULL;
            return NO_ERROR;
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
        *report = ERR_INVALID_SYNTAX;
        free_data_image(&data_img_obj[--data_arr_obj_index]);
        return FAILURE;
    }
    return NO_ERROR;
}

/**
 * Generates the output file for the object code.
 *
 * Generates the (.obj/ .ext/ .ent) output file based on the data image and symbol table,
 * and by the output file extension determined by dir.
 *
 * @param src The source file_context pointer.
 * @param dir The directive type.
 * @return The status of the operation: NO_ERROR on success, FAILURE or TERMINATE if an error occurred.
 */
status generate_output_by_dest(file_context *src, Directive dir) {
    file_context *dest = NULL;
    status report = NO_ERROR;

    typedef status (*write_func)(file_context *src, FILE *dest);
    write_func p_write_func = NULL;

    char *file_name= src->file_name_wout_ext;

    if (dir == DEFAULT) {
        dest = create_file_context(file_name, OBJECT_EXT, FILE_EXT_LEN, FILE_MODE_WRITE_PLUS, &report);
        p_write_func = write_data_img_to_stream;
    } else if (dir == EXTERN) {
        dest = create_file_context(file_name, EXTERNAL_EXT, FILE_EXT_LEN_OUT, FILE_MODE_WRITE_PLUS, &report);
        p_write_func = write_extern_to_stream;
    } else if (dir == ENTRY) {
        dest = create_file_context(file_name, ENTRY_EXT, FILE_EXT_LEN_OUT, FILE_MODE_WRITE_PLUS, &report);
        p_write_func = write_entry_to_stream;
    }
    else {
        handle_error(TERMINATE, generate_output_by_dest);
        return TERMINATE;
    }

    if (dest && p_write_func)
        report = p_write_func(src, dest->file_ptr);
    else
        return TERMINATE;

    if (report != NO_ERROR) {
        fclose(dest->file_ptr);
        dest->file_ptr = NULL;
        remove(dest->file_name);
    }

    free_file_context(&dest);
    return report;
}

/**
 * Writes the data image to the specified output stream.
 *
 * Writes the data image entries to the specified output stream. Includes the
 * instruction count (IC) and data count (DC) in the output.
 *
 * @param src The source file_context pointer.
 * @param dest The output stream to write the data image to.
 * @return The status of the output generation: NO_ERROR on success, TERMINATE if no data
 *         image is available or an error occurred.
 */
status write_data_img_to_stream(file_context *src, FILE *dest) {
    size_t i;
    int error_flag = 0;
    data_image *runner = NULL;

    if (!data_arr_obj_index) return FAILURE; /* not an actual error, just no output file has been created */

    for (i = 0; i < data_arr_obj_index; i++) {
        runner = data_img_obj[i];
        if (!runner->value && runner->p_sym && runner->concat == VALUE) {
            if (runner->p_sym->data) {
                runner->value = malloc(sizeof(int));
                if (runner->value != NULL)
                    *(runner->value) = *(runner->p_sym->data->value);
            }
            else {
                error_flag = 1;
                handle_error(ERR_LABEL_DOES_NOT_EXIST, src, runner->p_sym->label, runner->lc);
            }
        }
        else if (!runner->is_word_complete && runner->concat == ADDRESS
                 && handle_address_reference(runner, runner->p_sym) != NO_ERROR) {
            error_flag = 1;
            handle_error(ERR_LABEL_DOES_NOT_EXIST, src, runner->p_sym->label, runner->lc);
        }
    }

    fprintf(dest, "%lu %lu", (unsigned long)IC, (unsigned long)DC);

    for (i = 0; i < data_arr_obj_index && !error_flag; i++) {
        if (!data_img_obj[i]->is_word_complete)
            create_base64_word(data_img_obj[i]);
        if (!data_img_obj[i]->base64_word) {
            error_flag = 1;
            handle_error(TERMINATE, "write_data_img_to_stream()");
            break;
        }
        fprintf(dest, "\n%s", data_img_obj[i]->base64_word);
        free(data_img_obj[i]->base64_word);
        data_img_obj[i]->base64_word = NULL;
    }
    return error_flag ? TERMINATE : NO_ERROR;
}

/**
 * Writes the entry symbols and their addresses to the specified output stream.
 *
 * Writes the entry symbols and their corresponding addresses to the specified
 * output stream. Only generates output if there are existing entry symbols.
 *
 * @param src The source file_context pointer.
 * @param dest The output stream to write the entry information to.
 * @return The status of the output generation: NO_ERROR if output was generated
 *         and no errors were encountered, TERMINATE if no output was generated
 *         (no entry symbols), or FAILURE in case of an error.
 *
 */
status write_entry_to_stream(file_context *src, FILE *dest) {
    int i;
    int error_flag = 0;
    int has_entry = 0;
    symbol *runner = NULL;

    if (!data_arr_obj_index) return FAILURE;

    for (i = 0; i < symbol_count; i++) {
        runner = symbol_table[i];
        if (runner && runner->sym_dir == ENTRY) {
            has_entry = 1;
            if (runner->is_missing_info) {
                error_flag = 1;
                handle_error(ERR_LABEL_DOES_NOT_EXIST, src, runner->label, runner->lc);
                continue;
            }
            if (!error_flag)
                fprintf(dest, "%s\t%d\n", runner->label, runner->address_decimal);
        }
    }
    return error_flag ? FAILURE : !has_entry ? TERMINATE : NO_ERROR;
}

/**
 * Writes the extern symbols and their addresses to the specified output stream.
 *
 * Writes the extern symbols and their corresponding addresses to the specified
 * output stream. Generates output only if there are existing extern symbols.
 *
 * @param src The source file_context pointer.
 * @param dest The output stream to write the extern information to.
 * @return The status of the output generation: NO_ERROR if output was generated
 *         and no errors were encountered, TERMINATE if no output was generated
 *         (no extern symbols), or FAILURE in case of an error.
 */
status write_extern_to_stream(file_context *src, FILE *dest) {
    int i;
    int error_flag = 0;
    int has_extern = 0;
    data_image *runner = NULL;
    symbol *sym = NULL;

    if (!data_arr_obj_index) return FAILURE;

    for (i = 0; i < data_arr_obj_index; i++) {
        runner = data_img_obj[i];
        if (runner && runner->p_sym && runner->p_sym->sym_dir == EXTERN) {
            has_extern = 1;
            if (runner->p_sym->address_binary) free (runner->p_sym->address_binary);

            runner->p_sym->address_binary = decimal_to_binary12(EXTERNAL);
            if (!runner->p_sym->address_binary || !(runner->value = malloc(sizeof (int)))) {
                handle_error(ERR_MEM_ALLOC);
                return ERR_MEM_ALLOC;
            }

            runner->p_sym->is_missing_info = 0;
            runner->p_sym->address_decimal = 0;
            *(runner->value) = 0;
            if (create_base64_word(runner) == NO_ERROR && (runner->is_word_complete = 1))
                fprintf(dest, "%s\t%d\n", runner->p_sym->label, runner->data_address);
            else {
                handle_error(TERMINATE, "write_extern_to_stream()");
                error_flag = 1;
            }
        }
    }
    for (i = 0; i < symbol_count && !error_flag; i++) {
        sym = symbol_table[i];
        if (sym->sym_dir == EXTERN && sym->is_missing_info)
            handle_error(WARN_UNUSED_EXT, src, sym->label, sym->lc);
    }
    return  !has_extern ? TERMINATE : NO_ERROR;
}

/**
 * Frees the global data image arrays and symbol table.
 */
void free_global_data_and_symbol() {
    free_data_image_array(&data_img_obj, &data_arr_obj_index);
    free_symbol_table(&symbol_table, &symbol_count);
    DC = IC = 0;
    next_free_address = ADDRESS_START;
    (void) add_data_image(NULL, NULL, NULL); /* resetting static variables */
    (void) string_parser(NULL, NULL, NULL, NULL);
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
    remove(p_src->file_name);
    p_src->file_ptr = NULL;
    free_file_context(&p_src);
    *src = NULL;
    free_global_data_and_symbol();
}