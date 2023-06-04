/* data.h
 * data image and base conversions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include "data.h"
#include "passes.h"
#include "errors.h"
#include "utils.h"


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
    unsigned int input_len = strlen(input);
    unsigned int truncate_len = input_len - length;
    char* truncated;
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
 * Concatenates the binary representations of components into a 12-bit binary string based on the specified action.
 *
 * @param action   The action indicating the concatenation type (concat_actions enum).
 * @param ...      Variable arguments depending on the action type.
 *
 * @return A dynamically allocated 12-bit binary string representation of the concatenated components,
 *         or NULL if memory allocation fails or the input strings are truncated incorrectly.
 */
char* fconcat_12bits(concat_actions action , ... ) {
    va_list args;
    char* src_op;
    char* opcode;
    char* dest_op;
    char* a_r_e;
    char* cat_str = calloc(BINARY_BITS + 1, sizeof(char));

    if (!cat_str) {
        printf("error");
        return NULL;
    }
    va_start(args, action);

    if (action == DEFAULT_12BIT) {
        /* ___ ____ ___ __  - src, opcode, dest, a/r/e */
        src_op = truncate_string(va_arg(args, const char*), SRC_DEST_OP_BINARY_LEN);
        opcode = truncate_string(va_arg(args, const char*), OPCODE_BINARY_LEN);
        dest_op = truncate_string(va_arg(args, const char*), SRC_DEST_OP_BINARY_LEN);
        a_r_e = truncate_string(va_arg(args, const char*), A_R_E_BINARY_LEN);

        if (!(src_op && opcode && dest_op && a_r_e)) {
            free(cat_str);
            if (src_op) free(src_op);
            if (opcode) free(opcode);
            if (dest_op) free(dest_op);
            if (a_r_e) free(a_r_e);

            handle_error(ERR_MEM_ALLOC);
            return NULL;
        }

        cat_str = strcat(cat_str,src_op);
        cat_str = strcat(cat_str,opcode);
        cat_str = strcat(cat_str, dest_op);
        cat_str = strcat(cat_str,a_r_e);

        free(src_op);
        free(opcode);
        free(dest_op);
        free(a_r_e);

    }
    else if (action == REG_DEST) {
        /* _____ _____ __  - 0, dest, 0 */
        dest_op = truncate_string(va_arg(args, const char*), REGISTER_BINARY_LEN);

        if (!dest_op) {
            handle_error(ERR_MEM_ALLOC);
            free(cat_str);
            return NULL;
        }
        memset(cat_str, '0', BINARY_BITS);
        strncpy(cat_str + REGISTER_BINARY_LEN, dest_op, REGISTER_BINARY_LEN);
        free(dest_op);
    }
    else if (action == REG_SRC) {
        /* _____ _______  - src, 0 */
        src_op = truncate_string(va_arg(args, const char*), REGISTER_BINARY_LEN);

        if (!src_op) {
            handle_error(ERR_MEM_ALLOC);
            free(cat_str);
            return NULL;
        }
        memset(cat_str, '0', BINARY_BITS);
        strncpy(cat_str, src_op, REGISTER_BINARY_LEN); /* start from 11 --> 7 */
        free(src_op);
    }
    else if (action == REG_REG) {
        /* _____ _____ __  - src, dest, 0 */
        src_op = truncate_string(va_arg(args, const char*), REGISTER_BINARY_LEN);
        dest_op = truncate_string(va_arg(args, const char*), REGISTER_BINARY_LEN);

        if (!(src_op && dest_op)) {
            handle_error(ERR_MEM_ALLOC);
            if(src_op) free(src_op);
            if(dest_op) free(dest_op);

            free(cat_str);
            return NULL;
        }
        memset(cat_str, '0', BINARY_BITS);
        strncpy(cat_str, src_op, REGISTER_BINARY_LEN);
        strncpy(cat_str + REGISTER_BINARY_LEN, dest_op, REGISTER_BINARY_LEN);
        free(src_op);
        free(dest_op);
    }
    else if (action == ADDRESS) {
        /* __________ __  - address (src) , a/r/e */
        src_op = truncate_string(va_arg(args, const char*), ADDRESS_BINARY_LEN);
        a_r_e = truncate_string(va_arg(args, const char*), A_R_E_BINARY_LEN);

        if (!(src_op && a_r_e)) {
            handle_error(ERR_MEM_ALLOC);
            if(src_op) free(src_op);
            if(a_r_e) free(a_r_e);

            free(cat_str);
            return NULL;
        }

        cat_str = strcat(cat_str,src_op);
        cat_str = strcat(cat_str,a_r_e);

        free(src_op);
        free(a_r_e);
    }
    else {
        handle_error(TERMINATE, "fconcat_12bits()");
        free(cat_str);
        return NULL;
    }

    va_end(args);
    cat_str[BINARY_BITS] = '\0';

    return cat_str;
}

/**
 * Frees the memory allocated for a Symbol structure, including its members.
 *
 * @param symbol_t Pointer to a pointer to the Symbol structure to be freed.
 *                 The pointer will be set to NULL after freeing the memory.
 */
void free_symbol(symbol** symbol_t) {
    int i;
    if (!symbol_t) return;

    if ((*symbol_t)->name) free((*symbol_t)->name);
    if ((*symbol_t)->address) free((*symbol_t)->address);

    if ((*symbol_t)->values != NULL) {
        for (i = 0; (*symbol_t)->values[i] != NULL; i++)
            free((*symbol_t)->values[i]);
        free((*symbol_t)->values);
    }

    free((*symbol_t));
    *symbol_t = NULL;
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
    if ((*data)->binary_word) free((*data)->binary_word);
    if ((*data)->base64_word) free((*data)->base64_word);
    if ((*data)->symbol_t) free_symbol(&((*data)->symbol_t));
    free(*data);
    *data = NULL;
}


