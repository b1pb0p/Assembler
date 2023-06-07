/* first_pass.c
 * Assembler's first pass process.
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "passes.h"
#include "utils.h"
#include "errors.h"
#include "data.h"

#define FREE_AND_RETURN_FAILURE { free_data_image(&data_img); \
free_file_context_array(contexts, INPUT_OUTPUT_NUM_FILES); \
return FAILURE; \
};


symbol **symbol_table = NULL;
data_image *data_img = NULL;
size_t symbol_count = 0;
size_t data_img_size = 0;

status assembler_first_pass(file_context** contexts) {
    file_context* src = contexts[0];
    file_context* out_entry = contexts[1];
    file_context* out_extern = contexts[2];
    file_context* out_obj = contexts[3];


    return NO_ERROR;
}


status assembler_second_pass(file_context** contexts) {
    file_context* src = contexts[0];
    file_context* out_entry = contexts[1];
    file_context* out_extern = contexts[2]; /* TODO: check if can be resolved in first pass */
    file_context* out_obj = contexts[3];
    int i;

    if (!src || !out_obj)
    FREE_AND_RETURN_FAILURE;

    fprintf(out_obj->file_ptr, "%d %d\n", src->ic, src->dc);

    for (i = 0; i < data_img_size; i++) {
        if (data_img[i].missing_info) {
            if (!data_img[i].symbol_t->address_decimal) {
                handle_error(ERR_LABEL_DOES_NOT_EXIST, src, data_img->lc);
                FREE_AND_RETURN_FAILURE;
            }
            data_img[i].base64_word = convert_bin_to_base64(data_img->symbol_t->address_binary);
            if (!data_img[i].base64_word) FREE_AND_RETURN_FAILURE;
        }
        if (!data_img[i].base64_word) FREE_AND_RETURN_FAILURE;
        fprintf(out_obj->file_ptr, "%s\n", data_img[i].base64_word);

    }

    free_data_image(&data_img);
    free_file_context_array(contexts, INPUT_OUTPUT_NUM_FILES); /* TODO: free ** at main */
    return NO_ERROR;
}

/**
 * Add a symbol to the symbol table.
 * If the symbol already exists and has missing information, its information is updated with the provided address.
 * If the symbol already exists and has complete information, NULL is returned.
 * If the symbol is new, it is added to the symbol table with the provided label and address.
 *
 * @param label The label of the symbol to add.
 * @param address The address of the symbol.
 * @return A pointer to the added symbol if it is new or has missing information, or NULL if the symbol already exists with complete information.
 *         Returns NULL in case of memory allocation errors during symbol creation or table expansion.
 */
symbol* add_symbol(file_context *src, const char* label, int address) {
    symbol** new_symbol_table = NULL;
    symbol* new_symbol = NULL;
    symbol* existing_symbol = find_symbol(label);

    if (existing_symbol) {
        if (existing_symbol->is_missing_info && address != INVALID_ADDRESS)
            update_symbol_info(existing_symbol, address);
        else {
            handle_error(ERR_DUP_LABEL, src);
            return NULL;
        }
    } else {
        new_symbol_table = realloc(symbol_table, (symbol_count + 1) * sizeof(symbol*));
        new_symbol = malloc(sizeof(symbol));

        if (!new_symbol_table || !new_symbol || copy_string(&(new_symbol->label), label) != NO_ERROR)    {
            handle_error(ERR_MEM_ALLOC);
            free_symbol(&new_symbol);
            free_symbol_table();
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
 * Frees the memory allocated for the symbol table, including all symbols and their members.
 */
void free_symbol_table() {
    size_t i;

    if (!symbol_table)
        return;

    for (i = 0; i < symbol_count; ++i)
        if (symbol_table[i])
            free_symbol(&symbol_table[i]);

    free(symbol_table);
    symbol_table = NULL;
    symbol_count = 0;
}

/*


    char line[MAX_LINE_LENGTH];
    int IC = 0; // Instruction Counter
    int DC = 0; // Data Counter
    int hasSymbolDefinition = 0; // Flag for symbol definition

    while (fgets(line, sizeof(line), sourceFile) != NULL) {
        // Check if line is empty or a comment
        if (line[0] == '\n' || line[0] == ';') {
            continue;
        }

        // Remove newline character at the end
        line[strcspn(line, "\n")] = '\0';

        // Tokenize the line
        char* token = strtok(line, " \t");
        if (token == NULL) {
            continue; // Empty line
        }

        // Check if the first field is a symbol
        int isSymbol = (strchr(token, ':') != NULL);
        if (isSymbol) {
            hasSymbolDefinition = 1;
            // Process symbol definition, insert into symbol table
            char* symbolName = strtok(token, ":");
            Symbol* existingSymbol = findSymbol(symbolName);
            if (existingSymbol != NULL) {
                printf("Error: Symbol '%s' already defined.\n", symbolName);
                return 1;
            }
            insertSymbol(symbolName, (hasSymbolDefinition == DATA) ? DC : IC, hasSymbolDefinition ? DATA : EXTERN);
            token = strtok(NULL, " \t"); // Move to next token
        }

        // Process directives and instructions
        if (strcmp(token, "data") == 0) {
            // Process data directive
            token = strtok(NULL, " \t"); // Move to next token
            while (token != NULL) {
                // Process data value
                int value = atoi(token);
                // Update data image and increment DC
                // ...
                token = strtok(NULL, " \t"); // Move to next token
            }
        } else if (strcmp(token, "string") == 0) {
            // Process string directive
            token = strtok(NULL, ""); // Move to next token (remaining line)
            // Process string value
            // Update data image and increment DC
            // ...
        } else if (strcmp(token, "entry") == 0) {
            // Process entry directive
            token = strtok(NULL, " \t"); // Move to next token
            while (token != NULL) {
                // Process entry symbol
                Symbol* entrySymbol = findSymbol(token);
                if (entrySymbol != NULL) {
                    entrySymbol->type = ENTRY;
                } else {
                    printf("Error: Entry symbol '%s' not found.\n", token);
                    return 1;
                }
                token = strtok(NULL, " \t"); // Move to next token
            }
        } else if (strcmp(token, "extern") == 0) {
            // Process extern directive
            token = strtok(NULL, " \t"); // Move to next token
            while (token != NULL) {
                // Process extern symbol
                Symbol* existingSymbol = findSymbol(token);
                if (existingSymbol != NULL) {
                    printf("Error: Symbol '%s' already defined.\n", token);
                    return 1;
                }
                insertSymbol(token, 0, EXTERN);
                token = strtok(NULL, " \t"); // Move to next token
            }
        } else {
            // Process instruction
            // Update symbol table and increment IC accordingly
            // Look for operation name in operation table
            // Analyze instruction operands and calculate L
            // Build binary code of the instruction
        }
    }

    fclose(sourceFile);

    // Update data type symbols in the symbol table with final IC value
    for (int i = 0; i < symbolCount; i++) {
        if (symbolTable[i].type == DATA) {
            symbolTable[i].value += IC;
        }
    }

    // Start second pass

    return 0;
}
*/