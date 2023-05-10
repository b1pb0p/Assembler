/* errors.c
* Error Handling
* @author Bar Toplian - 323869065- bar.toplian@gmail.com
*/
#include <stdio.h>
#include <stdarg.h>
#include "errors.h"
#include "utils.h"

/* Status messages */
const char *MSG[MSG_LEN] = {
        "Assembly completed without errors. Output files generated.",
        "Assembly terminated with errors. No output files generated.",
        "Preprocessor - The preprocessing stage is finished.",
        "First Pass - The first pass stage is finished.",
        "Second Pass - The second pass stage is finished.",
        "Assembler - Memory allocation error.",
        "Assembler - Unable to open file - %s",
        "Assembler - File opened successfully - %s.",
        "Preprocessor - Output file successfully generated - %s.",
        "Preprocessor - No output file generated - %s.",
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
        "%s - Duplicate macro name on line %d.",
        "%s - Missing 'endmcro' on line %d.",

};


void handle_error(int code, ...) {
    va_list args;
    if (code == NO_ERROR) {
        printf("TERMINATED ->\t");
        printf("%s\n", MSG[code]);
    }
    else {
        fprintf(stderr, "ERROR ->\t");
        file_context* fc;
        // Error messages that require additional arguments
        va_start(args, code);
        fc = va_arg(args, file_context*);
        if (code == ERR_OPEN_FILE) {
            fprintf(stderr, MSG[code], fc->file_name);
        }
        else if (code >= ERR_MEM_ALLOC) {
            fprintf(stderr, "%s", MSG[code]);
        }
        else {
            char* error_file = fc->file_name;
            int error_line = va_arg(args, int);
            fprintf(stderr, MSG[code], error_file, error_line);
        }
        va_end(args);
    }
    fprintf(stderr, "\n");
}

void handle_progress(int code, ...) {
    va_list args;
    file_context* fc;
    printf("PROGRESS ->\t");

    va_start(args, code);
    fc = va_arg(args, file_context*);
    if (!fc) handle_error(FAILURE);
    else if (code <= PRE_FILE_OK)
        printf(MSG[code], fc->file_name);
    else
        printf(MSG[code], fc->file_name, fc->lc);
    va_end(args);
    printf("\n");
}
