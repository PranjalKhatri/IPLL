#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define TEST_FILE "test_input.txt"
#define TEMP_INPUT "current_test.txt"
#define NUM_TESTS 100

int main() {
    FILE *fp = fopen(TEST_FILE, "r");
    if (!fp) {
        perror("Could not open test_input.txt");
        return 1;
    }

    int passed = 0;
    char n_k_line[256];
    char array_line[4096]; 
    char expected_line[256];

    for (int t = 1; t <= NUM_TESTS; t++) {
        if (!fgets(n_k_line, sizeof(n_k_line), fp)) break;
        if (!fgets(array_line, sizeof(array_line), fp)) break;
        if (!fgets(expected_line, sizeof(expected_line), fp)) break;

        float expected = atof(expected_line);
        FILE *tmp = fopen(TEMP_INPUT, "w");
        fprintf(tmp, "%s%s", n_k_line, array_line);
        fclose(tmp);

        char command[512];
        snprintf(command, sizeof(command), "./bin/test < %s", TEMP_INPUT);
        
        FILE *asm_pipe = popen(command, "r");
        if (!asm_pipe) {
            perror("Failed to run assembly program");
            return 1;
        }

        char buffer[128];
        float actual = 0.0f;
        while (fgets(buffer, sizeof(buffer), asm_pipe) != NULL) {
            if (sscanf(buffer, "%f", &actual) == 1) {
            }
        }
        pclose(asm_pipe);

        if (fabs(actual - expected) < 0.01f) {
            passed++;
        } else {
            printf("Test %d FAILED | Expected: %f, Got: %f\n", t, expected, actual);
        }
    }

    fclose(fp);
    remove(TEMP_INPUT);
    printf("\n--- FINAL RESULTS ---\n");
    printf("Passed: %d/%d\n", passed, NUM_TESTS);

    return (passed == NUM_TESTS) ? 0 : 1;
}
