#include <stdint.h>
#include <stdio.h>
void ReadUInt(unsigned int *ref);
int ListFindMax(int *lst, unsigned int cnt);
void PrintString(char*ptr,unsigned int cnt);
void ListMostRepeating(void);

int arr[] = {345, 762, 1345, 6714, 1457, 45, 5};
int arrayMax(int *arr, int sz) {
  int mx = INT32_MIN;
  for (int i = 0; i < sz; i++) {
    if (mx < arr[i])
      mx = arr[i];
  }
  return mx;
}
int test() {
  // unsigned int a;
  // ReadUInt(&a);
  // printf("Read unsigned int %d\n", a);
  // int listmax = ListFindMax(arr, sizeof(arr) / sizeof(arr[0]));
  // if (listmax != arrayMax(arr, sizeof(arr) / sizeof(arr[0]))) {
  //   return 1;
  // }
  // PrintString("Printing string.",17);
    ListMostRepeating();
  return 0;
}
