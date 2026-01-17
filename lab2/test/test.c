#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

void ReadFloat(float *ptr);
void FloatListInput(float *ptr, unsigned int sz);
void FloatListFindMinIdx(float *ptr, unsigned int *mnidx, unsigned int sz);
void FloatSelectionSort(float *ptr, unsigned int sz);
void KeleSortFloatList();
void generate_and_find_kth();
int main() {
  /*
  float f;
  ReadFloat(&f);
  printf("%f",f);
  */
  /*
  float list[5];
  printf("Enter 5 float: ");
  FloatListInput(list, 5);
  for(int i = 0;i< 5;i++){
    printf("%f ",list[i]);
  }
  printf("\n");
  unsigned int mnidx;
  FloatListFindMinIdx(list, &mnidx, 5);
  printf("Min of list is %f ",list[mnidx]);
  */
  /*
  FloatSelectionSort(list,5);
  for(int i = 0;i< 5;i++){
    printf("%f ",list[i]);
  }
  */
  KeleSortFloatList();
  /*
    test input generation
  for(int i = 0;i < 100;i++){
    generate_and_find_kth();    
  }
  printf("\n");
  */

  return 0;
}
void generate_and_find_kth() {
  int n, k;

  srand(time(NULL));

  n = (rand() % 16) + 5;

  k = (rand() % n) + 1;

  printf("%d %d\n",n,k);
  float *arr = (float *)malloc(n * sizeof(float));
  for (int i = 0; i < n; i++) {
    arr[i] = ((float)rand() / (float)(RAND_MAX)) * 100.0f;
    printf("%.2f ", arr[i]);
  }
  printf("\n");

  for (int i = 0; i < n - 1; i++) {
    int max_idx = i;
    for (int j = i + 1; j < n; j++) {
      if (arr[j] > arr[max_idx]) {
        max_idx = j;
      }
    }
    float temp = arr[max_idx];
    arr[max_idx] = arr[i];
    arr[i] = temp;
  }

  int target_idx = n - k;
  printf("%f\n", arr[target_idx]);

  free(arr);
}
