// See https://www.json.org/ for the grammar rules.
#include "json.h"
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

#define JN_MAX_NUMBER_LEN 1024

// Forward declarations for recursion.
static jnValue parseValue(FILE* file);

// Read past whitespace, and return the first non-whitespace char.
static int getNonSpaceChar(FILE *file) {
  int c = getc(file);
  while (isspace(c)) {
    c = getc(file);
  }
  return c;
}

// Append a character to the temp char buffer.  Resize it if required.
static inline int getNextCharAndReallocBuffer(FILE *file, char *buf, uint32 *allocated, uint32 *pos) {
  int c = getc(file);
  if (*pos == *allocated) {
    char *newBuf = utMakeString(*allocated << 1);
    memcpy(newBuf, buf, *allocated);
    *allocated <<= 1;
    buf = newBuf;
  }
  buf[(*pos)++] = c;
  return c;
}

// Parse a string.  Check that it is legal, but don't un-escape it.
static utSym parseString(FILE *file) {
  uint32 allocated = 1024;
  uint32 pos = 0;
  char *buf = utMakeString(allocated);
  int c = getNextCharAndReallocBuffer(file, buf, &allocated, &pos);
  while (c != '"') {
    if (c == EOF) {
      utExit("EOF in string");
    } else if (c == '\\') {
      c = getNextCharAndReallocBuffer(file, buf, &allocated, &pos);
      switch (c) {
        case '"': case '\\': case '/': case 'b': case 'f': case 'n': case 'r': case 't': case 'u':
            break;
        default:
          utExit("Invalid escape sequence");
      }
    }
    c = getNextCharAndReallocBuffer(file, buf, &allocated, &pos);
  }
  buf[--pos] = '\0';
  return utSymCreate(buf);
}

// Get the next character and add it to the buffer.
static inline int getNextChar(FILE *file, char *buf, uint32 *pos) {
  if (*pos == JN_MAX_NUMBER_LEN) {
    utExit("Number too long");
  }
  int c = getc(file);
  buf[(*pos)++] = c;
  return c;
}

// Parse a number.  Return a double.
static double parseNumber(FILE *file, char firstChar) {
  char buf[JN_MAX_NUMBER_LEN + 1]; // One extra for terminating '\0'.
  uint32 pos = 0;
  buf[pos++] = firstChar;
  if (firstChar == '-') {
    firstChar = getNextChar(file, buf, &pos);
    if (!isdigit(firstChar)) {
      utExit("Numbers must start with a digit");
    }
  }
  // Parse integer portion.
  int c = getNextChar(file, buf, &pos);
  if (firstChar != '0') {
    while (isdigit(c)) {
      c = getNextChar(file, buf, &pos);
    }
  }
  if (c == '.') {
    // Parse fractional portion.
    c = getNextChar(file, buf, &pos);
    if (!isdigit(c)) {
      utExit("Digit is required after dot");
    }
    do {
      c = getNextChar(file, buf, &pos);
    } while (isdigit(c));
  }
  if (c == 'e' || c == 'E' || c == '+' || c == '-') {
    // Parse exponent.
    if (c == '+' || c == '-') {
      c = getNextChar(file, buf, &pos);
    }
    if (!isdigit(c)) {
      utExit("Exponent cannot be empty");
    }
    do {
      c = getNextChar(file, buf, &pos);
    } while (isdigit(c));
  }
  ungetc(c, file);
  buf[--pos] = '\0';
  // Now parse it.
  char *endptr;
  double value = strtod(buf, &endptr);
  if (endptr == NULL && *endptr != '\0') {
    utExit("Could not parse number");
  }
  return value;
}

// Create an entry.
static jnEntry entryCreate(jnObject object, utSym key, jnValue value) {
  jnEntry entry = jnEntryAlloc();
  jnEntrySetSym(entry, key);
  jnEntrySetValue(entry, value);
  jnObjectAppendEntry(object, entry);
  return entry;
}

// Parse an entry.  |firstChar| should be '"'.
static jnEntry parseEntry(FILE *file, jnObject object, int firstChar) {
  if (firstChar != '"') {
    utExit("Keys must be strings");
  }
  utSym key = parseString(file);
  int c = getNonSpaceChar(file);
  if (c != ':') {
    utExit("Expected :");
  }
  jnValue value = parseValue(file);
  return entryCreate(object, key, value);
}

// Parse an object, without the preceeding '{', but including the final '}'.
static jnObject parseObject(FILE *file) {
  jnObject object = jnObjectCreate();
  char c = getNonSpaceChar(file);
  while (c != '}') {
    jnEntry entry = parseEntry(file, object, c);
    jnObjectAppendEntry(object, entry);
    c = getNonSpaceChar(file);
    if (c == ',') {
      c = getNonSpaceChar(file);
      if (c == '}') {
        utExit("comma before }");
      }
    } else if (c != '}') {
      utExit("Missing comma separator");
    }
  }
  return object;
}

// Parse an array, without the preceeding '[', but including the final ']'.
static jnArray parseArray(FILE *file) {
  jnArray array = jnArrayCreate();
  char c = getNonSpaceChar(file);
  while (c != ']') {
    ungetc(c, file);
    jnValue value = parseValue(file);
    jnArrayAppendValue(array, value);
    c = getNonSpaceChar(file);
    if (c == ',') {
      c = getNonSpaceChar(file);
      if (c == ']') {
        utExit("comma before }");
      }
    } else if (c != ']') {
      utExit("Missing comma separator");
    }
  }
  return array;
}

// Compare the input to the string and give an error if they do not match.
static void expectString(FILE *file, char *string) {
  char c = *string++;
  while (c != '\0') {
    if (getc(file) != c) {
      utExit("Invalid input: should end with: %s", string);
    }
    c = *string++;
  }
}

// Parse a JSON value.
static jnValue parseValue(FILE* file) {
  int c = getNonSpaceChar(file);
  switch(c) {
    case '{': return jnObjectValueCreate(parseObject(file));
    case '[': return jnArrayValueCreate(parseArray(file));
    case '"': return jnStringValueCreate(parseString(file));
    case '0' ... '9': case '-': return jnNumberValueCreate(parseNumber(file, c));
    case 't':
      expectString(file, "rue");
      return jnValueCreate(JN_VAL_TRUE);
    case 'f':
      expectString(file, "alse");
      return jnValueCreate(JN_VAL_FALSE);
    case 'n':
      expectString(file, "ull");
      return jnValueCreate(JN_VAL_NULL);
    case EOF:
      return jnValueNull;
    default:
      utExit("Syntax error at char %c\n", c);
  }
  return jnValueNull;  // Can't get here.
}

// Parse a JSON string from a file.
jnValue jnParse(FILE *file) {
  return parseValue(file);
}
