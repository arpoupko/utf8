UTF-8 String Manipulation in C

This C file contains functions for manipulating UTF-8 encoded strings. These functions include operations such as encoding and decoding, len, and strcmp.
Functions

my_utf8_decode: Decodes a UTF-8 encoded string.

  int my_utf8_decode(char *input, char *output);

my_utf8_encode: Encodes a Unicode code point into UTF-8.

  int my_utf8_encode(char *input, char *output);

my_utf8_check: Checks the validity of a UTF-8 encoded string.

  bool my_utf8_check(const unsigned char* string);

my_utf8_strlen: Calculates the length of a UTF-8 encoded string in terms of characters.

  int my_utf8_strlen(const char *string);

my_utf8_charat: Returns the UTF-8 character at a specific index.

  char *my_utf8_charat(char *string, int index);

my_utf8_strcmp: Compares two UTF-8 strings.

  int my_utf8_strcmp(const char *string1, const char *string2);

is_utf8_start: Checks if a byte is the start of a UTF-8 character.

  bool is_utf8_start(unsigned char byte);

getCategory: Retrieves the ASCII category of a UTF-8 character.

  const char* getCategory(const char* utf8Char);

is_ascii_string: Checks if a string consists of only ASCII characters.

  bool is_ascii_string(const char *input);

convertToBraille: Converts lowercase alphabet letters to Braille Unicode characters.
  
  char *convertToBraille(char *input);

first_letter: Retrieves the first letter of a string.

  char *first_letter(char *string);

Usage

To use these functions, include the C file in your project and call the functions as needed.
There is a main fucntion that calls the testing suite.

Feel free to contribute, report issues, or suggest improvements.
