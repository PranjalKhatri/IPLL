
---

**Roll No.:** 230101077
**Name:** Pranjal

The code is **self-documenting** (self-explanatory variable and function names),
however **comments are provided where necessary**.

It is **recommended to read the code from the corresponding folders** for better structure and clarity,
instead of reading everything from a single combined file.

---

## Standalone Build

### Two-Pass Assembler

```bash
gcc -o 230101077_twopass 230101077_twopass.c
./230101077_twopass input.txt output.txt
```

### One-Pass Assembler

```bash
gcc -o 230101077_onepass 230101077_onepass.c
./230101077_onepass input.txt output.txt
```

---

## Using `make`

### One-Pass

```bash
cd 1Pass
make
./assembler input.txt output.txt
```

### Two-Pass

```bash
cd 2Pass
make
./assembler input.txt output.txt
```

---

#### NOTE:
in windows *mingw32-make* is used instead of *make*.
and use 
```bash
assembler.exe input.txt output.txt
```
