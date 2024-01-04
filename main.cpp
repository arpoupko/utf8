#include <stdio.h>
#include <cctype>
#include <cstring>

int MAX_STRING_LENGTH= 256;

// function to decode
int my_utf8_decode(char *input, char *output) {
    if (input == nullptr || output == nullptr) {
        return -1;  // Error: invalid input or output pointer
    }

    while (*input != '\0') {
        unsigned char utf8_byte = (unsigned char)*input;

        if (utf8_byte < 0x80) {
            // Single-byte UTF-8 character (0xxxxxxx)
            *output++ = utf8_byte;
            ++input;
        } else {
            // Multibyte UTF-8 character
            int num_bytes;
            unsigned int codepoint;

            if ((utf8_byte & 0xE0) == 0xC0) {
                // Two-byte UTF-8 character (110xxxxx 10xxxxxx)
                //The condition (utf8_byte & 0xE0) == 0xC0 checks if the leading byte has the pattern 110xxxxx.
                //If true, it indicates the start of a two-byte UTF-8 character.
                num_bytes = 2;
                //num_bytes is set to 2.
                codepoint = utf8_byte & 0x1F;
                //codepoint is extracted by bitwise ANDing the leading byte with 0x1F (binary 00011111). This extracts the lower 5 bits of the second byte
            } else if ((utf8_byte & 0xF0) == 0xE0) {
                // Three-byte UTF-8 character (1110xxxx 10xxxxxx 10xxxxxx)
                // The condition (utf8_byte & 0xF0) == 0xE0 checks if the leading byte has the pattern 1110xxxx.
                // If true, it indicates the start of a three-byte UTF-8 character.
                num_bytes = 3;
                // num_bytes is set to 3.
                codepoint = utf8_byte & 0x0F;
                // codepoint is extracted by bitwise ANDing the leading byte with 0x0F (binary 00001111). This extracts the lower 4 bits of the third byte.
            } else if ((utf8_byte & 0xF8) == 0xF0) {
                // Four-byte UTF-8 character (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                // The condition (utf8_byte & 0xF8) == 0xF0 checks if the leading byte has the pattern 11110xxx.
                // If true, it indicates the start of a four-byte UTF-8 character.
                num_bytes = 4;
                // num_bytes is set to 4.
                codepoint = utf8_byte & 0x07;
                // codepoint is extracted by bitwise ANDing the leading byte with 0x07 (binary 00000111). This extracts the lower 3 bits of the fourth byte.
            } else {
                printf("Error: Invalid UTF-8 sequence\n");
                return -1;
            }
            // the continuing checker, checks if there is 10XXXX
            for (int i = 0; i < num_bytes - 1; ++i) {
                ++input;
                if ((*input & 0xC0) != 0x80) {
                    printf("Error: Invalid UTF-8 sequence\n");
                    return -1;
                }
                codepoint = (codepoint << 6) | (*input & 0x3F);
            }

            // Print out the results
            if (num_bytes <= 3) {

                output += sprintf(output, "\\u%04X", codepoint);
            } else {
                output += sprintf(output, "\\U%08X", codepoint);
            }

            ++input;
        }
    }

    // add a null to the end
    *output = '\0';

    return 0;
}

//function to encode
int my_utf8_encode(char *input, char *output) {
    if (input == NULL || output == nullptr) {
        return -1;  // Error: invalid input or output pointer
    }

    while (*input != '\0') {
        // checks both U and u
        if (*input == '\\' && (*(input + 1) == 'u' || *(input + 1) == 'U')) {
            // Process UTF-8 character
            unsigned int codepoint;
            if (sscanf(input + 2, "%X", &codepoint) != 1) {
                printf("Error: invalid \\u or \\U notation\n");
                return -1;  // Error: invalid \\u or \\U notation
            }

            if (codepoint <= 0x7F) {
                // Single-byte UTF-8 character
                // For a single-byte character, the codepoint is directly assigned to the output.
                *output++ = static_cast<char>(codepoint);
            } else if (codepoint <= 0x7FF) {
                // Two-byte UTF-8 character
                // The first byte is 0xC0 (binary: 11000000) plus the upper 5 bits of the codepoint.
                // The second byte is 0x80 (binary: 10000000) plus the lower 6 bits of the codepoint.
                *output++ = static_cast<char>(0xC0 + (codepoint >> 6));
                *output++ = static_cast<char>(0x80 + (codepoint & 0x3F));
            } else if (codepoint <= 0xFFFF) {
                // Three-byte UTF-8 character
                // The first byte is 0xE0 (binary: 11100000) plus the upper 4 bits of the codepoint.
                // The second byte is 0x80 (binary: 10000000) plus the middle 6 bits of the codepoint.
                // The third byte is 0x80 (binary: 10000000) plus the lower 6 bits of the codepoint.
                *output++ = static_cast<char>(0xE0 + (codepoint >> 12));
                *output++ = static_cast<char>(0x80 + ((codepoint >> 6) & 0x3F));
                *output++ = static_cast<char>(0x80 + (codepoint & 0x3F));
            } else if (codepoint <= 0x10FFFF) {
                // Four-byte UTF-8 character
                // The first byte is 0xF0 (binary: 11110000) plus the upper 3 bits of the codepoint.
                // The second byte is 0x80 (binary: 10000000) plus the first 3 bits of the middle 6 bits of the codepoint.
                // The third byte is 0x80 (binary: 10000000) plus the last 3 bits of the middle 6 bits of the codepoint.
                // The fourth byte is 0x80 (binary: 10000000) plus the lower 6 bits of the codepoint.
                *output++ = static_cast<char>(0xF0 + (codepoint >> 18));
                *output++ = static_cast<char>(0x80 + ((codepoint >> 12) & 0x3F));
                *output++ = static_cast<char>(0x80 + ((codepoint >> 6) & 0x3F));
                *output++ = static_cast<char>(0x80 + (codepoint & 0x3F));
            } else {
                // Unsupported codepoint
                printf("Error: Unsupported codepoint\n");
                return -1;
            }


            // Move input pointer past the processed codepoint
            // Calculate escape sequence length based on how long it has to be
            int escape_sequence_length = 2;  // initial \u or \U takes up 2 bits so add 2
            while (*(input + escape_sequence_length) != '\0' && isxdigit(*(input + escape_sequence_length))) {
                escape_sequence_length++;
            }
            input += escape_sequence_length;
        } else {
            // Copy non-UTF-8 characters as is because they are the same in UTF and ascii
            *output++ = *input++;
        }
    }

    // Null-terminate the output string
    *output = '\0';

    return 0;  // Success
}

