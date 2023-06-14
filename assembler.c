/* Assembler Project in 20465 - System Programming Laboratory
 * @author Bar Toplian - 323869065- bar.toplian@gmail.com
 */

#include <stdio.h>
#include <stdlib.h>
#include "errors.h"
#include "utils.h"
#include "preprocessor.h"
#include "passes.h"

#define HANDLE_STATUS(file, code) if ((code) == ERR_MEM_ALLOC) { \
    handle_error(code, (file)); \
    if (file) free_file_context(&(file)); \
    return ERR_MEM_ALLOC; \
    }

#define CHECK_ERROR_CONTINUE(report, file) \
    if ((report) != NO_ERROR) {      \
    handle_error(ERR_FOUND_ASSEMBLER, (file));\
        continue;                    \
    }                                      \
else \
break;

status preprocess_file(const char* file_name, file_context** dest , int index, int max);

/** TODO: remove after testing!! */
void test() {
    char *lines[3] = {
            "LENGTH: .data 6,-9,15",
            "K: .data 22",
            "MLAB: .data K"
    };
    char label[36];
    int i;
    size_t size;
    status code = NO_ERROR;
    symbol *sym = NULL;
    file_context *fc = create_file_context("as", ASSEMBLY_EXT, FILE_EXT_LEN, FILE_MODE_READ, &code);
    for (i = 0; i < 3; i++) {
        code = NO_ERROR;
        size = get_word(&lines[i],label,NORMAL);
        sym = declare_label(fc, label, size);
        process_line_w_label(fc, lines[i], sym, &code);
    }

}
/** TODO: remove after testing!! */

int main(int argc, char *argv[]) {
    int i;
    status report;
    file_context *dest_am = NULL;

    if (argc == 1) {
        handle_error(FAILURE);
        exit(FAILURE);
    }
    test();
    return 0;

    for (i = 1; i < argc; i++) {
        report = preprocess_file(argv[i], &dest_am, i, argc - 1);
        if (report != NO_ERROR && assembler_first_pass(&dest_am))
            handle_error(ERR_FIRST_PASS, i, argc - 1, argv[i]);
        else
            handle_progress(FIRST_PASS_OK, i, argc - 1, argv[i]);
        CHECK_ERROR_CONTINUE(report, argv[i]);
    }

    /* TODO: update goodbye message */
    return 0;
}

/**
 * Processes the input source file for assembler preprocessing.
 *
 * Reads the source file and process it accordingly by the assembler passes and the preprocessor.

 *
 * @param file_name     The name of the input source file to process.
 * @param dest          Pointer to the destination file_context struct.
 * @param index         The index of the file being processed.
 * @param max           The total number of files to be processed.
 *
 * @return The status of the file processing.
 * @return NO_ERROR if successful, or FAILURE if an error occurred.
 */
status preprocess_file(const char* file_name, file_context** dest , int index, int max) {
    file_context *src = NULL;
    status code = NO_ERROR;

    src = create_file_context(file_name, ASSEMBLY_EXT, FILE_EXT_LEN, FILE_MODE_READ, &code);
    HANDLE_STATUS(src, code);

    handle_progress(OPEN_FILE, src);

    *dest = create_file_context(file_name, PREPROCESSOR_EXT,  FILE_EXT_LEN, FILE_MODE_WRITE, &code);
    HANDLE_STATUS(*dest, code);

    code = assembler_preprocessor(src, *dest);

    if (src) free_file_context(&src);

    if (code != NO_ERROR) {
        handle_error(ERR_PRE, index, max, file_name);
        free_file_context(dest);
        return FAILURE;
    } else {
        handle_progress(PRE_FILE_OK, *dest, index, max);
        return NO_ERROR;
    }
}