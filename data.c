/* data.h
 * data image and base conversions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <ctype.h>
#include "data.h"
#include "passes.h"
#include "errors.h"
#include "utils.h"

#define get_register_num(reg) ((char) (reg)[2] - '0')

/* "Private" helper functions */
status concat_default_12bit(data_image *data, char** binary_word);
status concat_reg_dest(data_image *data, char** binary_word);
status concat_reg_src(data_image *data, char** binary_word);
status concat_reg_reg(data_image *data, char** binary_word);
status concat_address(data_image *data, char** binary_word);

/**
 * Converts a decimal number to a binary string representation.
 *
 * @param decimal The decimal number to be converted to binary.
 * @return A dynamically allocated binary string representation of the decimal number,
 * or NULL if memory allocation fails.
 */
char* decimal_to_binary12(int decimal) {
    int i, carry;
    int is_negative = 0;
    char *binary = malloc((BINARY_BITS + 1) * sizeof(char));
    static const char *lookup_bin[COMMANDS_LEN] = {
            "000000000000",
            "000000000001",
            "000000000010",
            "000000000011",
            "000000000100",
            "000000000101",
            "000000000110",
            "000000000111",
            "000000001000",
            "000000001001",
            "000000001010",
            "000000001011",
            "000000001100",
            "000000001101",
            "000000001110",
            "000000001111"
    };

    if (!binary) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    if (decimal >= 0 && decimal <= 15) {
        strcpy(binary, lookup_bin[decimal]);
        return binary;
    }

    if (decimal < 0) {
        decimal = -decimal;
        is_negative = 1;
    }

    for (i = BINARY_BITS - 1; i >= 0; i--) {
        binary[i] = (char)((decimal % 2) + '0');
        decimal /= 2;
    }

    if (is_negative) {
        /* Two's complement for is_negative numbers */
        for (i = 0; i < BINARY_BITS; i++)
            binary[i] = (binary[i] == '0') ? '1' : '0';

        carry = 1;
        for (i = BINARY_BITS - 1; i >= 0; i--) {
            int sum = (binary[i] - '0') + carry;
            binary[i] = (char)((sum % 2) + '0');
            carry = sum / 2;
        }
    }

    binary[BINARY_BITS] = '\0';
    return binary;
}

/**
 * Truncates an input string to the specified length.
 *
 * @param input  The input string to be truncated.
 * @param length The desired length of the truncated string.
 * @return A dynamically allocated truncated string,
 * or NULL if an error occurs or `length` is greater than the input string length.
 */
char* truncate_string(const char *input, int length) {
    unsigned int truncate_len;
    char* truncated = NULL;

    if (!input) {
        if (!(truncated = calloc(length + 1, sizeof(char ))))
            handle_error(ERR_MEM_ALLOC);
        return truncated;
    }

    truncate_len = BINARY_BITS - length;

    if (length >= BINARY_BITS) {
        handle_error(TERMINATE, "truncate_string()");
        return NULL;
    }

    truncated = malloc((length + 1) * sizeof(char));
    if (!truncated) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    strncpy(truncated, input + truncate_len, length);
    truncated[length] = '\0';

    return truncated;
}

/**
 * Converts a 12-bit binary string to a Base64 string.
 *
 * @param binary The input binary string to be converted.
 * @return A Base64 string representation of the input.
 *         Returns NULL if an error occurs.
 */
char* binary12_to_base64(const char *binary) {
    int i = 0;
    int j = 0;
    unsigned char value = 0;
    static const char* lookup_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* base64 = malloc(BASE64_CHARS + 1);

    if (!base64) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    } else if (!binary)
        return NULL;

    if (binary[BINARY_BITS] != '\0') {
        handle_error(TERMINATE, "binary12_to_base64()");
        return NULL;
    }
    while (binary[i] != '\0') {
        value = (value << 1) | (binary[i] - '0');
        i++;

        if (i % BINARY_BASE64_BITS == 0) {
            base64[j] = lookup_table[value];
            j++;
            value = 0;
        }
    }

    base64[j] = '\0';
    return base64;
}

