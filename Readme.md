repo : https://github.com/PranjalKhatri/IPLL
Minimal safety checks are done.
If asked for int, give a number in unsigned int range, else expect a seg fault. ¯\\_(ツ)_/¯

# Building
## For testing with C
```bash
# in the corresponding lab directory
make MODE=test
./bin/test

```
## For standalone build 
This will be done using the asm file in root directory.
for lab1 setA:
```bash
nasm -f elf32 230101077_seta.asm
ld -m elf_i386 230101077_seta.o
./a.out
```

for lab1 setB1:
```bash
nasm -f elf32 230101077_setb1.asm
gcc -m32 230101077_setb1.o
./a.out

```
for lab1 setB2:
```bash
nasm -f elf32 230101077_setb2.asm
ld -m elf_i386 230101077_setb2.o
./a.out

```
