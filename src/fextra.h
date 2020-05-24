#include <stdio.h>

size_t fsize(FILE* f);

// READ FUNCTIONS //

// reading
short fgetsh(FILE* f);
int fgeti(FILE* f);
long long fgetll(FILE* f);

// reading float/double
float fgetf(FILE* f);
double fgetd(FILE* f);

// WRITE FUNCTIONS //

// writing
int fputsh(short c, FILE* f);
int fputi(int c, FILE* f);
int fputll(long long c, FILE* f);

// writing float/double
int fputf(float c, FILE* f);
int fputd(double c, FILE* f);
