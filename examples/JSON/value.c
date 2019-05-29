#include "json.h"

// Create a value of the given type.
jnValue jnValueCreate(jnValueType type) {
  jnValue value = jnValueAlloc();
  jnValueSetType(value, type);
  return value;
}

// Create an object value.
jnValue jnObjectValueCreate(jnObject object) {
  jnValue value = jnValueCreate(JN_VAL_OBJECT);
  jnValueSetObject(value, object);
  return value;
}

// Create an array value.
jnValue jnArrayValueCreate(jnArray array) {
  jnValue value = jnValueCreate(JN_VAL_ARRAY);
  jnValueSetArray(value, array);
  return value;
}

// Create a string value.
jnValue jnStringValueCreate(utSym string) {
  jnValue value = jnValueCreate(JN_VAL_STRING);
  jnValueSetString(value, string);
  return value;
}

// Create a number value.
jnValue jnNumberValueCreate(double number) {
  jnValue value = jnValueCreate(JN_VAL_NUMBER);
  jnValueSetNumber(value, number);
  return value;
}

// Print the valiue.
void jnPrintValue(jnValue value) {
  switch (jnValueGetType(value)) {
    case JN_VAL_STRING:
      printf("\"%s\"", utSymGetName(jnValueGetString(value)));
      break;
    case JN_VAL_NUMBER:
      printf("%g", jnValueGetNumber(value));
      break;
    case JN_VAL_OBJECT:
      jnPrintObject(jnValueGetObject(value));
      break;
    case JN_VAL_ARRAY:
      jnPrintArray(jnValueGetArray(value));
      break;
    case JN_VAL_TRUE:
      printf("true");
      break;
      break;
    case JN_VAL_FALSE:
      printf("false");
      break;
    case JN_VAL_NULL:
      printf("null");
      break;
  }
}

