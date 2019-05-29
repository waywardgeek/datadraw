#include "jndatabase.h"

// Main functions.
void jnStart(void);
void jnStop(void);
jnValue jnParse(FILE *file);

// Value methods.
jnValue jnValueCreate(jnValueType type);
jnValue jnObjectValueCreate(jnObject object);
jnValue jnArrayValueCreate(jnArray array);
jnValue jnStringValueCreate(utSym string);
jnValue jnNumberValueCreate(double number);
void jnPrintValue(jnValue value);

// Object methods.
static inline jnObject jnObjectCreate(void) { return jnObjectAlloc(); }
void jnPrintObject(jnObject object);

// Array methods.
static inline jnArray jnArrayCreate(void) { return jnArrayAlloc(); }
void jnPrintArray(jnArray array);

// String methods.
// TODO: Write an API to escape/unescape strings.

// Number methods.
