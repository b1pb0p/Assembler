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

/* "Private" helper functions */
status concat_default_12bit(data_image* data, char** binary_word);
status concat_reg_dest(data_image* data, char** binary_word);
status concat_reg_src(data_image* data, char** binary_word);
status concat_reg_reg(data_image* data, char** binary_word);
status concat_address(data_image* data, char** binary_word);

/**
 * Converts a decimal number to a binary string representation.
 *
 * @param decimal The decimal number to be converted to binary.
 *
 * @return A dynamically allocated binary string representation of the decimal number, or NULL if memory allocation fails.
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
 *
 * @return A dynamically allocated truncated string, or NULL if an error occurs or,
 * `length` is greater than the input string length.
 */
char* truncate_string(const char* input, int length) {
    unsigned int input_len, truncate_len;
    char* truncated = NULL;

    if (!input) {
        if (!(truncated = calloc(length + 1, sizeof(char ))))
            handle_error(ERR_MEM_ALLOC);
        return truncated;
    }

    input_len = strlen(input);
    truncate_len = input_len - length;

    if (length >= input_len) {
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
 * Converts a 12 bits binary string to a Base64 string.
 *
 * Processes the input binary string and converts it to a Base64 string representation.
 *
 * @param binary The input binary string to be converted.
 *
 * @return A Base64 string representation of the input.
 *         Returns NULL if an error occurs.
 */
char* convert_bin_to_base64(const char* binary) {
    int i = 0;
    int j = 0;
    unsigned char value = 0;
    static const char* lookup_table = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    char* base64 = malloc(BASE64_CHARS + 1);

    if (!base64) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    if (binary[BINARY_BITS] != '\0') {
        handle_error(TERMINATE, "convert_bin_to_base64()");
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
 * Creates the base64 word for a data_image structure by concatenating the binary components
 * and converting the binary word to base64 format.
 *
 * @param data The data_image structure containing the binary components.
 * @return The status of the base64 word creation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status create_base64_word(data_image* data) {
    status report = NO_ERROR;
    char* binary_word = malloc((BINARY_BITS + 1) * sizeof(char));

    if (!binary_word) {
        handle_error(ERR_MEM_ALLOC);
        return FAILURE;
    }

    if (!data) {
        handle_error(TERMINATE, "create_base64_word()");
        free(binary_word);
        return FAILURE;
    }

    //if (!data->is_word_complete)
    //    return NO_ERROR; /* TODO: check ; To not trigger any false alarm, will be handled in the second pass */

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
    else if (data->concat == VALUE)
        report = (binary_word = decimal_to_binary12(*(data->value))) ? NO_ERROR : FAILURE;
    else
        handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Invalid concat action)");

    if (report != FAILURE) {
        /* Convert binary to base64 */
        data->base64_word = convert_bin_to_base64(binary_word);
        if (!data->base64_word)
            report = FAILURE; /* Error message printed via convert_bin_to_base64() */
    }

    free(binary_word);
    return report;
}

/**
 * Creates and initializes a new data image with the specified location counter (lc).
 *
 * @param lc The location counter value for the data image.
 * @return A pointer to the newly created data image, or NULL if memory allocation fails.
 */
data_image* create_data_image(int lc) {
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

    p_ret->concat = DEFAULT_12BIT;
    p_ret->p_sym = NULL;
    p_ret->value = NULL;

    p_ret->is_word_complete = 0;
    p_ret->lc = lc;

    return p_ret;
}

/**
 * Concatenates the binary components of a data_image structure according to the DEFAULT_12BIT concatenation.
 *
 * @param data The data_image structure containing the binary components.
 * @param binary_word A pointer to a char array where the concatenated binary word will be stored.
 * @return The status of the concatenation operation. Returns NO_ERROR if successful, or FAILURE if an error occurs.
 */
status concat_default_12bit(data_image* data, char** binary_word) {
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
status concat_reg_dest(data_image* data, char** binary_word) {
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
status concat_reg_src(data_image* data, char** binary_word) {
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
status concat_reg_reg(data_image* data, char** binary_word) {
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
status concat_address(data_image* data, char** binary_word) {
    char *src_op = NULL, *a_r_e = NULL;
    status report = NO_ERROR;

    src_op = truncate_string(data->binary_src, ADDRESS_BINARY_LEN);
    a_r_e = truncate_string(data->binary_a_r_e, A_R_E_BINARY_LEN);

    if (!(src_op && a_r_e)) {
        handle_error(TERMINATE, "concat_address()");
        report = FAILURE;
    }

    if (report != FAILURE) {
        strcpy(*binary_word, src_op);
        strcat(*binary_word, a_r_e);
    }

    free_strings(AMT_WORD_2, &src_op, &a_r_e);
    return report;
}

/**
 * Determines the addressing mode based on the input source string.
 *
 * @param word The input source string to analyze.
 * @return The addressing mode determined based on the source string.
 *         Possible return values are:
 *         - REGISTER: If the source string starts with '@'.
 *         - DIRECT: If the source string starts with a digit.
 *         - INDIRECT: If the source string starts with an alphabetic character.
 *         - INVALID: If the source string does not match any addressing mode.
 */
Addressing_modes get_addressing_mode(file_context *src, const char *word) {
    if (*word == REGISTER_CH) {
        if (is_valid_register(word))
            return REGISTER;
        handle_error(ERR_INVALID_REGISTER, src);
        return INVALID;
    }
        return REGISTER;

    if (*word == '+' || *word == '-')
        word++;

    if (isdigit(*word))
        return DIRECT;

    if (isalpha(*word))
        return INDIRECT;

    handle_error(TERMINATE, "get_addressing_mode()");
    return INVALID;
}

/**
 * Checks if the combination of Command and addressing modes is legal.
 *
 * @param cmd  The Command to check.
 * @param src  The addressing mode of the source operand.
 * @param dest The addressing mode of the destination operand.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status is_legal_addressing(Command cmd, Addressing_modes src, Addressing_modes dest) {
    status error_flag = NO_ERROR;

    if (cmd <= SUB || cmd == LEA) {
        if (src == INVALID) {
            printf("error invalid addressing mode of src");
            error_flag = FAILURE;
        }

        if ((cmd == MOV && dest == INVALID) || (!(dest == INDIRECT || dest == REGISTER))) {
            printf("error invalid addressing mode of dest");
            error_flag = FAILURE;
        }
    }
    else if (cmd >= RTS) {
        if (src != INVALID || dest != INVALID) {
            printf("error this Command doesn't take any parameters");
            error_flag = FAILURE;
        }
    }
    else if (cmd == PRN) {
        if (src != INVALID) {
            printf("error invalid addressing mode of src");
            error_flag = FAILURE;
        }

        if (dest == INVALID) {
            printf("error invalid addressing mode of dest");
            error_flag = FAILURE;
        }
    }

    return error_flag;
}

/**
 * Frees the memory allocated for a Symbol structure, including its members.
 *
 * @param symbol_t Pointer to a pointer to the Symbol structure to be freed.
 *                 The pointer will be set to NULL after freeing the memory.
 */
void free_symbol(symbol** symbol_t) {
    if (!symbol_t || !(*symbol_t)) return;

    if ((*symbol_t)->label) {
        free((*symbol_t)->label);
        (*symbol_t)->label = NULL;
    }

    if ((*symbol_t)->address_binary) {
        free((*symbol_t)->address_binary);
        (*symbol_t)->address_binary = NULL;
    }

    if ((*symbol_t)->data) {
        free_data_image(&(*symbol_t)->data);
        (*symbol_t)->data = NULL;
    }

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

    if (!p_symbol_table || !(*p_symbol_table))
        return;

    symbol_table = *p_symbol_table;

    for (i = 0; i < *size; ++i)
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
    if ((*data)->binary_src) free((*data)->binary_src);
    if ((*data)->binary_opcode) free((*data)->binary_opcode);
    if ((*data)->binary_dest) free((*data)->binary_dest);
    if ((*data)->binary_a_r_e) free((*data)->binary_a_r_e);
    if ((*data)->base64_word) free((*data)->base64_word);
    if ((*data)->p_sym) free((*data)->p_sym);
    if ((*data)->value) free((*data)->value);
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

    if (!data_array || !*data_array) return;

    for (i = 0; i < *size; i++) {
        free_data_image(&((*data_array)[i]));
    }

    free(*data_array);
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