/**
 * Processes the decimal values for a data image, setting the binary representations
 * of the source operand, opcode, destination operand, and A/R/E bits.
 *
 * @param data The data image structure to be processed.
 * @param src_op The addressing mode of the source operand.
 * @param opcode The opcode of the command.
 * @param dest_op The addressing mode of the destination operand.
 * @param are The A/R/E (Absolute/Relocation/External) bits.
 * @return The status of the processing operation. Possible return values are:
 *         - NO_ERROR: If the processing is successful.
 *         - ERR_MEM_ALLOC: If there is a memory allocation error.
 */
status process_data_img_dec(data_image *data, Adrs_mod src_op, Command opcode, Adrs_mod dest_op, ARE are) {
    int has_alloc_err;

    data->binary_src = decimal_to_binary12(src_op);
    data->binary_opcode = decimal_to_binary12(opcode);
    data->binary_dest = decimal_to_binary12(dest_op);
    data->binary_a_r_e = decimal_to_binary12(are);

    has_alloc_err = !data->binary_src || !data->binary_opcode || !data->binary_dest || !data->binary_a_r_e;

    if (has_alloc_err || create_base64_word(data) != NO_ERROR) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;;
    }
    return NO_ERROR;
}

/**
 * Assembles an operand into a data_image structure based on the addressing mode and concatenation mode.
 *
 * @param src The source file context.
 * @param con_md The concatenation mode.
 * @param mode The addressing mode.
 * @param word The operand word to assemble.
 * @param ... Additional operands (only used for REG_REG concatenation mode).
 * @return A pointer to the assembled data_image structure, or NULL if an error occurs.
 */
data_image *assemble_operand_data_img(file_context *src, Concat_mode con_md, Adrs_mod mode, char* word, ...) {
    va_list args;
    symbol *sym = NULL;
    status temp_report;
    data_image *data = add_data_image(src, NULL, &temp_report);

    if (!data || !(data->concat = con_md)) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    } else if (mode == DIRECT) {
        sym = add_symbol(src, word, INVALID_ADDRESS, &temp_report);
        temp_report = sym ? handle_address_reference(data, sym) : TERMINATE;
    } else if (mode == IMMEDIATE) {
        data->binary_src = decimal_to_binary12(safe_atoi(word));
        data->binary_a_r_e = decimal_to_binary12(ABSOLUTE);
        data->concat = ADDRESS; /* Adding the A/R/E bits */
        temp_report = create_base64_word(data) == NO_ERROR && (data->is_word_complete = 1) ? NO_ERROR : FAILURE;
    } else if (con_md == REG_SRC || con_md == REG_DEST) {
        temp_report = handle_register_data_img(data, con_md, word);
    } else if (con_md == REG_REG) {
        va_start(args, word);
        temp_report = handle_register_data_img(data, con_md, word, va_arg(args, char*));
        va_end(args);
    } else {
        handle_error(TERMINATE, "assemble_operand_data_img()");
        temp_report = TERMINATE;
    }
    return temp_report == NO_ERROR ? data : NULL;
}

/**
 * Handles the assembly of register operands into a data_image structure.
 *
 * @param data The data_image structure to assemble the register operands into.
 * @param con_act The concatenation action (REG_SRC, REG_DEST, or REG_REG).
 * @param reg The register operand.
 * @param ... Additional register operands (only used for REG_REG concatenation action).
 * @return The status of the assembly process. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status handle_register_data_img(data_image *data, Concat_mode con_act, char *reg, ...) {
    va_list args;
    char *sec_reg = NULL;

    if (!data || !reg || (con_act != REG_SRC && con_act != REG_DEST && con_act != REG_REG)) {
        handle_error(TERMINATE, "handle_address_reference()");
        return TERMINATE;
    }
    else if (con_act == REG_SRC)
        data->binary_src = decimal_to_binary12(get_register_num(reg));
    else if (con_act == REG_DEST)
        data->binary_dest = decimal_to_binary12(get_register_num(reg));
    else {/* (con_act == REG_REG) */
        va_start(args, reg);
        sec_reg = va_arg(args, char*);
        data->binary_src = decimal_to_binary12(get_register_num(reg));
        data->binary_dest = decimal_to_binary12(get_register_num(sec_reg));
        va_end(args);
    }
    return create_base64_word(data) == NO_ERROR && (data->is_word_complete = 1) ? NO_ERROR : FAILURE;
}

