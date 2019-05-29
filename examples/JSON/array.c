#include "json.h"
#include <stdio.h>

// Print an array.
void jnPrintArray(jnArray array) {
  putchar('[');
  bool firstTime = true;
  jnValue value;
  jnForeachArrayValue(array, value) {
    if (!firstTime) {
      printf(", ");
    }
    firstTime = false;
    jnPrintValue(value);
  } jnEndArrayValue;
  putchar(']');
}
