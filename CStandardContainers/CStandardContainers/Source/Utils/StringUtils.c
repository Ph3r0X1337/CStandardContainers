#include "StringUtils.h"
#include "MemoryUtils.h"

#define CSC_HIGH_SURROGATE_MIN (CSC_WORD)0xD800
#define CSC_HIGH_SURROGATE_MAX (CSC_WORD)0xDBFF
#define CSC_LOW_SURROGATE_MIN (CSC_WORD)0xDC00
#define CSC_LOW_SURROGATE_MAX (CSC_WORD)0xDFFF

CSC_SIZE_T CSCAPI CSC_StringUtilsStrLenAnsiString(_In_ CONST CSC_CHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN requireNullTerminator)
{
	CSC_SIZE_T iterator;

	if (!pCStr || maxChars > CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING)
	{
		return CSC_STRING_INVALID_LENGTH;
	}

	for (iterator = (CSC_SIZE_T)0; ((requireNullTerminator) ? iterator <= maxChars : iterator < maxChars); ++iterator)
	{
		if (!pCStr[iterator])
		{
			break;
		}

		if (requireNullTerminator && iterator >= maxChars)
		{
			return CSC_STRING_INVALID_LENGTH;
		}
	}

	return iterator;
}

CSC_SIZE_T CSCAPI CSC_StringUtilsStrLenWideString(_In_ CONST CSC_WCHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxCodePoints, _In_ CONST CSC_BOOLEAN requireNullTerminator)
{
	CSC_SIZE_T iterator;
	CSC_CodePointType currentCPT;
	CONST CSC_WCHAR* pCurrCodePoint = pCStr;

	if (!pCStr || maxCodePoints > CSC_MAXIMUM_STRING_LENGTH_WIDE_STRING)
	{
		return CSC_STRING_INVALID_LENGTH;
	}

	for (iterator = (CSC_SIZE_T)0; ((requireNullTerminator) ? iterator <= maxCodePoints : iterator < maxCodePoints); ++iterator, ++pCurrCodePoint)
	{
		if (!*pCurrCodePoint)
		{
			break;
		}

		if (requireNullTerminator && iterator >= maxCodePoints)
		{
			return CSC_STRING_INVALID_LENGTH;
		}

		if (CSC_StringUtilsIsBMPCharacter(*pCurrCodePoint))
		{
			continue;
		}

		if (!CSC_StringUtilsIsSurrogatePair(*pCurrCodePoint, pCurrCodePoint[(CSC_SIZE_T)1]))
		{
			return CSC_STRING_INVALID_LENGTH;
		}
		
		++pCurrCodePoint;
	}

	return iterator;
}

CSC_SIZE_T CSCAPI CSC_StringUtilsStrWideCharLenWideString(_In_ CONST CSC_WCHAR* CONST pCStr, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN requireNullTerminator, _Out_opt_ CSC_SIZE_T* CONST pCodePointLength)
{
	CSC_SIZE_T iterator, codePointLength;
	CSC_CodePointType currentCPT;

	if (!pCStr || maxChars > CSC_MAXIMUM_STRING_CHARS_WIDE_STRING)
	{
		if (pCodePointLength)
		{
			*pCodePointLength = CSC_STRING_INVALID_LENGTH;
		}

		return CSC_STRING_INVALID_LENGTH;
	}

	for (iterator = (CSC_SIZE_T)0, codePointLength = (CSC_SIZE_T)0; ((requireNullTerminator) ? iterator <= maxChars : iterator < maxChars); ++iterator, ++codePointLength)
	{
		if (!pCStr[iterator])
		{
			break;
		}

		if (requireNullTerminator && iterator >= maxChars)
		{
			codePointLength = CSC_STRING_INVALID_LENGTH;
			break;
		}

		if (CSC_StringUtilsIsBMPCharacter(pCStr[iterator]))
		{
			continue;
		}

		if (iterator >= maxChars - (CSC_SIZE_T)1 || !CSC_StringUtilsIsSurrogatePair(pCStr[iterator], pCStr[iterator + (CSC_SIZE_T)1]))
		{
			codePointLength = CSC_STRING_INVALID_LENGTH;
			break;
		}

		++iterator;
		continue;
	}

	if (codePointLength == CSC_STRING_INVALID_LENGTH)
	{
		iterator = codePointLength;
	}

	if (pCodePointLength)
	{
		*pCodePointLength = codePointLength;
	}

	return iterator;
}