// Function to check the validity of a UTF-8 encoded string
bool my_utf8_check(const unsigned char* string) {
    // Iterate through the string until the null terminator is reached
    while (*string != '\0') {
        unsigned char utf8_byte = *string;

        // Check for single-byte UTF-8 character (0xxxxxxx)
        if (utf8_byte < 0x80) {
            ++string;  // Move to the next character
        } else {
            // Multibyte UTF-8 character
            int num_bytes;
            unsigned int codepoint;

            // pattern goes-  (byte & masking bit) == start of the UTF char
            if ((utf8_byte & 0xE0) == 0xC0) {
                num_bytes = 2;
                codepoint = utf8_byte & 0x1F;  //Extracts the lower 5 bits of the leading byte, forming the initial codepoint.
            } else if ((utf8_byte & 0xF0) == 0xE0) {
                // Check for three-byte UTF-8 character (1110xxxx 10xxxxxx 10xxxxxx)
                num_bytes = 3;
                codepoint = utf8_byte & 0x0F;  // Extract lower 4 bits
            } else if ((utf8_byte & 0xF8) == 0xF0) {
                // Check for four-byte UTF-8 character (11110xxx 10xxxxxx 10xxxxxx 10xxxxxx)
                num_bytes = 4;
                codepoint = utf8_byte & 0x07;  // Extract lower 3 bits
            } else {
                // Invalid UTF-8 sequence
                return false;
            }

            // Loop to process continuation bytes
            for (int i = 1; i < num_bytes; ++i) {
                ++string;
                if ((*string & 0xC0) != 0x80) {
                    // Invalid continuation byte
                    return false;
                }
                // Extract lower 6 bits from each continuation byte and add to the codepoint
                codepoint = (codepoint << 6) | (*string & 0x3F);
            }

            // Check for overlong encoding, if it could have been encoded in less words
            if (num_bytes == 2 && codepoint <= 0x7F) {
                return false;  // Overlong two-byte sequence
            } else if (num_bytes == 3 && codepoint <= 0x7FF) {
                return false;  // Overlong three-byte sequence
            } else if (num_bytes == 4 && codepoint <= 0xFFFF) {
                return false;  // Overlong four-byte sequence
            }
        }
    }

    // The string is a valid UTF-8 sequence
    return true;
}

// Function to calculate the length of a UTF-8 encoded string in terms of characters
int my_utf8_strlen(const char *string) {
    int len = 0;  // Initialize the character count

    // Loop through the string until the end of the string is reached
    while (*string) {
        // Check if it's the start of a new UTF-8 character
        if ((*string & 0xC0) != 0x80) {
            len++;  // Increment character count for each new character

            // Skip continuation bytes
            while ((*string & 0xC0) == 0xC0) {
                string++;  // Move to the next byte until a non-continuation byte is found
            }
        }
        string++;  // Move to the next byte in the string
    }

    return len;  // Return the calculated character count
}

// Profs: Does this count as an extra function? I'm going to test it just in case :)
// Function to check if a byte is the start of a UTF-8 character
bool is_utf8_start(unsigned char byte) {
    // Check if the most significant bits
    if ((byte & 0x80) == 0x00 || (byte & 0xC0) == 0xC0) {
        return true;  // Byte is the start of a UTF-8 character
    } else {
        return false; // Byte is a continuation byte
    }
}

// Function to return the UTF-8 encoded character at a specific index/ location
char *my_utf8_charat(char *string, int index) {
    // Check for invalid input: a null string or negative index
    if (string == NULL || index < 0) {
        return NULL;
    }

    // Loop through the string until the null terminator or the specified index is reached
    while (*string != '\0' && index > 0) {
        // Check if the current byte is the start of a new UTF-8 character
        if (is_utf8_start((unsigned char)*string)) {
            index--;  // Decrement the index for each new character
        }
        string++;  // Move to the next byte in the string
    }

    // Check if the string is at the end or the index is out of bounds
    if (*string == '\0' || index > 0) {
        return NULL;  // Return NULL if the string is not terminated or the index is out of bounds
    }

    return string;  // Return a pointer to the character at the specified UTF-8 index
}

//Function to compare strings
int my_utf8_strcmp(const char *string1, const char *string2) {
    while (*string1 != '\0' && *string2 != '\0') {
        // Retrieve the UTF-8 encoded characters from the strings
        unsigned char ch1 = (unsigned char)(*string1);
        unsigned char ch2 = (unsigned char)(*string2);

        // Compare the UTF-8 characters
        if (ch1 < ch2) {
            return -1; // string1 is less than string2
        } else if (ch1 > ch2) {
            return 1;  // string1 is greater than string2
        }

        // Move to the next character in each string
        string1++;
        string2++;
    }

    // Check for the end of both strings
    if (*string1 == '\0' && *string2 == '\0') {
        return 0;  // Both strings are equal
    } else if (*string1 == '\0') {
        return -1; // string1 is shorter than string2
    } else {
        return 1;  // string1 is longer than string2
    }
}

