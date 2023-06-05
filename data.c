/* data.h
 * data image and base conversions.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
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
 * Concatenates the binary representations of components into a 12-bit binary string based on the specified action,
 * and converts the result to base64.
 *
 * @param data Pointer to the data_image structure.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status concatenate_and_convert_to_base64(data_image* data) {
    if (!data) {
        handle_error(TERMINATE, "concatenate_and_convert_to_base64()");
        return FAILURE;
    }

    if (data->missing_info)
        return NO_ERROR; /* To not trigger and false alarm, will be handled in second pass */

    if (data->concat == DEFAULT_12BIT) {
        /* ___ ____ ___ __  - src, opcode, dest, a/r/e */
        if (!(data->binary_src && data->binary_opcode && data->binary_dest && data->binary_a_r_e)) {
            handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Insufficient binary data)");
            return FAILURE;
        }

        strcpy(data->binary_word, data->binary_src);
        strcat(data->binary_word, data->binary_opcode);
        strcat(data->binary_word, data->binary_dest);
        strcat(data->binary_word, data->binary_a_r_e);
    }
    else if (data->concat == REG_DEST) {
        /* _____ _____ __  - 0, dest, 0 */
        if (!data->binary_dest) {
            handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Insufficient binary data)");
            return FAILURE;
        }

        memset(data->binary_word, '0', BINARY_BITS);
        strncpy(data->binary_word + REGISTER_BINARY_LEN, data->binary_dest, REGISTER_BINARY_LEN);
    }
    else if (data->concat == REG_SRC) {
        /* _____ _______  - src, 0 */
        if (!data->binary_src) {
            handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Insufficient binary data)");
            return FAILURE;
        }

        memset(data->binary_word, '0', BINARY_BITS);
        strncpy(data->binary_word, data->binary_src, REGISTER_BINARY_LEN); /* start from 11 --> 7 */
    }
    else if (data->concat == REG_REG) {
        /* _____ _____ __  - src, dest, 0 */
        if (!(data->binary_src && data->binary_dest)) {
            handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Insufficient binary data)");
            return FAILURE;
        }

        memset(data->binary_word, '0', BINARY_BITS);
        strncpy(data->binary_word, data->binary_src, REGISTER_BINARY_LEN);
        strncpy(data->binary_word + REGISTER_BINARY_LEN, data->binary_dest, REGISTER_BINARY_LEN);
    }
    else if (data->concat == ADDRESS) {
        /* __________ __  - address (src), a/r/e */
        if (!(data->binary_src && data->binary_a_r_e)) {
            handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Insufficient binary data)");
            return FAILURE;
        }

        strcpy(data->binary_word, data->binary_src);
        strcat(data->binary_word, data->binary_a_r_e);
    }
    else {
        handle_error(TERMINATE, "concatenate_and_convert_to_base64() (Invalid concat action)");
        return FAILURE;
    }

    /* Convert binary to base64 */
    data->base64_word = convert_bin_to_base64(data->binary_word);
    if (!data->base64_word)
        return FAILURE; /* Error message printed via convert_bin_to_base64() */

    return NO_ERROR;
}

/**
 * Determines the addressing mode based on the input source string.
 *
 * @param src The input source string to analyze.
 * @return The addressing mode determined based on the source string.
 *         Possible return values are:
 *         - REGISTER: If the source string starts with '@'.
 *         - DIRECT: If the source string starts with a digit.
 *         - INDIRECT: If the source string starts with an alphabetic character.
 *         - INVALID: If the source string does not match any addressing mode.
 */
addressing_modes get_addressing_mode(const char *src) {
    if (*src == REGISTER_CH)
        return REGISTER;

    if (*src == '+' || *src == '-')
        src++;

    if (isdigit(*src))
        return DIRECT;

    if (isalpha(*src))
        return INDIRECT;

    handle_error(TERMINATE, "get_addressing_mode()");
    return INVALID;
}

/**
 * Checks if the combination of command and addressing modes is legal.
 *
 * @param cmd  The command to check.
 * @param src  The addressing mode of the source operand.
 * @param dest The addressing mode of the destination operand.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status is_legal_addressing(command cmd, addressing_modes src, addressing_modes dest) {
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
            printf("error this command doesn't take any parameters");
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

/**
 * Frees the memory allocated for an array of data_image structures, including their members and the array itself.
 *
 * @param data_array Pointer to the array of data_image structures to be freed.
 * @param size       Size of the data_array.
 */
void free_data_image_array(data_image*** data_array, int size) {
    int i;

    if (!data_array || !*data_array) return;

    for (i = 0; i < size; i++) {
        free_data_image(&((*data_array)[i]));
    }

    free(*data_array);
    *data_array = NULL;
}


