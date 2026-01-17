#include <stdint.h>
#include <stdio.h>
void ReadUInt(unsigned int *ref);
int ListFindMax(int *lst, unsigned int cnt);
void PrintString(char *ptr, unsigned int cnt);
void ListInput(int *ptr, unsigned int sz);
void ZeroMemory(unsigned char *ptr, unsigned int bytesz);
void PrintUInt(unsigned int a);
void ListMostRepeating();
// void ListMostRepeating(void);

int arr[] = {345, 762, 1345, 671004, 1457, 45, 50000};
int arrayMax(int *arr, int sz) {
  int mx = INT32_MIN;
  for (int i = 0; i < sz; i++) {
    if (mx < arr[i])
      mx = arr[i];
  }
  return mx;
}
int main() {
  /*
  unsigned int a;
  ReadUInt(&a);
  printf("Read unsigned int %d\n", a);
  int listmax = ListFindMax(arr, sizeof(arr) / sizeof(arr[0]));
  if (listmax != arrayMax(arr, sizeof(arr) / sizeof(arr[0]))) {
    return 1;
  }
  PrintString("Printing string.",17);
  int inarr[5];
  printf("Enter 5 numbers\n");
  ListInput(inarr,5);
  for(int i = 0;i < 5;i++){
    printf("%d ",inarr[i]);
  }
  int rarr[5]={1,2,3,4,5};
  for (int i = 0; i < 5; i++) {
    printf("%d ", rarr[i]);
  }
  printf("\n");
  ZeroMemory((unsigned char*)rarr, sizeof(rarr));
  for (int i = 0; i < 5; i++) {
    printf("%d ", rarr[i]);
  }
  PrintUInt(10);
  printf("\n");
  PrintUInt(0);
  printf("\n");
  PrintUInt(239);
  */
  ListMostRepeating();
  return 0;
}