/**
 * Handles the assembly of address reference operands into a data_image structure.
 *
 * @param data The data_image structure to assemble the address reference operand into.
 * @param sym The symbol representing the address reference.
 * @return The status of the assembly process. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status handle_address_reference(data_image *data,  symbol *sym) {
    if (!data || !sym) {
        handle_error(TERMINATE, "handle_address_reference()");
        return TERMINATE;
    }
    else if (sym->address_binary && (data->binary_src = sym->address_binary) &&
             (data->binary_a_r_e = decimal_to_binary12(get_are(sym)))) {
        data->concat = ADDRESS;
        data->is_word_complete = 1;
        return create_base64_word(data);
    }
    data->p_sym = sym;
    return NO_ERROR;
}

/**
 * Determines the concatenation mode based on the addressing modes of the source and destination operands.
 *
 * @param src_op The addressing mode of the source operand.
 * @param dest_op The addressing mode of the destination operand.
 * @param cn1 Pointer to store the first concatenation mode.
 * @param cn2 Pointer to store the second concatenation mode.
 * @return The status of the concatenation mode determination. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status get_concat_mode(Adrs_mod src_op, Adrs_mod dest_op, Concat_mode *cn1, Concat_mode *cn2) {
    if (src_op && dest_op) {
        if (src_op == REGISTER && dest_op == REGISTER)
            *cn1 = *cn2 = REG_REG;
        else {
            *cn1 = get_concat_mode_one_op(src_op, INVALID_MD);
            *cn2 = get_concat_mode_one_op(INVALID_MD, dest_op);
        }
    }
    return (*cn1 == -1 || *cn2 == -1) ? FAILURE : NO_ERROR;
}

/**
 * Determines the concatenation mode based on a single addressing mode.
 *
 * @param src_op The addressing mode.
 * @param dest_op The addressing mode.
 * @return The determined concatenation mode.
 */
Concat_mode get_concat_mode_one_op(Adrs_mod src_op, Adrs_mod dest_op) {
    if ((src_op == IMMEDIATE && dest_op == INVALID_MD) || (src_op == INVALID_MD && dest_op == IMMEDIATE))
        return VALUE;
    else if ((src_op == DIRECT && dest_op == INVALID_MD) || (src_op == INVALID_MD && dest_op == DIRECT))
        return ADDRESS;
    else if (src_op == REGISTER && dest_op == INVALID_MD)
        return REG_SRC;
    else if (src_op == INVALID_MD && dest_op == REGISTER)
        return REG_DEST;
    else {
        handle_error(TERMINATE, "get_concat_mode_one_op()");
        return ILLEGAL_CONCAT;
    }
}

/**
 * Creates the base64 word for a data_image structure by concatenating the binary components
 * and converting the binary word to base64 format.
 *
 * @param data The data_image structure containing the binary components.
 * @return The status of the base64 word creation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status create_base64_word(data_image *data) {
    status report = NO_ERROR;
    char *binary_word = malloc((BINARY_BITS + 1) * sizeof(char));

    if (!binary_word) {
        handle_error(ERR_MEM_ALLOC);
        return ERR_MEM_ALLOC;
    }

    if (data->base64_word) {
        free(data->base64_word);
        data->base64_word = NULL;
    }

    if (!data) {
        handle_error(TERMINATE, "create_base64_word()");
        free(binary_word);
        return ERR_MEM_ALLOC;
    }

    if (data->concat == DEFAULT_12BIT)
        report = concat_default_12bit(data, &binary_word);
    else if (data->concat == REG_DEST)
        report = concat_reg_dest(data, &binary_word);
    else if (data->concat == REG_SRC)
        report = concat_reg_src(data, &binary_word);
    else if (data->concat == REG_REG)
        report = concat_reg_reg(data, &binary_word);
    else if (data->concat == ADDRESS)
        report = concat_address(data, &binary_word);
    else if (data->concat == VALUE) {
        free (binary_word);
        report = (binary_word = decimal_to_binary12(*(data->value))) ? NO_ERROR : FAILURE;
    }

    else
        handle_error(TERMINATE, "concatenate_and_convert_to_base64()");

    if (report != FAILURE) { /* Convert binary to base64 */
        data->base64_word = binary12_to_base64(binary_word);
        if (!data->base64_word)
            report = FAILURE; /* Error message printed via binary12_to_base64() */
        data->is_word_complete = 1;
    }

    free(binary_word);
    binary_word = NULL;
    return report;
}