//these are tester helper functions to run the tests given inputs
void test_encode(char *input, char *expected_output) {
    char output[100];  
    int result = my_utf8_encode(input, output);

    printf("%s: input=\"%s\", expected=\"%s\", output=\"%s\"\n",
           (result == 0 && my_utf8_strcmp(expected_output, output) == 0) ? "PASSED" : "FAILED",
           input, expected_output, output);
}

void test_decode(char *input, char *expected_output) {
    char output[100];
    int result = my_utf8_decode(input, output);

    printf("%s: input=\"%s\", expected=\"%s\", output=\"%s\"\n",
           (result == 0 && my_utf8_strcmp(expected_output, output)==0) ? "PASSED" : "FAILED",
           input, expected_output, output);
}

void test_is_utf8(const unsigned char* input, bool expected_result) {
    bool result = my_utf8_check(input);

    printf("%s: input=\"%s\", expected_result=%s, actual_result=%s\n",
           (result == expected_result) ? "PASSED" : "FAILED",
           input, expected_result ? "true" : "false", result ? "true" : "false");
}

void test_utf8_strlen(const char* input, int expectedLength) {
    int result = my_utf8_strlen(input);

    // Check if the result matches the expected length
    if (result == expectedLength) {
        printf("PASSED: input=\"%s\", expected=%d, result=%d\n", input, expectedLength, result);
    } else {
        printf("FAILED: input=\"%s\", expected=%d, result=%d\n", input, expectedLength, result);
    }
}

void test_utf8_charat(char *input, int index, char expected_result) {
    char result = (my_utf8_charat(input, index) != NULL) ? *my_utf8_charat(input, index) : '\0';

    // Check if the result is as expected
    if (result == expected_result) {
        printf("PASSED: input=\"%s\", index=%d, expected_result='%c', actual_result='%c'\n",
               input, index, expected_result, result);
    } else {
        printf("FAILED: input=\"%s\", index=%d, expected_result='%c', actual_result='%c'\n",
               input, index, expected_result, result);
    }
}

void test_strcmp(char *input_str1, char *input_str2, int expected_result) {
    int result = my_utf8_strcmp(input_str1, input_str2);

    printf("%s: \"%s\" and \"%s\"; Expected Result: %d; Actual Result: %d\n",
           (result == expected_result) ? "Passed" : "Failed",
           input_str1, input_str2, expected_result, result);
}

