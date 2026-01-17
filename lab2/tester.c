#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define NUM_TESTS 100

float selection_sort_and_get_kth(float* arr, int n, int k) {
    for (int i = 0; i < n - 1; i++) {
        int min_idx = i;
        for (int j = i + 1; j < n; j++) {
            if (arr[j] < arr[min_idx]) min_idx = j;
        }
        float temp = arr[i];
        arr[i] = arr[min_idx];
        arr[min_idx] = temp;
    }
    return arr[k - 1]; // k-th smallest
}

int main() {
    srand(time(NULL));
    int passed = 0;

    for (int t = 1; t <= NUM_TESTS; t++) {
        int n = (rand() % 20) + 5;
        int k = (rand() % n) + 1;
        float* arr = malloc(n * sizeof(float));

        FILE* input_file = fopen("test_input.txt", "w");
        fprintf(input_file, "%d %d\n", n, k);
        for (int i = 0; i < n; i++) {
            arr[i] = ((float)rand() / (float)RAND_MAX) * 100.0f;
            fprintf(input_file, "%f ", arr[i]);
        }
        fclose(input_file);

        float expected = selection_sort_and_get_kth(arr, n, k);

        FILE* pipe = popen("./bin/test < test_input.txt", "r");
        char buffer[128];
        float actual = 0.0f;
        
        while (fgets(buffer, sizeof(buffer), pipe) != NULL) {
            sscanf(buffer, "%f", &actual);
        }
        pclose(pipe);

        if (fabs(actual - expected) < 0.01f) {
            passed++;
            printf("expected %f, actual %f\n",expected,actual);
        } else {
            printf("Test %d FAILED: N=%d, K=%d | Expected: %f, Got: %f\n", t, n, k, expected, actual);
        }
        free(arr);
    }

    printf("\n--- RESULTS ---\n");
    printf("Passed: %d/%d\n", passed, NUM_TESTS);
    return (passed == NUM_TESTS) ? 0 : 1;
}