/**
 * Creates and initializes a new data image with the specified location counter (lc).
 *
 * @param lc The location counter value for the data image.
 * @return A pointer to the newly created data image, or NULL if memory allocation fails.
 */
data_image* create_data_image(int lc, int *address) {
    data_image* p_ret = malloc(sizeof(data_image));
    if (!p_ret) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    p_ret->binary_src = NULL;
    p_ret->binary_opcode = NULL;
    p_ret->binary_dest = NULL;
    p_ret->binary_a_r_e = NULL;
    p_ret->base64_word = NULL;

    p_ret->directive = DATA;
    p_ret->concat = DEFAULT_12BIT;
    p_ret->p_sym = NULL;
    p_ret->value = NULL;

    p_ret->is_word_complete = 0;
    p_ret->lc = lc;
    p_ret->data_address = (*address)++;

    return p_ret;
}

/**
 * Gets the A/R/E (Absolute/Relocation/External) bits for a given symbol.
 *
 * @param sym The symbol to retrieve the A/R/E bits from.
 * @return The A/R/E bits of the symbol.
 */
ARE get_are(symbol *sym) {
    return sym->sym_dir == EXTERN ? EXTERNAL : RELOCATABLE;
}

/**
 * Concatenates the binary components of a data_image structure according to the DEFAULT_12BIT concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a char array where the concatenated binary word will be stored.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_default_12bit(data_image *data, char **binary_word) {
    char *src_op = NULL, *opcode = NULL, *dest_op = NULL, *a_r_e = NULL;
    status report = NO_ERROR;

    src_op = truncate_string(data->binary_src, SRC_DEST_OP_BINARY_LEN);
    opcode = truncate_string(data->binary_opcode, OPCODE_BINARY_LEN);
    dest_op = truncate_string(data->binary_dest, SRC_DEST_OP_BINARY_LEN);
    a_r_e = truncate_string(data->binary_a_r_e, A_R_E_BINARY_LEN);

    if (!(src_op && opcode && dest_op && a_r_e)) {
        handle_error(TERMINATE, "concat_default_12bit()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        strcpy(*binary_word, src_op);
        strcat(*binary_word, opcode);
        strcat(*binary_word, dest_op);
        strcat(*binary_word, a_r_e);
    }

    (*binary_word)[BINARY_BITS] = '\0';
    free_strings(AMT_WORD_4, &src_op, &opcode, &dest_op, &a_r_e);
    return report;
}

/**
 * Concatenates the binary components of a data_image structure according to the REG_DEST concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a char pointer where the concatenated binary word will be stored.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_reg_dest(data_image *data, char** binary_word) {
    char *dest_op = NULL;
    status report = NO_ERROR;

    dest_op = truncate_string(data->binary_dest, SRC_DEST_OP_BINARY_LEN);

    if (!dest_op) {
        handle_error(TERMINATE, "concat_reg_dest()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        memset(*binary_word, '0', BINARY_BITS);
        strncpy(*binary_word + REGISTER_BINARY_LEN, dest_op, REGISTER_BINARY_LEN);
    }

    (*binary_word)[BINARY_BITS] = '\0';
    free_strings(AMT_WORD_1, &dest_op);
    return report;
}

/**
 * Concatenates the binary components of a data_image structure according to the REG_SRC concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a char pointer where the concatenated binary word will be stored.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_reg_src(data_image *data, char** binary_word) {
    char *src_op = NULL;
    status report = NO_ERROR;

    src_op = truncate_string(data->binary_src, REGISTER_BINARY_LEN);

    if (!src_op) {
        handle_error(TERMINATE, "concat_reg_src()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        memset(*binary_word, '0', BINARY_BITS);
        strncpy(*binary_word, src_op, REGISTER_BINARY_LEN); /* start from 11 --> 7 */
    }
    (*binary_word)[BINARY_BITS] = '\0';
    free_strings(AMT_WORD_1, &src_op);
    return report;
}