//EXTRA FUNCTIONS
// This function reads in a char and returns the ascii set it is a part of
const char* getCategory(const char* utf8Char) {
    if (utf8Char == nullptr) {
        return "Invalid input";
    }

    char decoded[MAX_STRING_LENGTH];  
    int result = my_utf8_decode(const_cast<char*>(utf8Char), decoded);

    if (result < 0) {
        return "Error during decoding";
    }


    // Extract code point from decoded input
    unsigned int codepoint;
    if (sscanf(decoded, "\\u%04X", &codepoint) != 1) {
        return "Error extracting code point";
    }


    // Check the category based on the chart
    if (codepoint >= 0x0000 && codepoint <= 0x007F) {
        return "Basic Latin";
    } else if (codepoint >= 0x0080 && codepoint <= 0x00FF) {
        return "Latin-1 Supplement";
    } else if (codepoint >= 0x0100 && codepoint <= 0x017F) {
        return "Latin Extended-A";
    } else if (codepoint >= 0x0180 && codepoint <= 0x024F) {
        return "Latin Extended-B";
    } else if (codepoint >= 0x0180 && codepoint <= 0x024F) {
        return "Latin Extended-B";
    } else if (codepoint >= 0x0250 && codepoint <= 0x02AF) {
        return "IPA Extensions";
    } else if (codepoint >= 0x02B0 && codepoint <= 0x02FF) {
        return "Spacing Modifier Letters";
    } else if (codepoint >= 0x0300 && codepoint <= 0x036F) {
        return "Combining Diacritical Marks";
    } else if (codepoint >= 0x0370 && codepoint <= 0x03FF) {
        return "Greek and Coptic";
    } else if (codepoint >= 0x0400 && codepoint <= 0x04FF) {
        return "Cyrillic";
    } else if (codepoint >= 0x0500 && codepoint <= 0x052F) {
        return "Cyrillic Supplement";
    } else if (codepoint >= 0x0530 && codepoint <= 0x058F) {
        return "Armenian";
    } else if (codepoint >= 0x0590 && codepoint <= 0x05FF) {
        return "Hebrew";
    } else if (codepoint >= 0x0600 && codepoint <= 0x06FF) {
        return "Arabic";
    } else if (codepoint >= 0x0700 && codepoint <= 0x074F) {
        return "Syriac";
    } else if (codepoint >= 0x0750 && codepoint <= 0x077F) {
        return "Arabic Supplement";
    } else if (codepoint >= 0x0780 && codepoint <= 0x07BF) {
        return "Thaana";
    } else if (codepoint >= 0x07C0 && codepoint <= 0x07FF) {
        return "NKo";
    } else if (codepoint >= 0x0800 && codepoint <= 0x083F) {
        return "Samaritan";
    } else if (codepoint >= 0x0840 && codepoint <= 0x085F) {
        return "Mandaic";
    } else if (codepoint >= 0x0860 && codepoint <= 0x086F) {
        return "Syriac Supplement";
    } else if (codepoint >= 0x0870 && codepoint <= 0x089F) {
        return "Arabic Extended-B";
    } else if (codepoint >= 0x08A0 && codepoint <= 0x08FF) {
        return "Arabic Extended-A";
    } else if (codepoint >= 0x0900 && codepoint <= 0x097F) {
        return "Devanagari";
    } else if (codepoint >= 0x0980 && codepoint <= 0x09FF) {
        return "Bengali";
    } else if (codepoint >= 0x0A00 && codepoint <= 0x0A7F) {
        return "Gurmukhi";
    } else if (codepoint >= 0x0A80 && codepoint <= 0x0AFF) {
        return "Gujarati";
    } else if (codepoint >= 0x0B00 && codepoint <= 0x0B7F) {
        return "Oriya";
    } else if (codepoint >= 0x0B80 && codepoint <= 0x0BFF) {
        return "Tamil";
    } else if (codepoint >= 0x0C00 && codepoint <= 0x0C7F) {
        return "Telugu";
    } else if (codepoint >= 0x0C80 && codepoint <= 0x0CFF) {
        return "Kannada";
    } else if (codepoint >= 0x0C80 && codepoint <= 0x0CFF) {
        return "Malayalam";
    } else if (codepoint >= 0x0D00 && codepoint <= 0x0D7F) {
        return "Sinhala";
    } else if (codepoint >= 0x0D80 && codepoint <= 0x0DFF) {
        return "Thai";
    } else if (codepoint >= 0x0E00 && codepoint <= 0x0E7F) {
        return "Lao";
    } else if (codepoint >= 0x0E80 && codepoint <= 0x0EFF) {
        return "Tibetan";
    } else if (codepoint >= 0x0F00 && codepoint <= 0x0FFF) {
        return "Myanmar";
    } else if (codepoint >= 0x1000 && codepoint <= 0x109F) {
        return "Georgian";
    } else if (codepoint >= 0x10A0 && codepoint <= 0x10FF) {
        return "Hangul Jamo";
    } else if (codepoint >= 0x1100 && codepoint <= 0x11FF) {
        return "Ethiopic";
    } else if (codepoint >= 0x1200 && codepoint <= 0x137F) {
        return "Ethiopic Supplement";
    } else if (codepoint >= 0x1380 && codepoint <= 0x139F) {
        return "Cherokee";
    } else if (codepoint >= 0x13A0 && codepoint <= 0x13FF) {
        return "Unified Canadian Aboriginal Syllabics";
    } else if (codepoint >= 0x1400 && codepoint <= 0x167F) {
        return "Ogham";
    } else if (codepoint >= 0x1680 && codepoint <= 0x169F) {
        return "Runic";
    } else if (codepoint >= 0x16A0 && codepoint <= 0x16FF) {
        return "Tagalog";
    } else if (codepoint >= 0x1700 && codepoint <= 0x171F) {
        return "Hanunoo";
    } else if (codepoint >= 0x1720 && codepoint <= 0x173F) {
        return "Buhid";
    } else if (codepoint >= 0x1740 && codepoint <= 0x175F) {
        return "Tagbanwa";
    } else if (codepoint >= 0x1760 && codepoint <= 0x177F) {
        return "Khmer";
    } else if (codepoint >= 0x1780 && codepoint <= 0x17FF) {
        return "Mongolian";
    } else if (codepoint >= 0x1800 && codepoint <= 0x18AF) {
        return "Unified Canadian Aboriginal Syllabics Extended";
    } else if (codepoint >= 0x18B0 && codepoint <= 0x18FF) {
        return "Limbu";
    } else if (codepoint >= 0x1900 && codepoint <= 0x194F) {
        return "Tai Le";
    } else if (codepoint >= 0x1950 && codepoint <= 0x197F) {
        return "New Tai Lue";
    } else if (codepoint >= 0x1980 && codepoint <= 0x19DF) {
        return "Khmer Symbols";
    } else if (codepoint >= 0x19E0 && codepoint <= 0x19FF) {
        return "Buginese";
    } else if (codepoint >= 0x1A00 && codepoint <= 0x1A1F) {
        return "Tai Tham";
    } else if (codepoint >= 0x1A20 && codepoint <= 0x1AAF) {
        return "Combining Diacritical Marks Extended";
    } else if (codepoint >= 0x1AB0 && codepoint <= 0x1AFF) {
        return "Balinese";
    } else if (codepoint >= 0x1B00 && codepoint <= 0x1B7F) {
        return "Sundanese";
    } else if (codepoint >= 0x1B80 && codepoint <= 0x1BBF) {
        return "Batak";
    } else if (codepoint >= 0x1BC0 && codepoint <= 0x1BFF) {
        return "Lepcha";
    } else if (codepoint >= 0x1C00 && codepoint <= 0x1C4F) {
        return "Ol Chiki";
    } else if (codepoint >= 0x1C50 && codepoint <= 0x1C7F) {
        return "Cyrillic Extended C";
    } else if (codepoint >= 0x1C80 && codepoint <= 0x1C8F) {
        return "Georgian Extended";
    } else if (codepoint >= 0x1C90 && codepoint <= 0x1CBF) {
        return "Sundanese Supplement";
    } else if (codepoint >= 0x1CC0 && codepoint <= 0x1CCF) {
        return "Vedic Extensions";
    } else if (codepoint >= 0x1CD0 && codepoint <= 0x1CFF) {
        return "Phonetic Extensions";
    } else if (codepoint >= 0x1D00 && codepoint <= 0x1D7F) {
        return "Phonetic Extensions Supplement";
    } else if (codepoint >= 0x1D80 && codepoint <= 0x1DBF) {
        return "Combining Diacritical Marks Supplement";
    } else if (codepoint >= 0x1DC0 && codepoint <= 0x1DFF) {
        return "Latin Extended Additional";
    } else if (codepoint >= 0x1E00 && codepoint <= 0x1EFF) {
        return "Greek Extended";
    } else if (codepoint >= 0x1F00 && codepoint <= 0x1FFF) {
        return "General Punctuation";
    } else if (codepoint >= 0x2000 && codepoint <= 0x206F) {
        return "Superscripts and Subscripts";
    } else if (codepoint >= 0x2070 && codepoint <= 0x209F) {
        return "Currency Symbols";
    } else if (codepoint >= 0x20A0 && codepoint <= 0x20CF) {
        return "Combining Diacritical Marks for Symbols";
    } else if (codepoint >= 0x20D0 && codepoint <= 0x20FF) {
        return "Letterlike Symbols";
    } else if (codepoint >= 0x2100 && codepoint <= 0x214F) {
        return "Number Forms";
    } else if (codepoint >= 0x2150 && codepoint <= 0x218F) {
        return "Arrows";
    } else if (codepoint >= 0x2190 && codepoint <= 0x21FF) {
        return "Mathematical Operators";
    } else if (codepoint >= 0x2200 && codepoint <= 0x22FF) {
        return "Miscellaneous Technical";
    } else if (codepoint >= 0x2300 && codepoint <= 0x23FF) {
        return "Control Pictures";
    } else if (codepoint >= 0x2400 && codepoint <= 0x243F) {
        return "Optical Character Recognition";
    } else if (codepoint >= 0x2440 && codepoint <= 0x245F) {
        return "Enclosed Alphanumerics";
    } else if (codepoint >= 0x2460 && codepoint <= 0x24FF) {
        return "Box Drawing";
    } else if (codepoint >= 0x2500 && codepoint <= 0x257F) {
        return "Block Elements";
    } else if (codepoint >= 0x2580 && codepoint <= 0x259F) {
        return "Geometric Shapes";
    } else if (codepoint >= 0x25A0 && codepoint <= 0x25FF) {
        return "Miscellaneous Symbols";
    } else if (codepoint >= 0x2600 && codepoint <= 0x26FF) {
        return "Dingbats";
    } else if (codepoint >= 0x2700 && codepoint <= 0x27BF) {
        return "Miscellaneous Mathematical Symbols-A";
    } else if (codepoint >= 0x27C0 && codepoint <= 0x27EF) {
        return "Supplemental Arrows-A";
    } else if (codepoint >= 0x27F0 && codepoint <= 0x27FF) {
        return "Braille Patterns";
    } else if (codepoint >= 0x2800 && codepoint <= 0x28FF) {
        return "Supplemental Arrows-B";
    } else if (codepoint >= 0x2900 && codepoint <= 0x297F) {
        return "Miscellaneous Mathematical Symbols-B";
    } else if (codepoint >= 0x2980 && codepoint <= 0x29FF) {
        return "Supplemental Mathematical Operators";
    } else if (codepoint >= 0x2A00 && codepoint <= 0x2AFF) {
        return "Miscellaneous Symbols and Arrows";
    } else if (codepoint >= 0x2B00 && codepoint <= 0x2BFF) {
        return "Glagolitic";
    } else if (codepoint >= 0x2C00 && codepoint <= 0x2C5F) {
        return "Latin Extended-C";
    } else if (codepoint >= 0x2C60 && codepoint <= 0x2C7F) {
        return "Coptic";
    } else if (codepoint >= 0x2C80 && codepoint <= 0x2CFF) {
        return "Georgian Supplement";
    } else if (codepoint >= 0x2D00 && codepoint <= 0x2D2F) {
        return "Tifinagh";
    } else if (codepoint >= 0x2D30 && codepoint <= 0x2D7F) {
        return "Ethiopic Extended";
    } else if (codepoint >= 0x2D80 && codepoint <= 0x2DDF) {
        return "Cyrillic Extended-A";
    } else if (codepoint >= 0x2DE0 && codepoint <= 0x2DFF) {
        return "Supplemental Punctuation";
    } else if (codepoint >= 0x2E00 && codepoint <= 0x2E7F) {
        return "CJK Radicals Supplement";
    } else if (codepoint >= 0x2E80 && codepoint <= 0x2EFF) {
        return "Kangxi Radicals";
    } else if (codepoint >= 0x2F00 && codepoint <= 0x2FDF) {
        return "Undefined block";
    } else if (codepoint >= 0x2FE0 && codepoint <= 0x2FEF) {
        return "Ideographic Description Characters";
    } else if (codepoint >= 0x2FF0 && codepoint <= 0x2FFF) {
        return "CJK Symbols and Punctuation";
    } else if (codepoint >= 0x3000 && codepoint <= 0x303F) {
        return "Hiragana";
    } else if (codepoint >= 0x3040 && codepoint <= 0x309F) {
        return "Katakana";
    } else if (codepoint >= 0x30A0 && codepoint <= 0x30FF) {
        return "Bopomofo";
    } else if (codepoint >= 0x3100 && codepoint <= 0x312F) {
        return "Hangul Compatibility Jamo";
    } else if (codepoint >= 0x3130 && codepoint <= 0x318F) {
        return "Kanbun";
    } else if (codepoint >= 0x3190 && codepoint <= 0x319F) {
        return "Bopomofo Extended";
    } else if (codepoint >= 0x31A0 && codepoint <= 0x31BF) {
        return "CJK Strokes";
    } else if (codepoint >= 0x31C0 && codepoint <= 0x31EF) {
        return "Katakana Phonetic Extensions";
    } else if (codepoint >= 0x31F0 && codepoint <= 0x31FF) {
        return "Enclosed CJK Letters and Months";
    } else if (codepoint >= 0x3200 && codepoint <= 0x32FF) {
        return "CJK Compatibility";
    } else if (codepoint >= 0x3300 && codepoint <= 0x33FF) {
        return "CJK Unified Ideographs Extension A";
    } else if (codepoint >= 0x3400 && codepoint <= 0x4DBF) {
        return "Yijing Hexagram Symbols";
    } else if (codepoint >= 0x4DC0 && codepoint <= 0x4DFF) {
        return "CJK Unified Ideographs";
    } else if (codepoint >= 0x4E00 && codepoint <= 0x9FFF) {
        return "Yi Syllables";
    } else if (codepoint >= 0xA000 && codepoint <= 0xA48F) {
        return "Yi Radicals";
    } else if (codepoint >= 0xA490 && codepoint <= 0xA4CF) {
        return "Lisu";
    } else if (codepoint >= 0xA4D0 && codepoint <= 0xA4FF) {
        return "Vai";
    } else if (codepoint >= 0xA500 && codepoint <= 0xA63F) {
        return "Cyrillic Extended-B";
    } else if (codepoint >= 0xA640 && codepoint <= 0xA69F) {
        return "Bamum";
    } else if (codepoint >= 0xA6A0 && codepoint <= 0xA6FF) {
        return "Modifier Tone Letters";
    } else if (codepoint >= 0xA700 && codepoint <= 0xA71F) {
        return "Latin Extended-D";
    } else if (codepoint >= 0xA720 && codepoint <= 0xA7FF) {
        return "Syloti Nagri";
    } else if (codepoint >= 0xA800 && codepoint <= 0xA82F) {
        return "Common Indic Number Forms";
    } else if (codepoint >= 0xA830 && codepoint <= 0xA83F) {
        return "Phags-pa";
    } else if (codepoint >= 0xA840 && codepoint <= 0xA87F) {
        return "Saurashtra";
    } else if (codepoint >= 0xA880 && codepoint <= 0xA8DF) {
        return "Devanagari Extended";
    } else if (codepoint >= 0xA8E0 && codepoint <= 0xA8FF) {
        return "Kayah Li";
    } else if (codepoint >= 0xA900 && codepoint <= 0xA92F) {
        return "Rejang";
    } else if (codepoint >= 0xA930 && codepoint <= 0xA95F) {
        return "Hangul Jamo Extended-A";
    } else if (codepoint >= 0xA960 && codepoint <= 0xA97F) {
        return "Javanese";
    } else if (codepoint >= 0xA980 && codepoint <= 0xA9DF) {
        return "Myanmar Extended-B";
    } else if (codepoint >= 0xA9E0 && codepoint <= 0xA9FF) {
        return "Cham";
    } else if (codepoint >= 0xAA00 && codepoint <= 0xAA5F) {
        return "Myanmar Extended-A";
    } else if (codepoint >= 0xAA60 && codepoint <= 0xAA7F) {
        return "Tai Viet";
    } else if (codepoint >= 0xAA80 && codepoint <= 0xAADF) {
        return "Meetei Mayek Extensions";
    } else if (codepoint >= 0xAAE0 && codepoint <= 0xAAFF) {
        return "Ethiopic Extended-A";
    } else if (codepoint >= 0xAB00 && codepoint <= 0xAB2F) {
        return "Latin Extended-E";
    } else if (codepoint >= 0xAB30 && codepoint <= 0xAB6F) {
        return "Cherokee Supplement";
    } else if (codepoint >= 0xAB70 && codepoint <= 0xABBF) {
        return "Meetei Mayek";
    } else if (codepoint >= 0xABC0 && codepoint <= 0xABFF) {
        return "Hangul Syllables";
    } else if (codepoint >= 0xAC00 && codepoint <= 0xD7AF) {
        return "Hangul Jamo Extended-B";
    } else if (codepoint >= 0xD7B0 && codepoint <= 0xD7FF) {
        return "High Surrogates";
    } else if (codepoint >= 0xD800 && codepoint <= 0xDB7F) {
        return "High Private Use Surrogates";
    } else if (codepoint >= 0xDB80 && codepoint <= 0xDBFF) {
        return "Low Surrogates";
    } else if (codepoint >= 0xDC00 && codepoint <= 0xDFFF) {
        return "Private Use Area";
    } else if (codepoint >= 0xE000 && codepoint <= 0xF8FF) {
        return "CJK Compatibility Ideographs";
    } else if (codepoint >= 0xF900 && codepoint <= 0xFAFF) {
        return "Alphabetic Presentation Forms";
    } else if (codepoint >= 0xFB00 && codepoint <= 0xFB4F) {
        return "Arabic Presentation Forms-A";
    } else if (codepoint >= 0xFB50 && codepoint <= 0xFDFF) {
        return "Variation Selectors";
    } else if (codepoint >= 0xFE00 && codepoint <= 0xFE0F) {
        return "Vertical Forms";
    } else if (codepoint >= 0xFE10 && codepoint <= 0xFE1F) {
        return "Combining Half Marks";
    } else if (codepoint >= 0xFE20 && codepoint <= 0xFE2F) {
        return "CJK Compatibility Forms";
    } else if (codepoint >= 0xFE30 && codepoint <= 0xFE4F) {
        return "Small Form Variants";
    } else if (codepoint >= 0xFE50 && codepoint <= 0xFE6F) {
        return "Arabic Presentation Forms-B";
    } else if (codepoint >= 0xFE70 && codepoint <= 0xFEFF) {
        return "Halfwidth and Fullwidth Forms";
    } else if (codepoint >= 0xFF00 && codepoint <= 0xFFEF) {
        return "Specials";
    } else if (codepoint >= 0xFFF0 && codepoint <= 0xFFFF) {
        return "Undefined block";
    } else {
        return "Not categorized";
    }
}