CSC_STATUS CSCAPI CSC_StringUtilsCompareAnsiString(_In_ CONST CSC_CHAR* CONST pFirst, _In_ CONST CSC_CHAR* CONST pSecond, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN strict)
{
	CSC_SIZE_T lengthFirst, lengthSecond, compareLength;

	if (!pFirst || !pSecond || maxChars > CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	lengthFirst = CSC_StringUtilsStrLenAnsiString(pFirst, CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING, (CSC_BOOLEAN)TRUE);
	lengthSecond = CSC_StringUtilsStrLenAnsiString(pSecond, CSC_MAXIMUM_STRING_LENGTH_ANSI_STRING, (CSC_BOOLEAN)TRUE);

	if (lengthFirst == CSC_STRING_INVALID_LENGTH || lengthSecond == CSC_STRING_INVALID_LENGTH)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (strict)
	{
		if (lengthFirst != lengthSecond)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}
		
		compareLength = lengthFirst;
	}
	else
	{
		compareLength = ((lengthFirst < lengthSecond) ? lengthFirst : lengthSecond);
	}

	compareLength = ((compareLength < maxChars) ? compareLength : maxChars);

	return CSC_MemoryUtilsCompareMemory((CSC_PCVOID)pFirst, (CSC_PCVOID)pSecond, compareLength);
}

CSC_STATUS CSCAPI CSC_StringUtilsCompareWideString(_In_ CONST CSC_WCHAR* CONST pFirst, _In_ CONST CSC_WCHAR* CONST pSecond, _In_ CONST CSC_SIZE_T maxChars, _In_ CONST CSC_BOOLEAN strict)
{
	CSC_SIZE_T charLengthFirst, charLengthSecond, codePointLengthFirst, codePointLengthSecond, compareLength;

	if (!pFirst || !pSecond || maxChars > CSC_MAXIMUM_STRING_CHARS_WIDE_STRING)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	charLengthFirst = CSC_StringUtilsStrWideCharLenWideString(pFirst, CSC_MAXIMUM_STRING_CHARS_WIDE_STRING, (CSC_BOOLEAN)TRUE, &codePointLengthFirst);
	charLengthSecond = CSC_StringUtilsStrWideCharLenWideString(pSecond, CSC_MAXIMUM_STRING_CHARS_WIDE_STRING, (CSC_BOOLEAN)TRUE, &codePointLengthSecond);

	if (charLengthFirst == CSC_STRING_INVALID_LENGTH || charLengthFirst == CSC_STRING_INVALID_LENGTH)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (strict)
	{
		if (codePointLengthFirst != codePointLengthSecond || charLengthFirst != charLengthSecond)
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}

		compareLength = charLengthFirst;
	}
	else
	{
		compareLength = ((charLengthFirst < charLengthSecond) ? charLengthFirst : charLengthSecond);
	}

	compareLength = ((compareLength < maxChars) ? compareLength : maxChars) * sizeof(CSC_WCHAR);

	return CSC_MemoryUtilsCompareMemory((CSC_PCVOID)pFirst, (CSC_PCVOID)pSecond, compareLength);
}


CSC_BOOLEAN CSCAPI CSC_StringUtilsIsBMPCharacter(_In_ CONST CSC_WCHAR wChar)
{
	return (CSC_BOOLEAN)(((CSC_WORD)wChar < CSC_HIGH_SURROGATE_MIN || (CSC_WORD)wChar > CSC_LOW_SURROGATE_MAX) ? TRUE : FALSE);
}

CSC_BOOLEAN CSCAPI CSC_StringUtilsIsSurrogatePair(_In_ CONST CSC_WCHAR leading, _In_ CONST CSC_WCHAR trailing)
{
	if (leading < CSC_HIGH_SURROGATE_MIN || leading > CSC_HIGH_SURROGATE_MAX)
	{
		return (CSC_BOOLEAN)FALSE;
	}

	if (trailing < CSC_LOW_SURROGATE_MIN || trailing > CSC_LOW_SURROGATE_MAX)
	{
		return (CSC_BOOLEAN)FALSE;
	}

	return TRUE;
}

CSC_CodePointType CSCAPI CSC_StringUtilsGetCodePointType(_In_ CONST CSC_WCHAR wChar)
{
	// Check value for BMP range.
	if ((CSC_WORD)wChar < CSC_HIGH_SURROGATE_MIN || (CSC_WORD)wChar > CSC_LOW_SURROGATE_MAX)
	{
		return csc_cpt_bmp;
	}

	// Check value for high surrogate range.
	if ((CSC_WORD)wChar <= CSC_HIGH_SURROGATE_MAX && (CSC_WORD)wChar >= CSC_HIGH_SURROGATE_MIN)
	{
		return csc_cpt_surrogate_high;
	}

	// Check value for low surrogate range.
	if ((CSC_WORD)wChar <= CSC_LOW_SURROGATE_MAX && (CSC_WORD)wChar >= CSC_LOW_SURROGATE_MIN)
	{
		return csc_cpt_surrogate_low;
	}

	// Should never occurr.
	return csc_cpt_invalid;
}