/**
 * Concatenates the binary components of a data_image structure according to the REG_REG concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a pointer to a char array where the concatenated binary word will be stored.
 *                    The function will allocate memory for the binary word and update the pointer accordingly.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_reg_reg(data_image *data, char** binary_word) {
    char *src_op = NULL, *dest_op = NULL;
    status report = NO_ERROR;

    src_op = truncate_string(data->binary_src, REGISTER_BINARY_LEN);
    dest_op = truncate_string(data->binary_dest, REGISTER_BINARY_LEN);

    if (!(src_op && dest_op)) {
        handle_error(TERMINATE, "concat_reg_reg()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        memset(*binary_word, '0', BINARY_BITS);
        strncpy(*binary_word, src_op, REGISTER_BINARY_LEN);
        strncpy(*binary_word + REGISTER_BINARY_LEN, dest_op, REGISTER_BINARY_LEN);
    }
    (*binary_word)[BINARY_BITS] = '\0';
    free_strings(AMT_WORD_2, &src_op, &dest_op);
    return report;
}

/**
 * Concatenates the binary components of a data_image structure according to the ADDRESS concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a pointer to a char array where the concatenated binary word will be stored.
 *                    The function will allocate memory for the binary word and update the pointer accordingly.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_address(data_image *data, char** binary_word) {
    char *src_op = NULL, *a_r_e = NULL;
    status report = NO_ERROR;

    if (!data->binary_a_r_e || !data->binary_src) {
        if (data->p_sym->sym_dir == EXTERN) {
            strcpy(*binary_word, data->p_sym->address_binary);
            (*binary_word)[BINARY_BITS] = '\0';
            return NO_ERROR;
        }
        src_op = truncate_string(data->p_sym->address_binary, ADDRESS_BINARY_LEN);
        a_r_e = truncate_string(decimal_to_binary12(RELOCATABLE), A_R_E_BINARY_LEN);
    } else {
        src_op = truncate_string(data->binary_src, ADDRESS_BINARY_LEN);
        a_r_e = truncate_string(data->binary_a_r_e, A_R_E_BINARY_LEN);
    }
    if (!(src_op && a_r_e)) {
        handle_error(TERMINATE, "concat_address()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        strcpy(*binary_word, src_op);
        strcat(*binary_word, a_r_e);
    }

    (*binary_word)[BINARY_BITS] = '\0';
    free_strings(AMT_WORD_2, &src_op, &a_r_e);
    return report;
}

/**
 * Determines the addressing mode based on the input source string.
 *
 * @param src The source file context.
 * @param word The input source string to analyze.
 * @param word_len The length of the input source string.
 * @param report A pointer to the status report.
 * @return The addressing mode determined based on the source string.
 *         Possible return values are: REGISTER, IMMEDIATE, DIRECT, and INVALID_MD.
 */
Adrs_mod get_addressing_mode(file_context *src, char *word, size_t word_len, status *report) {
    Value val_type;
    if (*word == REGISTER_CH) {
        if (is_valid_register(src, word, report))
            return REGISTER;
        handle_error(ERR_INVALID_REGISTER, src);
        return INVALID_MD;
    }

    val_type = validate_data(src, word, word_len, report);
    if (val_type == LBL)
        return DIRECT;
    else if (val_type == NUM)
        return IMMEDIATE;
    handle_error(TERMINATE, "get_addressing_mode()");
    return INVALID_MD;
}

/**
 * Checks the legality of the addressing modes for a given command and operands.
 *
 * @param src The source file context.
 * @param cmd The command being checked.
 * @param src_op The addressing mode of the source operand.
 * @param dest_op The addressing mode of the destination operand.
 * @param report A pointer to the status report.
 * @return 1 if the addressing modes are legal for the command, 0 otherwise.
 */
