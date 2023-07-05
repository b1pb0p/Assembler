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
    }

status preprocess_file(const char* file_name, file_context** dest , int index, int max);

/* TODO: REMOVE!!!!!!!!!!!!!!! */
void test_out(file_context *src) { /*TODO: REMOVE */
    status report;

    printf("\n***** TESTING *****\n");
    printf("\n***** START: write_extern_to_stream *****\n");
    report = write_extern_to_stream(src, stdout);
    printf("***** END: write_extern_to_stream - STATUS: %s *****\n", report != NO_ERROR ? "Failed" : "Passed");
    printf("\n***** START: write_entry_to_stream *****\n");
    report = write_entry_to_stream(src, stdout);
    printf("***** END: write_entry_to_stream - STATUS: %s *****\n", report != NO_ERROR ? "Failed" : "Passed");
    printf("\n***** START: print data_images *****\n");
    report = write_data_img_to_stream(src, stdout);
    printf("\n***** END: print data_images - STATUS: %s *****\n", report != NO_ERROR ? "Failed" : "Passed");
}

void test() { /** TODO: remove after testing!! */
#define number_of_lines 4
    symbol *sym = NULL;
    FILE* previousStderr = freopen("debug/error.log", "w", stderr);
    char *data_lines[number_of_lines] = {
            "LENGTH: .data XYZ , -9,15",
            ".data LENGTH,9, 7, 9",
            "XYZ: .data 51",
            ".data 6,-9,15, 22"

    };

    char *str_lines[number_of_lines] = {
            "STR: .string \"abcdef\"",
            ".string HELLO",
            "LONG: .string \"bar\"",
            ".string LONG, STR, \"r\""
    };

    char *ent_lines[number_of_lines] = {
            "VAR: .entry LENGTH, XYZ",
            ".entry LONG"
    };

    char *ext_lines[number_of_lines] = {
            "VAL: .extern PI",
            ".extern HELLO"
    };

    char label[36];
    char temp[36];
    int i;
    size_t size;
    status code = NO_ERROR;
    file_context *fc = create_file_context("as", ASSEMBLY_EXT, FILE_EXT_LEN, FILE_MODE_READ, &code);

    code = NO_ERROR;

    for (i = 0; i < number_of_lines; i++) {
        if (i == 0)
            printf("\n %d line \n",fc->lc);
        else
            printf("\n %d line \n",++fc->lc);

        if (i % 2 == 0) {
            size = get_word(&data_lines[i], label, SPACE);
            printf("Processing ... %s\n",data_lines[i]);
            get_word(&data_lines[i], temp, SPACE);
            (void)declare_label(fc, label, size, &code);
            process_data(fc, label, data_lines[i], &code);
        }
        else {
            (void)get_word(&data_lines[i], label, SPACE);
            printf("Processing ... %s\n",data_lines[i]);
            process_data(fc, NULL, data_lines[i], &code);
        }
    }

    for (i = 0; i < number_of_lines; i++) {
        printf("\n %d line \n", ++fc->lc);

        if (i % 2 == 0) {
            size = get_word(&str_lines[i], label, SPACE);
            printf("Processing ... %s\n", str_lines[i]);
            get_word(&str_lines[i], temp, SPACE);
            (void) declare_label(fc, label, size, &code);
            process_string(fc, label, str_lines[i], &code);
        } else {
            (void) get_word(&str_lines[i], label, SPACE);
            printf("Processing ... %s\n", str_lines[i]);
            process_string(fc, NULL, str_lines[i], &code);
        }
    }

    for (i = 0; i < number_of_lines - 2; i++) {
        printf("\n %d line \n", ++fc->lc);

        if (i == 0) {
            size = get_word(&ent_lines[i], label, SPACE);
            printf("Processing ... %s\n", ent_lines[i]);
            get_word(&ent_lines[i], temp, SPACE);
            (void) declare_label(fc, label, size, &code);
            process_directive(fc, ENTRY, label, ent_lines[i], &code);
        } else if (i == 1) {
            (void) get_word(&ent_lines[i], label, SPACE);
            printf("Processing ... %s\n", ent_lines[i]);
            process_directive(fc, ENTRY, NULL, ent_lines[i], &code);
        }
    }

    for (i = 0; i < number_of_lines - 2; i++) {
        printf("\n %d line \n", ++fc->lc);

        if (i == 0) {
            size = get_word(&ext_lines[i], label, SPACE);
            printf("Processing ... %s\n", ext_lines[i]);
            get_word(&ext_lines[i], temp, SPACE);
            (void) declare_label(fc, label, size, &code);
            process_directive(fc, EXTERN, label, ext_lines[i], &code);
        } else if (i == 1) {
            (void) get_word(&ext_lines[i], label, SPACE);
            printf("Processing ... %s\n", ext_lines[i]);
            process_directive(fc, EXTERN, NULL, ext_lines[i], &code);
        }
    }
    test_out(fc);
    freopen("/dev/tty", "w", stderr);
}

void cmd_test() {
    FILE* previousStderr = freopen("debug/error.log", "w", stderr);
    char *cmd_lines[number_of_lines] = {
            "prn -5",
            "END: stop",
            "LENGTH: .data -9,15",
            "MAIN: mov @r3 ,LENGTH"
    };
    char label[36];
    char temp[36];
    int i;
    size_t size;
    status code = NO_ERROR;
    file_context *fc = create_file_context("as", ASSEMBLY_EXT, FILE_EXT_LEN, FILE_MODE_READ, &code);
    code = NO_ERROR;

    printf("Processing ... %s\n",cmd_lines[0]);
    size = get_word(&cmd_lines[0], temp, SPACE);
    handle_one_operand(fc,PRN, NULL, cmd_lines[0], &code);
    fc->lc++;

    printf("Processing ... %s\n",cmd_lines[1]);
    size = get_word(&cmd_lines[1], label, SPACE);
    (void)declare_label(fc, label, size, &code);
    size = get_word(&cmd_lines[1], temp, SPACE);
    handle_no_operands(fc, STOP, label, cmd_lines[1], &code);

    fc->lc++;
    printf("Processing ... %s\n",cmd_lines[2]);
    size = get_word(&cmd_lines[2], label, SPACE);
    (void)declare_label(fc, label, size, &code);
    get_word(&cmd_lines[2], temp, SPACE);
    process_data(fc, label, cmd_lines[2], &code);

    fc->lc++;
    printf("Processing ... %s\n",cmd_lines[3]);
    size = get_word(&cmd_lines[3], label, SPACE);
    (void)declare_label(fc, label, size, &code);
    size = get_word(&cmd_lines[3], temp, SPACE);
    handle_two_operands(fc, MOV, label, cmd_lines[3], &code);
    test_out(fc);



    freopen("/dev/tty", "w", stderr);
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
   // test();
    // cmd_test();
    //return 0;

    for (i = 1; i < argc; i++) {
        report = preprocess_file(argv[i], &dest_am, i, argc - 1);
        CHECK_ERROR_CONTINUE(report, argv[i]);
        if (assembler_first_pass(&dest_am) != NO_ERROR)
            handle_error(ERR_FIRST_PASS, i, argc - 1, argv[i]);
        else
            handle_progress(FIRST_PASS_OK, i, argc - 1, argv[i]);
        CHECK_ERROR_CONTINUE(report, argv[i]);
    }

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

    *dest = create_file_context(file_name, PREPROCESSOR_EXT, FILE_EXT_LEN, FILE_MODE_WRITE_PLUS, &code);
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