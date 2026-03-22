
# Lexer for nanoC (Assignment 3)

* **Student**: Pranjal Khatri
* **Roll Number**: 230101077

---

##  Overview

This project implements a **lexical analyzer (lexer)** for a subset of the C language, referred to as **nanoC**, using **Flex**.

The lexer:

* Tokenizes the input source file
* Identifies keywords, identifiers, constants, strings, punctuators
* Maintains a **symbol table** for declared identifiers
* Tracks **line and column numbers**
* Outputs tokens and symbol table into separate files

---

##  Features

###  Token Recognition

The lexer recognizes:

* **Keywords**: `int`, `float`, `if`, `while`, etc.
* **Identifiers**
* **Integer constants**
* **Floating constants**
* **Character constants**
* **String literals**
* **Punctuators**
* **Comments** (single-line and multi-line)

---

###  Symbol Table

* Stores identifiers declared after **type keywords**
* Handles multiple declarations:

  ```c
  int a, b, c;
  ```
* Prevents duplicate entries
* Stores:

  * Identifier name
  * Line number
  * Column number

---

###  Line & Column Tracking

* Maintains accurate **line numbers**
* Tracks **column position**
* Handles:

  * Multi-line comments
  * Newlines correctly

---

###  Error Handling

* Unknown tokens are reported
* Errors include:

  * Line number
  * Column number

---

##  File Structure

```
.
├── a3_230101077.l           # Flex lexer specification
├── hash.c                   # Hash table implementation
├── hash.h                   # Hash table header
├── Makefile                 # Build automation
├── Readme.md                # Documentation
├── a3_230101077_test.nc     # Test input file
├── a3_230101077_token.txt   # Token output
├── a3_230101077_st.txt      # Symbol table output
```

---

##  Compilation & Execution

###  Using Makefile (Recommended)

```bash
make
make run
```

###  Clean build files

```bash
make clean
```

---

###  Manual Compilation

```bash
flex a3_230101077.l
gcc lex.yy.c hash.c -o lexer
./lexer a3_230101077_test.nc
```

If you don't have flex symlink to the flex in the bin directory of provided flex folder.
---

##  Output Files

### 1. Token File

**File:** `a3_230101077_token.txt`

Format:

```
<KEYWORD, int >
<IDENTIFIER, x >
<PUNCTUATOR, ; >
```

Includes line markers:

```
----------<LINE 1>----------
```

---

### 2. Symbol Table File

**File:** `a3_230101077_st.txt`

Format:

```
SYMBOL          LINE       COL
----------------------------------------
x               1          5
y               2          7
```

---

##  Design Details

###  Identifier Handling Logic

* After encountering a type keyword (`int`, `float`, etc.)
* The lexer expects identifiers
* Commas propagate the expectation:

  ```c
  int a, b, c;
  ```

---

###  Comment Handling

* **Single-line comments**:

  ```c
  // comment
  ```
* **Multi-line comments**:

  ```c
  /* comment */
  ```
* Properly updates line and column counters

---

###  Memory Management

* Symbol table entries are dynamically allocated
* Custom free function ensures:

  * No memory leaks
  * Proper cleanup of all entries

---

##  Limitations

* Does not fully parse complex declarations:

  ```c
  int a = 5, b;     // initialization partially handled
  ```
* No scope handling (global only)
* No type storage in symbol table

---