int is_legal_addressing(file_context *src, Command cmd, Adrs_mod src_op, Adrs_mod dest_op, status *report) {
    if (cmd > STOP) {
        *report = ERR_INVALID_OPCODE;
        return 0;
    } else if ((cmd <= JSR && dest_op == INVALID_MD) || (cmd <= SUB && src_op == INVALID_MD)) {
        handle_error(ERR_MISS_OPERAND, src);
        *report = ERR_MISS_OPERAND;
        return 0;
    } else if (((cmd >= INC || (cmd >= NOT && cmd <= CLR)) && src_op != INVALID_MD)
                                        || (cmd >= RTS && dest_op != INVALID_MD)) {
        handle_error(ERR_TOO_MANY_OPERANDS, src);
        *report = ERR_TOO_MANY_OPERANDS;
        return 0;
    } else if ((cmd <= JSR && (cmd != PRN && cmd != CMP))
        && ((dest_op == IMMEDIATE) || (cmd == LEA && src_op != DIRECT))) {
        handle_error(ERR_INVALID_OPERAND, src);
        *report = ERR_INVALID_OPERAND;
        return 0;
    }
    return 1;
}

/**
 * Frees the memory allocated for a Symbol structure, including its members.
 *
 * @param symbol_t Pointer to a pointer to the Symbol structure to be freed.
 *                 The pointer will be set to NULL after freeing the memory.
 */
void free_symbol(symbol **symbol_t) {
    if (!symbol_t || !(*symbol_t)) return;

    if ((*symbol_t)->label)
        free((*symbol_t)->label);
    if ((*symbol_t)->address_binary)
        free((*symbol_t)->address_binary);
    free(*symbol_t);
    *symbol_t = NULL;
}

/**
 * Frees the memory allocated for a symbol table.
 *
 * @param p_symbol_table Pointer to the symbol table.
 * @param size Pointer to the size of the symbol table.
 */
void free_symbol_table(symbol ***p_symbol_table, size_t *size) {
    size_t i;
    symbol** symbol_table = NULL;

    if (!p_symbol_table || !(*p_symbol_table) || !(symbol_table = realloc(*p_symbol_table, (*size) * sizeof (symbol*))))
        return;

    for (i = 0; i < (*size); ++i)
        if (symbol_table[i])
            free_symbol(&(symbol_table[i]));

    free(symbol_table);
    *p_symbol_table = NULL;
    *size = 0;
}

/**
 * Frees the memory allocated for a data_image structure, including its members and the data pointer itself.
 *
 * @param data Pointer to the data_image structure to be freed.
 */
void free_data_image(data_image** data) {
    if (data == NULL || *data == NULL) return;
    if (!((*data)->p_sym && (*data)->binary_src)) free((*data)->binary_src);
    if ((*data)->binary_opcode) free((*data)->binary_opcode);
    if ((*data)->binary_dest) free((*data)->binary_dest);
    if ((*data)->binary_a_r_e) free((*data)->binary_a_r_e);
    if ((*data)->base64_word) free((*data)->base64_word);
    if ((*data)->value) {
        free((*data)->value);
        (*data)->value = NULL;
    }
    free(*data);
    *data = NULL;
}

/**
 * Frees the memory allocated for an array of data_image structures, including their members and the array itself.
 *
 * @param data_array Pointer to the array of data_image structures to be freed.
 * @param size       Size of the data_array.
 */
void free_data_image_array(data_image ***data_array, size_t *size) {
    int i;
    data_image **p_data_array = NULL;

    if (!data_array || !*data_array || !( p_data_array = realloc(*data_array, (*size * sizeof(data_image*)))))
        return;

    for (i = 0; i < *size; i++)
        free_data_image(&p_data_array[i]);

    free(p_data_array);
    *data_array = NULL;
    *size = 0;
}

/**
 * Frees dynamically allocated memory for a given number of strings.
 *
 * @param num_strings The number of strings to free.
 * @param ...         Pointers to the strings to be freed.
 *                    Note: The pointers should be of type char**.
 *
 * Usage: free_strings(num_strings, &str1, &str2, &str3, ...);
 */
void free_strings(int num_strings, ...) {
    char** str_ptr;
    int i;
    va_list args;
    va_start(args, num_strings);

    for (i = 0; i < num_strings; ++i) {
        str_ptr = va_arg(args, char**);
        if (*str_ptr) {
            free(*str_ptr);
            *str_ptr = NULL;
        }
    }

    va_end(args);
}