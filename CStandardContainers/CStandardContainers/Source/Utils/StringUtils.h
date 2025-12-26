#ifndef CSC_STRING_UTILS
#define CSC_STRING_UTILS

/*
Description:
The StringUtils provide a small selection of helper functions to work with ansi or UTF-16 LE strings.
None of these helper functions perform any changes on the given data, they only provide information.
The utilities provide options to query string length or for UTF-16 LE the length of a string in code units aswell.
Furthermore they include comparison functions to perform a mostly memory based compare of two given strings.
Specifically for UTF-16 LE strings there are also helper functions that help with identifying individual code units as a particular UTF-16 code point type.
While ansi strings simply consist of characters, UTF-16 strings consist of UTF-16 code units, which take up 16 bits (hence UTF-16).
Code units are also referred to as "characters" in the CSC library, while the string length is determined by the amount of UTF-16 code points, which can take up either one or two UTF-16 code units.
Every code point is essentially the equivalent of a "unicode character" in the terms of UTF-16.
*/

// Include the current configuration of the library containing various type definitions and other things.
#include "../Configuration/Configuration.h"

// Definition of invalid string length and invalid index into a string.
#define CSC_STRING_INVALID_LENGTH (CSC_SIZE_T)-1
#define CSC_STRING_INVALID_INDEX CSC_STRING_INVALID_LENGTH

// Definition of maximum string length for ansi strings and UTF-16 LE strings as well as the maximum amount of code units for UTF-16 LE strings based on the data bus address width.
// Architectures with an address width of 32 bits or more share the same restrictions in maximum size.
#if CSC_ADDRESS_BUS_WIDTH_DATA < CSC_ADDRESS_BUS_WIDTH_DATA_32BIT
#define CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING (CSC_SIZE_T)0x1FFF
#define CSC_MAXIMUM_STRING_LENGTH_WIDE_STRING (CSC_SIZE_T)0x07FF
#define CSC_MAXIMUM_STRING_CHARS_WIDE_STRING (CSC_SIZE_T)0x0FFE
#else
#define CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING (CSC_SIZE_T)0x1FFFFFFF
#define CSC_MAXIMUM_STRING_LENGTH_WIDE_STRING (CSC_SIZE_T)0x07FFFFFF
#define CSC_MAXIMUM_STRING_CHARS_WIDE_STRING (CSC_SIZE_T)0x0FFFFFFE
#endif

// Defines the different UTF-16 code point types.
typedef enum _CSC_CodePointType
{
	csc_cpt_invalid = 0,
	csc_cpt_bmp,
	csc_cpt_surrogate_high,
	csc_cpt_surrogate_low
} CSC_CodePointType;

// Helper function to retrieve the length of an ansi string in ansi characters, performing the check for up to maxChars of characters.
// If the null terminator is required, the function might check up to maxChars + 1 characters to be able to verify the null terminator.
// Supplying a value for maxChars higher than CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING is not supported.
CSC_SIZE_T CSCAPI CSC_StringUtilsStrLenAnsiString(_In_ CONST CSC_CHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN requireNullTerminator);
// Helper function to retrieve the length of an UTF-16 LE string in UTF-16 code points, performing the check for up to maxCodePoints of code points.
// If the null terminator is required, the function might check up to maxCodePoints + 1 code points to be able to verify the null terminator.
// Supplying a value for maxCodePoints higher than CSC_MAXIMUM_STRING_LENGTH_WIDE_STRING is not supported.
CSC_SIZE_T CSCAPI CSC_StringUtilsStrLenWideString(_In_ CONST CSC_WCHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxCodePoints, _In_ CONST CSC_BOOLEAN requireNullTerminator);
// Helper function to retrieve the length of an UTF-16 LE string in UTF-16 code units, performing the check for up to maxChars of code units.
// If the null terminator is required, the function might check up to maxChars + 1 code units to be able to verify the null terminator.
// Supplying a value for maxChars higher than CSC_MAXIMUM_STRING_CHARS_WIDE_STRING is not supported.
CSC_SIZE_T CSCAPI CSC_StringUtilsStrWideCharLenWideString(_In_ CONST CSC_WCHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN requireNullTerminator, _Out_opt_ CSC_SIZE_T* CONST pCodePointLength);

// Helper function to compare two ansi strings for up to maxChars of characters.
// If strict mode is used, both strings must match in length, otherwise the shorter string length of both strings is taken for comparison if it is smaller than maxChars.
// Returns CSC_STATUS_SUCCESS if both strings match, CSC_STATUS_GENERAL_FAILURE if they don't match or another status code if an error occurrs.
// Supplying a value for maxChars higher than CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING is not supported.
CSC_STATUS CSCAPI CSC_StringUtilsCompareAnsiString(_In_ CONST CSC_CHAR* CONST pFirst, _In_ CONST CSC_CHAR* CONST pSecond, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN strict);
// Helper function to compare two UTF-16 LE strings for up to maxChars of code units.
// If strict mode is used, both strings must match in length, otherwise the shorter string length of both strings is taken for comparison if it is smaller than maxChars.
// Returns CSC_STATUS_SUCCESS if both strings match, CSC_STATUS_GENERAL_FAILURE if they don't match or another status code if an error occurrs.
// Supplying a value for maxChars higher than CSC_MAXIMUM_STRING_CHARS_WIDE_STRING is not supported.
CSC_STATUS CSCAPI CSC_StringUtilsCompareWideString(_In_ CONST CSC_WCHAR* CONST pFirst, _In_ CONST CSC_WCHAR* CONST pSecond, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN strict);

// Helper function to determine whether a supplied UTF-16 code unit is a bmp code point.
CSC_BOOLEAN CSCAPI CSC_StringUtilsIsBMPCharacter(_In_ CONST CSC_WCHAR wChar);
// Helper function to determine whether supplied leading and trailing UTF-16 code units are a valid surrogate pair.
CSC_BOOLEAN CSCAPI CSC_StringUtilsIsSurrogatePair(_In_ CONST CSC_WCHAR leading, _In_ CONST CSC_WCHAR trailing);
// Helper function to determine the UTF-16 code point type of a supplied UTF-16 code unit.
CSC_CodePointType CSCAPI CSC_StringUtilsGetCodePointType(_In_ CONST CSC_WCHAR wChar);

#endif