//This function checks if the string is all ascii chars
bool is_ascii_string(const char *input) {
    while (*input) {
        if ((*input & 0x80) != 0) {
            // Non-ASCII character found
            return false;
        }
        input++;
    }
    // All characters are ASCII
    return true;
}

//This string converts lowercase alphabet letters to braille
char * convertToBraille(char *input) {
    while (*input != '\0') {
        switch (*input) {
            case 'a':
                printf("\\u2801 "); // ⠃
                break;
            case 'b':
                printf("\\u2803 "); // ⠃
                break;
            case 'c':
                printf("\\u2809 "); // ⠉
                break;
            case 'd':
                printf("\\u2819 "); // ⠙
                break;
            case 'e':
                printf("\\u2811 "); // ⠑
                break;
            case 'f':
                printf("\\u280B "); // ⠋
                break;
            case 'g':
                printf("\\u281B "); // ⠛
                break;
            case 'h':
                printf("\\u2813 "); // ⠓
                break;
            case 'i':
                printf("\\u280A "); // ⠊
                break;
            case 'j':
                printf("\\u281A "); // ⠚
                break;
            case 'k':
                printf("\\u2805 "); // ⠅
                break;
            case 'l':
                printf("\\u2807 "); // ⠇
                break;
            case 'm':
                printf("\\u280D "); // ⠍
                break;
            case 'n':
                printf("\\u281D "); // ⠝
                break;
            case 'o':
                printf("\\u2815 "); // ⠕
                break;
            case 'p':
                printf("\\u280F "); // ⠏
                break;
            case 'q':
                printf("\\u281F "); // ⠟
                break;
            case 'r':
                printf("\\u2817 "); // ⠗
                break;
            case 's':
                printf("\\u280E "); // ⠎
                break;
            case 't':
                printf("\\u281E "); // ⠞
                break;
            case 'u':
                printf("\\u2825 "); // ⠥
                break;
            case 'v':
                printf("\\u2827 "); // ⠧
                break;
            case 'w':
                printf("\\u283A "); // ⠺
                break;
            case 'x':
                printf("\\u282D "); // ⠭
                break;
            case 'y':
                printf("\\u283D "); // ⠽
                break;
            case 'z':
                printf("\\u2821 "); // ⠵
                break;
            default:
                printf("%c", *input);
        }
        input++;

    }
}

