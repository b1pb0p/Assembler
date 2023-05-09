/* errors.c
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "errors.h"

/* Status messages */
const char *ERR_MSG[ERR_MSG_LEN] = {
        "Assembly completed without errors. Output files generated.",
        "Assembly terminated with errors. No output files generated.",
        "Assembler - Memory allocation error.",
        "Error: Unable to open file %s",
        "%s - Invalid opcode on line %d.",
        "%s - Invalid operand on line %d.",
        "%s - Missing operand on line %d.",
        "%s - Too many operands on line %d.",
        "%s - Illegal use of operand on line %d.",
        "%s - Label used before definition on line %d.",
        "%s - Missing label on line %d.",
        "%s - Duplicate label on line %d.",
        "%s - Invalid register used on line %d.",
        "%s - Extraneous text on line %d.",
        "%s - Missing '@' symbol on line %d.",
        "%s - Missing ';' symbol after label declaration on line %d.",
        "%s - Missing ',' symbol on line %d.",
        "%s - Line too long on line %d. Mustn't exceed 80 characters.",
        "%s - Invalid macro name on line %d.",

};


void handle_error(int err_code, ...) {
    va_list args;
    if (err_code == NO_ERROR) {
        printf("\n---------------------STATUS---------------------\n");
        printf("%s\n", ERR_MSG[err_code]);
    }
    else {
        fprintf(stderr, "\n---------------------STATUS---------------------\n");

        // Error messages that require additional arguments
        va_start(args, err_code);

        if (err_code == ERR_OPEN_FILE) {
            char* error_file = va_arg(args, char*);
            fprintf(stderr, ERR_MSG[err_code], error_file);
        }
        else if (err_code == ERR_MEM_ALLOC) {
            fprintf(stderr, ERR_MSG[err_code]);
        }
        else {
            file_context* fc = va_arg(args, file_context*);
            char* error_file = fc->file_name;
            int error_line = va_arg(args, int);
            fprintf(stderr, ERR_MSG[err_code], error_file, error_line);
        }
        va_end(args);
    }
    fprintf(stderr, "\n");
}