// Executable program that parses, and then prints, a JSON value.
#include "json.h"
#include <stdio.h>


// Initialize the JSON dateabase.
void jnStart(void) {
  utStart();
  jnDatabaseStart();
}

// Clean up memory used by the JSON database.
void jnStop(void) {
  jnDatabaseStop();
  utStop(false);
}

// Read input string from stdin, parse it, print the resulting value, and exit.
int main(int argc, char **argv) {
  if (argc != 1) {
    printf("Usage: json\n    Reads JSON string from STDIN, then prints it.\n");
    return 1;
  }
  jnStart();
  jnValue value = jnParse(stdin);
  jnPrintValue(value);
  jnValueDestroy(value);  // Not actually needed: jnStop frees all memory.
  jnStop();
  return 0;
}