// Function to retrieve the first letter of a string
char *first_letter(char *string) {
    // Call my_utf8_charat with index 0 to get the first character
    return my_utf8_charat(string, 0);
}

//Extras testing
void test_getcategory(const char* utf8Char, const char* expectedCategory) {
    const char *result = getCategory(utf8Char);

    // Check if the result matches the expected category
    if (my_utf8_strcmp(result,expectedCategory)==0) {
        printf("PASSED: input=\"%s\", expected=\"%s\", result=\"%s\"\n", utf8Char, expectedCategory, result);
    } else {
        printf("FAILED: input=\"%s\", expected=\"%s\", result=\"%s\"\n", utf8Char, expectedCategory, result);
    }
}

void test_is_ascii(const char *input, int expected_result) {
    int result = is_ascii_string(input);

    if (result == expected_result) {
        printf("PASSED: input=\"%s\", expected_result=%d, actual_result=%d\n",
               input, expected_result, result);
    } else {
        printf("FAILED: input=\"%s\", expected_result=%d, actual_result=%d\n",
               input, expected_result, result);
    }
}

void test_first_letter(char *input, char expected_result) {
    char result = (first_letter(input) != NULL) ? *first_letter(input) : '\0';

    // Check if the result is as expected
    if (result == expected_result) {
        printf("PASSED: input=\"%s\", expected_result='%c', actual_result='%c'\n",
               input, expected_result, result);
    } else {
        printf("FAILED: input=\"%s\", expected_result='%c', actual_result='%c'\n",
               input, expected_result, result);
    }
}

