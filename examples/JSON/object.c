#include "json.h"
#include <stdio.h>

// Print an object.
void jnPrintObject(jnObject object) {
  putchar('{');
  bool firstTime = true;
  jnEntry entry;
  jnForeachObjectEntry(object, entry) {
    if (!firstTime) {
      printf(", ");
    }
    firstTime = false;
    printf("\"%s\":", jnEntryGetName(entry));
    jnPrintValue(jnEntryGetValue(entry));
  } jnEndObjectEntry;
  putchar('}');
}
