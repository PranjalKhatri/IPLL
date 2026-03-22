// =======================
// KEYWORDS + IDENTIFIERS
// =======================

int x;
float y;
double z;
char c;
unsigned long ul;
signed short ss;
void func;

int a, b, c1;   // multiple declarations (important test)
float f1, f2, f3;

// =======================
// IDENTIFIER EDGE CASES
// =======================

int _valid;
int valid123;
int __;
int a_b_c;

int 1invalid;   // should trigger UNKNOWN or error

// =======================
// INTEGER CONSTANTS
// =======================

int i1 = 0;
int i2 = 123;
int i3 = 99999;

// =======================
// FLOAT CONSTANTS
// =======================

float f = 3.14;
float f_2 = .25;
float f_3 = 10.;

// =======================
// CHAR CONSTANTS
// =======================

char ch1 = 'a';
char ch2 = '\n';
char ch3 = '\\';
char ch4 = '\'';   // escaped single quote
char invalchar = 'abc';

// =======================
// STRING LITERALS
// =======================

char *s1 = "hello";
char *s2 = "line\nbreak";
char *s3 = "escaped \\ \" test";

// =======================
// PUNCTUATORS
// =======================

x++;
y--;
x += 10;
y -= 5;
z *= 2;
z /= 3;
z %= 2;

if (x == y && y != z || x < z) {
    x = y;
}

a = b ? c1 : x;

arr[i] = 10;
ptr->field;

<< >> <= >= == != && || + - * / % & | ^ ~ ! = < > ? : ; , . ... #

// =======================
// COMMENTS
// =======================

// single line comment

/*
   multi-line comment
   spanning multiple lines
*/

// nested-like (should not actually nest)

// =======================
// MIXED CODE
// =======================

int main() {
    int count = 10;
    float avg = 0.0;

    for (int i = 0; i < count; i++) {
        avg += i;
    }

    avg = avg / count;

    if (avg > 5.0) {
        return 1;
    } else {
        return 0;
    }
}

// =======================
// ERROR / UNKNOWN TOKENS
// =======================

@
$
`
