/* first_pass.c
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "first_pass.h"
#include "errors.h"


/**
 * Converts a decimal number to a binary string representation.
 *
 * @param decimal The decimal number to be converted to binary.
 *
 * @return A dynamically allocated binary string representation of the decimal number, or NULL if memory allocation fails.
 */
char* decimal_to_binary12(int decimal) {
    char* binary = (char*)malloc((BINARY_BITS + 1) * sizeof(char));
    if (!binary) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    int i;
    int isNegative = 0;

    if (decimal < 0) {
        decimal = -decimal;
        isNegative = 1;
    }

    for (i = BINARY_BITS - 1; i >= 0; i--) {
        binary[i] = (char)((decimal % 2) + '0');
        decimal /= 2;
    }

    if (isNegative) {
        /* Two's complement for negative numbers */
        for (i = 0; i < BINARY_BITS; i++) {
            binary[i] = (binary[i] == '0') ? '1' : '0';
        }
        int carry = 1;
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
    unsigned int truncate_len = length;
    if (truncate_len > input_len) {
        handle_error(TERMINATE, "truncate_string()");
        return NULL;
    }

    truncate_len = input_len - truncate_len;

    char* truncated = (char*)malloc((truncate_len + 1) * sizeof(char));
    if (!truncated) {
        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    strcpy(truncated, input + truncate_len);
    truncated[truncate_len] = '\0';
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
 * Concatenates the binary representations of four components into a 12-bit binary string.
 * Concatenating by this format ___ ____ ___ __ for each parameter respectively.
 *
 * @param src_op   The source operand binary representation.
 * @param opcode   The opcode binary representation.
 * @param dest_op  The destination operand binary representation.
 * @param a_r_e    The A/R/E (Addressing/Record/Extended) bits binary representation.
 *
 * @return A dynamically allocated 12-bit binary string representation of the concatenated components,
 *         or NULL if memory allocation fails or any of the input strings are truncated incorrectly.
 */
char* concat_12bits(const char* src_op, const char* opcode, const char* dest_op, const char* a_r_e) {
    /* ___ ____ ___ __ */
    char *tr_src, *tr_opcode, *tr_dest, *tr_are, *cat_str;
    cat_str = NULL;
    cat_str = calloc(BINARY_BITS + 1, sizeof(char));
    tr_src = truncate_string(src_op, SRC_DEST_OP_BINARY_LEN);
    tr_opcode = truncate_string(opcode, OPCODE_BINARY_LEN);
    tr_dest = truncate_string(dest_op, SRC_DEST_OP_BINARY_LEN);
    tr_are = truncate_string(a_r_e, A_R_E_BINARY_LEN);

    if (!(cat_str && tr_src && tr_opcode && tr_dest && tr_are)) {
        if (cat_str) free(cat_str);
        if (tr_src) free(tr_src);
        if (tr_opcode) free(tr_opcode);
        if (tr_dest) free(tr_dest);
        if (tr_are) free(tr_are);

        handle_error(ERR_MEM_ALLOC);
        return NULL;
    }

    cat_str = strcat(cat_str,tr_src);
    cat_str = strcat(cat_str,tr_opcode);
    cat_str = strcat(cat_str, tr_dest);
    cat_str = strcat(cat_str,tr_are);

    free(tr_src);
    free(tr_opcode);
    free(tr_dest);
    free(tr_are);

    cat_str[BINARY_BITS] = '\0';
    return cat_str;
}

#define MAX_SYMBOL_LENGTH 32
#define MAX_SYMBOLS 100

//typedef struct {
//    char name[MAX_SYMBOL_LENGTH];
//    int value;
//    DirectiveType type;
//} Symbol;
//
//Symbol symbolTable[MAX_SYMBOLS];
//int symbolCount = 0;
//
//void insertSymbol(const char* name, int value, DirectiveType type) {
//    strcpy(symbolTable[symbolCount].name, name);
//    symbolTable[symbolCount].value = value;
//    symbolTable[symbolCount].type = type;
//    symbolCount++;
//}
//
//Symbol* findSymbol(const char* name) {
//    for (int i = 0; i < symbolCount; i++) {
//        if (strcmp(symbolTable[i].name, name) == 0) {
//            return &symbolTable[i];
//        }
//    }
//    return NULL;
//}
//
//    char line[MAX_LINE_LENGTH];
//    int IC = 0; // Instruction Counter
//    int DC = 0; // Data Counter
//    int hasSymbolDefinition = 0; // Flag for symbol definition
//
//    while (fgets(line, sizeof(line), sourceFile) != NULL) {
//        // Check if line is empty or a comment
//        if (line[0] == '\n' || line[0] == ';') {
//            continue;
//        }
//
//        // Remove newline character at the end
//        line[strcspn(line, "\n")] = '\0';
//
//        // Tokenize the line
//        char* token = strtok(line, " \t");
//        if (token == NULL) {
//            continue; // Empty line
//        }
//
//        // Check if the first field is a symbol
//        int isSymbol = (strchr(token, ':') != NULL);
//        if (isSymbol) {
//            hasSymbolDefinition = 1;
//            // Process symbol definition, insert into symbol table
//            char* symbolName = strtok(token, ":");
//            Symbol* existingSymbol = findSymbol(symbolName);
//            if (existingSymbol != NULL) {
//                printf("Error: Symbol '%s' already defined.\n", symbolName);
//                return 1;
//            }
//            insertSymbol(symbolName, (hasSymbolDefinition == DATA) ? DC : IC, hasSymbolDefinition ? DATA : EXTERN);
//            token = strtok(NULL, " \t"); // Move to next token
//        }
//
//        // Process directives and instructions
//        if (strcmp(token, "data") == 0) {
//            // Process data directive
//            token = strtok(NULL, " \t"); // Move to next token
//            while (token != NULL) {
//                // Process data value
//                int value = atoi(token);
//                // Update data image and increment DC
//                // ...
//                token = strtok(NULL, " \t"); // Move to next token
//            }
//        } else if (strcmp(token, "string") == 0) {
//            // Process string directive
//            token = strtok(NULL, ""); // Move to next token (remaining line)
//            // Process string value
//            // Update data image and increment DC
//            // ...
//        } else if (strcmp(token, "entry") == 0) {
//            // Process entry directive
//            token = strtok(NULL, " \t"); // Move to next token
//            while (token != NULL) {
//                // Process entry symbol
//                Symbol* entrySymbol = findSymbol(token);
//                if (entrySymbol != NULL) {
//                    entrySymbol->type = ENTRY;
//                } else {
//                    printf("Error: Entry symbol '%s' not found.\n", token);
//                    return 1;
//                }
//                token = strtok(NULL, " \t"); // Move to next token
//            }
//        } else if (strcmp(token, "extern") == 0) {
//            // Process extern directive
//            token = strtok(NULL, " \t"); // Move to next token
//            while (token != NULL) {
//                // Process extern symbol
//                Symbol* existingSymbol = findSymbol(token);
//                if (existingSymbol != NULL) {
//                    printf("Error: Symbol '%s' already defined.\n", token);
//                    return 1;
//                }
//                insertSymbol(token, 0, EXTERN);
//                token = strtok(NULL, " \t"); // Move to next token
//            }
//        } else {
//            // Process instruction
//            // Update symbol table and increment IC accordingly
//            // Look for operation name in operation table
//            // Analyze instruction operands and calculate L
//            // Build binary code of the instruction
//        }
//    }
//
//    fclose(sourceFile);
//
//    // Update data type symbols in the symbol table with final IC value
//    for (int i = 0; i < symbolCount; i++) {
//        if (symbolTable[i].type == DATA) {
//            symbolTable[i].value += IC;
//        }
//    }
//
//    // Start second pass
//
//    return 0;
//}
