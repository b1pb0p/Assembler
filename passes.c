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

#define INVAL_INPUT_RETURN(name) if (!src || !out_obj || !out_entry || !out_extern) { \
    handle_error(TERMINATE, "assembler_" #name "_pass()"); \
    return FAILURE; \
}

symbol **symbol_table = NULL;
data_image **data_img = NULL;
size_t symbol_count = 0;
size_t data_img_count = 0;

status assembler_first_pass(file_context ***contexts) {
    char line[MAX_BUFFER_LENGTH];
    file_context *src, *out_entry, *out_extern, *out_obj;
    status report = NO_ERROR;

    if (!contexts || !*contexts)
        return FAILURE;

    src = (*contexts)[0];
    out_entry = (*contexts)[1];
    out_extern = (*contexts)[2];
    out_obj = (*contexts)[3];

    INVAL_INPUT_RETURN(first);

    /* Checking for comment lines (;), invalid line start and handling too long lines
     * is taken care of at the preprocessor stage. */
    while (fscanf(src->file_ptr, "%[^\n]%*c", line) == 1) {
        report =  process_line(line, src->ic, src->dc);
        src->lc++;
    }


    /* Generate object output file only if no error occurred */
    if (report == NO_ERROR)
        report = assembler_second_pass(contexts);
    else /* Cleanup output files if an error occurred */
        cleanup_output_files(contexts);

    return report;
}

/*** NEED TO BE WRITTEN AGAIN ***/
status process_line(char *p_line, int ic, int dc) {
    char label[MAX_LABEL_LENGTH];
    int word_length;
    status report = NO_ERROR;

    word_length = get_word(&p_line, label);

    if (is_label(label)) {
        report = process_label(label, ic);
        if (report != NO_ERROR) {
            return report;
        }
        word_length = get_word(&p_line, label);
    }

    if (is_command(label)) {
        char first_param[MAX_LABEL_LENGTH], second_param[MAX_LABEL_LENGTH];
        report = process_command_word(label, p_line, first_param, second_param);
        if (report == NO_ERROR) {
            /* TODO: FILL WITH CODE HOPEFULLY */
        }
    }

    return report;
}

int is_label(const char *label) {
    size_t length = strlen(label);
    if (length == 0 || length > MAX_LABEL_LENGTH)
        return 0;
    if (label[length - 1] == ':')
        return 1;
    return 0;
}

status process_label(const char *label, int ic) {
    char clean_label[MAX_LABEL_LENGTH];
    symbol *new_symbol = NULL;

    strncpy(clean_label, label, strlen(label) - 1);
    clean_label[strlen(label) - 1] = '\0';

    if (is_directive(clean_label) || is_command(clean_label)) {
        handle_error(TERMINATE, "Invalid label name");
        return FAILURE;
    }

    new_symbol = create_symbol(clean_label, ic + ADDRESS_START);
    return NO_ERROR;
}

status process_command_word(const char *command, char *p_line, char *first_param, char *second_param) {
    size_t word_length;

    word_length = get_word(&p_line, first_param);

    if (*p_line == ',') {
        p_line++;
        word_length = get_word(&p_line, second_param);

        if (*p_line != '\n' && *p_line != '\0') {
            handle_error(TERMINATE, "Invalid command format");
            return FAILURE;
        }
    } else {
        if (*p_line != '\n' && *p_line != '\0') {
            handle_error(TERMINATE, "Invalid command format");
            return FAILURE;
        }
    }

    return NO_ERROR;
}

status skip_white_spaces(char *line) {
    int i;

    for (i = 0; line[i] == ' ' || line[i] == '\t' || line[i] == '\n'; i++)
        ;
    if (line[i] == '\0' || line[i] == '\n')
        return EOF;

    strcpy(line, line + i);
    return NO_ERROR;
}
/*** UP HERE BUDDY ***/



/**
 * Perform the second pass of the assembler, generating the object output.
 *
 * @param contexts The array of file contexts containing input and output file information.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status assembler_second_pass(file_context ***contexts) {
    file_context *src, *out_entry, *out_extern, *out_obj;
    status report = NO_ERROR;

    if (!contexts || !*contexts)
        return FAILURE;

    src = (*contexts)[0];
    out_entry = (*contexts)[1];
    out_extern = (*contexts)[2];
    out_obj = (*contexts)[3];

    INVAL_INPUT_RETURN(second);

    report = generate_obj_output(out_obj, src->ic, src->dc);

    /* free global data_image and symbol_table arrays */
    free_data_image_array(&data_img, &data_img_count);
    free_symbol_table(&symbol_table, &symbol_count);

    if (report != NO_ERROR)
        cleanup_output_files(contexts);

    return report;
}


/**
 * Generate the object output file based on the data image and symbol table.
 *
 * @param obj_file The file context for the object output file.
 * @param ic The instruction count.
 * @param dc The data count.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status generate_obj_output(file_context *obj_file, int ic, int dc) {
    int i;
    status report = NO_ERROR;

    fprintf(obj_file->file_ptr, "%d %d\n", ic, dc);

    for (i = 0; i < data_img_count; i++) {
        if ((*data_img[i]).missing_info)
            report = FAILURE;

        (*data_img[i]).base64_word = convert_bin_to_base64((*data_img)->symbol_t->address_binary);
        if (!(*data_img[i]).base64_word)
            report = FAILURE;

        fprintf(obj_file->file_ptr, "%s\n", (*data_img[i]).base64_word);

    }

    return report;
}

/**
 * Write the symbol information to the specified file context,
 * which can be an entry or extern file.
 *
 * @param ctx The file context for writing symbol information.
 * @param sym The symbol to write.
 * @return The status of the operation (NO_ERROR or FAILURE).
 */
status write_symbol_info(file_context *ctx, symbol *sym) {
    if (!ctx)
        return FAILURE;

    fprintf(ctx->file_ptr, "%s\t%d\n", sym->label, sym->address_decimal);

    return NO_ERROR;
}

/**
 * Cleanup and remove the output files associated with the file contexts.
 *
 * @param contexts The array of file context pointers.
 */
void cleanup_output_files(file_context ***contexts) {
    int i;

    for (i = 0; i < OUTPUT_NUM_FILES; i++) {
        if (!*contexts[i]) continue;
        fclose((*contexts[i])->file_ptr);
        remove((*contexts[i])->file_name);
        free_file_context(contexts[i]);
    }
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
            free_symbol_table(&symbol_table, &symbol_count);
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