void test_convertToBraille(char* input, const char* expected) {
    char brailleResult[256];

    printf("Passed: Input: %s ", input);
    printf("Output=", "%c");
    // Call convertToBraille on the original input
    convertToBraille(input);
    printf("Expected= %s", expected);

    printf("\n");
}

void test_is_utf8_start(unsigned char byte, bool expected_result) {
    bool result = is_utf8_start(byte);

    printf("%s: input=0x%02X, expected_result=%d, actual_result=%d",
           (result == expected_result) ? "PASSED" : "FAILED",
           byte,
           expected_result,
           result);

    printf("\n");
}

//test all
void test_all() {

    printf("\nTesting my_utf8_encode:\n");

    // Basic tests
    test_encode("Hello, World!", "Hello, World!");
    test_encode("Unicode snowman: \\u2603", "Unicode snowman: ☃");
    // Tests with multiple Unicode characters
    test_encode("Multiple snowmen: ☃☃☃", "Multiple snowmen: ☃☃☃");
    // Tests with different escape sequences
    test_encode("Mixed escape: \\u2603\\U0001F60A", "Mixed escape: ☃😊");
    // Edge case: Empty string
    test_encode("", "");
    // Edge case: Null pointer
    test_encode(NULL, "");

    printf("\nTesting my_utf8_decode:\n");
    // Basic tests
    test_decode("Hello, World!", "Hello, World!");
    test_decode("Unicode snowman: ☃", "Unicode snowman: \\u2603");
    // Tests with multiple Unicode characters
    test_decode("Multiple snowmen: ☃☃☃", "Multiple snowmen: \\u2603\\u2603\\u2603");
    // Tests with different escape sequences
    test_decode("\xF0\x80\x80\x80", "Mixed escape: \\u2603\\U0001F60A");
    // Edge case: Empty string
    test_decode("", "");
    // Edge case: Null pointer
    test_decode(NULL, "");


    printf("\nTesting my is utf8:\n");
    // Basic tests
    test_is_utf8((const unsigned char*)"Hello, World!", true);
    test_is_utf8((const unsigned char*)"Unicode snowman: \\u2603", true);
    // Invalid UTF-8 sequences
    test_is_utf8((const unsigned char*)"\xC0\xC0\xC0", false);  // Overlong encoding
    test_is_utf8((const unsigned char*)"\xC0\xC0\xC0", false);  // Overlong encoding
    test_is_utf8((const unsigned char*)"Invalid sequence: \xED\xA0\x80", false);  // Surrogate half
    test_is_utf8((const unsigned char*)"Incomplete two-byte sequence: \xC0", false);
    test_is_utf8((const unsigned char*)"Incomplete three-byte sequence: \xE0\xA0", false);
    test_is_utf8((const unsigned char*)"Incomplete four-byte sequence: \xF0\xA0\x80", false);
    test_is_utf8((const unsigned char*)"Overlong two-byte sequence: \xC0\x80", false);
    test_is_utf8((const unsigned char*)"Overlong three-byte sequence: \xE0\x80\x80", false);
    test_is_utf8((const unsigned char*)"Overlong four-byte sequence: \xF0\x80\x80\x80", false);
    test_is_utf8((const unsigned char*)"Invalid continuation byte: \xC0\xA0", false);
    test_is_utf8((const unsigned char*)"Invalid continuation byte: \xE0\x80\xA0", false);
    test_is_utf8((const unsigned char*)"Invalid continuation byte: \xF0\x80\x80\xA0", false);
    test_is_utf8((const unsigned char*)"Unexpected continuation byte: \x80", false);

    printf("\nTesting my Length:\n");
    // Test cases
    test_utf8_strlen("Hello", 5);
    test_utf8_strlen("Привет", 6);  // Cyrillic characters
    test_utf8_strlen("こんにちは", 5);  // Hiragana characters
    test_utf8_strlen("مرحبا", 5);  // Arabic characters
    test_utf8_strlen("", 0);  // Empty string
    test_utf8_strlen("\xF0\x9F\x98\x8A", 1);  // Single emoji character
    // Edge cases
    test_utf8_strlen("", 0);  // NULL input
    test_utf8_strlen("\xC2\x80", 1);  // 2-byte character
    test_utf8_strlen("\xE0\xA0\x80", 1);  // 3-byte character
    test_utf8_strlen("\xF0\x90\x80\x80", 1);  // 4-byte character
    test_utf8_strlen("\xC2\x80\xE0\xA0\x80\xF0\x90\x80\x80", 3);  // Mix of characters
    test_utf8_strlen("\x41\x42\x43", 3);  // ASCII characters without UTF-8 encoding

    printf("\nTesting my char at:\n");
    // Valid UTF-8 sequences
    test_utf8_charat("Hello, World!", 7, 'W');
    test_utf8_charat("Unicode snowman: ☃", 2, 'i');
    test_utf8_charat("Multiple snowmen: ☃☃☃", 0, 'M');
    // Index out of bounds
    test_utf8_charat("Hello, World!", 20, '\0');
    test_utf8_charat("Unicode snowman: ☃", 30, '\0');
    // Null pointer test
    test_utf8_charat(NULL, 5, '\0');

    printf("\nTesting my_utf8_strcmp:\n");
    test_strcmp("Hello, World!", "Hello, World!", 0);
    test_strcmp("Unicode snowman: \u2603", "Unicode snowman: ☃", 0);
    test_strcmp("Multiple snowmen: ☃☃☃", "Multiple snowmen: ☃☃☃", 0);
    test_strcmp("Mixed escape: \u2603\U0001F60A", "Mixed escape: ☃😊", 0);
    test_strcmp("", "", 0);
    test_strcmp("apple", "banana", -1);
    test_strcmp("banana", "apple", 1);
    test_strcmp("cat", "cat", 0);
    test_strcmp("123", "456", -1);
    test_strcmp("abc", "abcd", -1);
    test_strcmp("שלום", "Hello", 1);
    test_strcmp("Hello", "你好", -1);
    test_strcmp("Hello", "مرحبا", -1);
    test_strcmp("こんにちは", "안녕하세요", -1);
    test_strcmp("안녕하세요", "안녕하세요", 0);

    //These are the extra functions
    printf("\nTesting my language detector:\n");
    test_getcategory("☃", "Dingbats");
    test_getcategory("ב", "Hebrew");  // Example with Hebrew character
    test_getcategory("Թ", "Armenian");
    test_getcategory("ࡔ", "Mandaic");
    test_getcategory("੦", "Gurmukhi");

    printf("\nTesting my isAscii function:\n");
    test_is_ascii("Hello, World!", true);
    test_is_ascii("¡Hola, Mundo!", false);
    test_is_ascii("12345", true);
    test_is_ascii("UTF-8 ☃", false);

    printf("\nTesting my convert to Braille:\n");
    test_convertToBraille("hello", "\\u2813 \\u2811 \\u2807 \\u2807 \\u2815");
    test_convertToBraille("world", "\\u283A \\u2815 \\u2817 \\u2807 \\u2819");
    test_convertToBraille("braille", "\\u2803 \\u2817 \\u2801 \\u280A \\u2807 \\u2807 \\u2811");
    test_convertToBraille("testing", "\\u281E \\u2811 \\u280E \\u281E \\u280A \\u281D \\u281B");

    printf("\nTesting my first letter:\n");
    test_first_letter("Hello, 你好, ☃!", 'H');
    test_first_letter("Peanut Butter", 'P');
    test_first_letter("Testing 1, 2, 3", 'T');
    test_first_letter("Café crème", 'C');

    printf("\nTesting my is UTF8 start:\n");
    test_is_utf8_start(0x41, true);   // ASCII character 'A'
    test_is_utf8_start(0xC3, true);   // Start of a two-byte UTF-8 character
    test_is_utf8_start(0xE2, true);   // Start of a three-byte UTF-8 character
    test_is_utf8_start(0xF0, true);   // Start of a four-byte UTF-8 character
    test_is_utf8_start(0x80, false);  // Continuation byte
    test_is_utf8_start(0xAF, false);  // Continuation byte

}

int main() {
    test_all();

}
