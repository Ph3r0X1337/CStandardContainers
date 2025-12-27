#include "MemoryUtils.h"

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemMoveBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size)
{
	CSC_SIZE_T iterator;
	CSC_BOOLEAN reverseDir = ((CONST CSC_BYTE* CONST)pDst > (CONST CSC_BYTE* CONST)pSrc && (CONST CSC_BYTE* CONST)pDst < (CONST CSC_BYTE* CONST)pSrc + size) ? (CSC_BOOLEAN)TRUE : (CSC_BOOLEAN)FALSE;

	if (!pDst || !pSrc || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDst == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < size; ++iterator)
	{
		if (reverseDir)
		{
			((CSC_BYTE* CONST)pDst)[size - iterator - (CSC_SIZE_T)1] = ((CONST CSC_BYTE* CONST)pSrc)[size - iterator - (CSC_SIZE_T)1];
		}
		else
		{
			((CSC_BYTE* CONST)pDst)[iterator] = ((CONST CSC_BYTE* CONST)pSrc)[iterator];
		}
	}

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemSetBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size)
{
	CSC_SIZE_T iterator;

	if (!pDst || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < size; ++iterator)
	{
		((CSC_BYTE* CONST)pDst)[iterator] = value;
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemoryBasic(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size)
{
	CSC_SIZE_T iterator;

	if (!pFirst || !pSecond || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < size; ++iterator)
	{
		if (((CONST CSC_BYTE* CONST)pFirst)[iterator] != ((CONST CSC_BYTE* CONST)pSecond)[iterator])
		{
			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	return CSC_STATUS_SUCCESS;
}

CSC_STATUS CSCAPI CSC_MemoryUtilsCopyMemoryBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size)
{
	return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetMemoryBasic(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size)
{
	return CSC_MemoryUtilsMemSetBasic(pDst, value, size);
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetZeroMemoryBasic(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_SIZE_T size)
{
	return CSC_MemoryUtilsMemSetBasic(pDst, (CSC_BYTE)0, size);
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetArrayValueBasic(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T elementCount)
{
	CSC_STATUS status;
	CSC_SIZE_T iterator;

	if (!pDst || !pValue || !elementSize || !elementCount)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < elementCount; ++iterator)
	{
		status = CSC_MemoryUtilsMemMoveBasic((CSC_PVOID)((CSC_BYTE* CONST)pDst + iterator * elementSize), pValue, elementSize);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

	return CSC_STATUS_SUCCESS;
}

#if CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION == TRUE && CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

typedef enum _CSC_SizeType
{
	csc_st_byte = 0,
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT
	csc_st_word,
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT
	csc_st_dword,
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT
	csc_st_qword
#endif
#endif
#endif
} CSC_SizeType;

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemMove(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_PVOID pChunkDst;
	CSC_PCVOID pChunkSrc;
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT
	CSC_SizeType smallerSizeType;
#endif
	CSC_BOOLEAN reverseDir = ((CONST CSC_BYTE* CONST)pDst > (CONST CSC_BYTE* CONST)pSrc && (CONST CSC_BYTE* CONST)pDst < (CONST CSC_BYTE* CONST)pSrc + size) ? (CSC_BOOLEAN)TRUE : (CSC_BOOLEAN)FALSE;

	if (!pDst || !pSrc || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDst == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		smallerSizeType = csc_st_byte;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		smallerSizeType = csc_st_word;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		smallerSizeType = csc_st_dword;
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsMemMove(pDst, pSrc, size, smallerSizeType);
	}

#if CSC_MEMORY_UTILS_USE_UNALIGNED_ACCESS == FALSE

	if ((CSC_DATA_POINTER_TYPE)pDst % elementSize != (CSC_DATA_POINTER_TYPE)pSrc % elementSize)
	{
		return CSC_MemoryUtilsMemMove(pDst, pSrc, size, smallerSizeType);
	}

#endif

	prefixLength = (CSC_SIZE_T)((reverseDir) ? ((CSC_DATA_POINTER_TYPE)pDst + size) : (elementSize - ((CSC_DATA_POINTER_TYPE)pDst % elementSize))) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkDst = (CSC_PVOID)((CSC_BYTE* CONST)pDst + ((reverseDir) ? suffixLength : prefixLength));
	pChunkSrc = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pSrc + ((reverseDir) ? suffixLength : prefixLength));

	if (prefixLength)
	{
		if (reverseDir)
		{
			CSC_MemoryUtilsMemMove((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSrc + chunkLength), prefixLength, smallerSizeType);
		}
		else
		{
			CSC_MemoryUtilsMemMove(pDst, pSrc, prefixLength, smallerSizeType);
		}
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{
		case csc_st_word:

			if (reverseDir)
			{
				((CSC_WORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_WORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_WORD* CONST)pChunkDst)[iterator] = ((CONST CSC_WORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			if (reverseDir)
			{
				((CSC_DWORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_DWORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_DWORD* CONST)pChunkDst)[iterator] = ((CONST CSC_DWORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			if (reverseDir)
			{
				((CSC_QWORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_QWORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_QWORD* CONST)pChunkDst)[iterator] = ((CONST CSC_QWORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		if (reverseDir)
		{
			CSC_MemoryUtilsMemMove(pDst, pSrc, suffixLength, smallerSizeType);
		}
		else
		{
			CSC_MemoryUtilsMemMove((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSrc + chunkLength), suffixLength, smallerSizeType);
		}
	}

#endif

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemMoveNonRecursive(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_PVOID pChunkDst;
	CSC_PCVOID pChunkSrc;
	CSC_BOOLEAN reverseDir = ((CONST CSC_BYTE* CONST)pDst > (CONST CSC_BYTE* CONST)pSrc && (CONST CSC_BYTE* CONST)pDst < (CONST CSC_BYTE* CONST)pSrc + size) ? (CSC_BOOLEAN)TRUE : (CSC_BOOLEAN)FALSE;

	if (!pDst || !pSrc || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDst == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
	}

#if CSC_MEMORY_UTILS_USE_UNALIGNED_ACCESS == FALSE

	if ((CSC_DATA_POINTER_TYPE)pDst % elementSize != (CSC_DATA_POINTER_TYPE)pSrc % elementSize)
	{
		return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
	}

#endif

	prefixLength = (CSC_SIZE_T)((reverseDir) ? ((CSC_DATA_POINTER_TYPE)pDst + size) : (elementSize - ((CSC_DATA_POINTER_TYPE)pDst % elementSize))) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkDst = (CSC_PVOID)((CSC_BYTE* CONST)pDst + ((reverseDir) ? suffixLength : prefixLength));
	pChunkSrc = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pSrc + ((reverseDir) ? suffixLength : prefixLength));

	if (prefixLength)
	{
		if (reverseDir)
		{
			CSC_MemoryUtilsMemMoveBasic((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSrc + chunkLength), prefixLength);
		}
		else
		{
			CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, prefixLength);
		}
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{
		case csc_st_word:

			if (reverseDir)
			{
				((CSC_WORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_WORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_WORD* CONST)pChunkDst)[iterator] = ((CONST CSC_WORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			if (reverseDir)
			{
				((CSC_DWORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_DWORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_DWORD* CONST)pChunkDst)[iterator] = ((CONST CSC_DWORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			if (reverseDir)
			{
				((CSC_QWORD* CONST)pChunkDst)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1] = ((CONST CSC_QWORD* CONST)pChunkSrc)[(chunkLength / elementSize) - iterator - (CSC_SIZE_T)1];
			}
			else
			{
				((CSC_QWORD* CONST)pChunkDst)[iterator] = ((CONST CSC_QWORD* CONST)pChunkSrc)[iterator];
			}
				
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		if (reverseDir)
		{
			CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, suffixLength);
		}
		else
		{
			CSC_MemoryUtilsMemMoveBasic((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSrc + chunkLength), suffixLength);
		}
	}

#endif

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemSet(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_PVOID pChunkDst;
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT
	CSC_SizeType smallerSizeType;
	CONST CSC_WORD wordValue = (CSC_WORD)value + ((CSC_WORD)value << (CSC_WORD)0x8);
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT
	CONST CSC_DWORD dwordValue = (CSC_DWORD)wordValue + ((CSC_DWORD)wordValue << (CSC_DWORD)0x10);
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT
	CONST CSC_QWORD qwordValue = (CSC_QWORD)dwordValue + ((CSC_QWORD)dwordValue << (CSC_QWORD)0x20);
#endif
#endif
#endif

	if (!pDst || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsMemSetBasic(pDst, value, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		smallerSizeType = csc_st_byte;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		smallerSizeType = csc_st_word;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		smallerSizeType = csc_st_dword;
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsMemSet(pDst, value, size, smallerSizeType);
	}

	prefixLength = (elementSize - (CSC_SIZE_T)((CSC_DATA_POINTER_TYPE)pDst % elementSize)) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkDst = (CSC_PVOID)((CSC_BYTE* CONST)pDst + prefixLength);

	if (prefixLength)
	{
		CSC_MemoryUtilsMemSet(pDst, value, prefixLength, smallerSizeType);
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{

		case csc_st_word:

			((CSC_WORD* CONST)pChunkDst)[iterator] = wordValue;
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			((CSC_DWORD* CONST)pChunkDst)[iterator] = dwordValue;
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			((CSC_QWORD* CONST)pChunkDst)[iterator] = qwordValue;
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		CSC_MemoryUtilsMemSet((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), value, suffixLength, smallerSizeType);
	}

#endif

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsMemSetNonRecursive(_When_(return == CSC_STATUS_SUCCESS, _Out_) CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_PVOID pChunkDst;
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT
	CONST CSC_WORD wordValue = (CSC_WORD)value + ((CSC_WORD)value << (CSC_WORD)0x8);
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT
	CONST CSC_DWORD dwordValue = (CSC_DWORD)wordValue + ((CSC_DWORD)wordValue << (CSC_DWORD)0x10);
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT
	CONST CSC_QWORD qwordValue = (CSC_QWORD)dwordValue + ((CSC_QWORD)dwordValue << (CSC_QWORD)0x20);
#endif
#endif
#endif

	if (!pDst || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsMemSetBasic(pDst, value, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsMemSetBasic(pDst, value, size);
	}

	prefixLength = (elementSize - (CSC_SIZE_T)((CSC_DATA_POINTER_TYPE)pDst % elementSize)) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkDst = (CSC_PVOID)((CSC_BYTE* CONST)pDst + prefixLength);

	if (prefixLength)
	{
		CSC_MemoryUtilsMemSetBasic(pDst, value, prefixLength);
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{
		case csc_st_word:

			((CSC_WORD* CONST)pChunkDst)[iterator] = wordValue;
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			((CSC_DWORD* CONST)pChunkDst)[iterator] = dwordValue;
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			((CSC_QWORD* CONST)pChunkDst)[iterator] = qwordValue;
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		CSC_MemoryUtilsMemSetBasic((CONST CSC_PVOID)((CSC_BYTE* CONST)pChunkDst + chunkLength), value, suffixLength);
	}

#endif

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemoryRecursive(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_STATUS status;
	CSC_PCVOID pChunkFirst;
	CSC_PCVOID pChunkSecond;
#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT
	CSC_SizeType smallerSizeType;
#endif

	if (!pFirst || !pSecond || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		smallerSizeType = csc_st_byte;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		smallerSizeType = csc_st_word;
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		smallerSizeType = csc_st_dword;
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, size, smallerSizeType);
	}

#if CSC_MEMORY_UTILS_USE_UNALIGNED_ACCESS == FALSE

	if ((CSC_DATA_POINTER_TYPE)pFirst % elementSize != (CSC_DATA_POINTER_TYPE)pSecond % elementSize)
	{
		return CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, size, smallerSizeType);
	}

#endif

	prefixLength = (elementSize - (CSC_SIZE_T)((CSC_DATA_POINTER_TYPE)pFirst % elementSize)) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkFirst = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pFirst + prefixLength);
	pChunkSecond = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pSecond + prefixLength);

	if (prefixLength)
	{
		status = CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, prefixLength, smallerSizeType);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{
		case csc_st_word:

			if (((CONST CSC_WORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_WORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			if (((CONST CSC_DWORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_DWORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			if (((CONST CSC_QWORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_QWORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		status = CSC_MemoryUtilsCompareMemoryRecursive((CONST CSC_PCVOID)((CSC_BYTE* CONST)pChunkFirst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSecond + chunkLength), suffixLength, smallerSizeType);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

#endif

	return CSC_STATUS_SUCCESS;
}

static CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemoryNonRecursive(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_SizeType sizeType)
{
	CSC_SIZE_T elementSize, iterator, prefixLength, suffixLength, chunkLength;
	CSC_STATUS status;
	CSC_PCVOID pChunkFirst;
	CSC_PCVOID pChunkSecond;

	if (!pFirst || !pSecond || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}

	switch (sizeType)
	{
	case csc_st_byte:

		return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	case csc_st_word:

		elementSize = sizeof(CSC_WORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

	case csc_st_dword:

		elementSize = sizeof(CSC_DWORD);
		break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

	case csc_st_qword:

		elementSize = sizeof(CSC_QWORD);
		break;

#endif
#endif
#endif

	default:

		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_8BIT

	if (size < elementSize)
	{
		return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);
	}

#if CSC_MEMORY_UTILS_USE_UNALIGNED_ACCESS == FALSE

	if ((CSC_DATA_POINTER_TYPE)pFirst % elementSize != (CSC_DATA_POINTER_TYPE)pSecond % elementSize)
	{
		return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);
	}

#endif

	prefixLength = (elementSize - (CSC_SIZE_T)((CSC_DATA_POINTER_TYPE)pFirst % elementSize)) % elementSize;
	suffixLength = (size - prefixLength) % elementSize;
	chunkLength = size - prefixLength - suffixLength;
	pChunkFirst = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pFirst + prefixLength);
	pChunkSecond = (CSC_PCVOID)((CONST CSC_BYTE* CONST)pSecond + prefixLength);

	if (prefixLength)
	{
		status = CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, prefixLength);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

	for (iterator = (CSC_SIZE_T)0; iterator < (chunkLength / elementSize); ++iterator)
	{
		switch (sizeType)
		{
		case csc_st_word:

			if (((CONST CSC_WORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_WORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_16BIT

		case csc_st_dword:

			if (((CONST CSC_DWORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_DWORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#if CSC_DATA_BUS_WIDTH > CSC_DATA_BUS_WIDTH_32BIT

		case csc_st_qword:

			if (((CONST CSC_QWORD* CONST)pChunkFirst)[iterator] != ((CONST CSC_QWORD* CONST)pChunkSecond)[iterator])
			{
				return CSC_STATUS_GENERAL_FAILURE;
			}
				
			break;

#endif
#endif

		default:

			return CSC_STATUS_GENERAL_FAILURE;
		}
	}

	if (suffixLength)
	{
		status = CSC_MemoryUtilsCompareMemoryBasic((CONST CSC_PCVOID)((CSC_BYTE* CONST)pChunkFirst + chunkLength), (CONST CSC_PCVOID)((CONST CSC_BYTE* CONST)pChunkSecond + chunkLength), suffixLength);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

#endif

	return CSC_STATUS_SUCCESS;
}

#endif


CSC_STATUS CSCAPI CSC_MemoryUtilsCopyMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pSrc, _In_ CONST CSC_SIZE_T size)
{
	if (!pDst || !pSrc || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDst == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}
	
#if CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION == FALSE
	return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
#else
#if CSC_MEMORY_UTILS_USE_RECURSION == FALSE
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsMemMoveNonRecursive(pDst, pSrc, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsMemMoveNonRecursive(pDst, pSrc, size, csc_st_dword);
#else
	return CSC_MemoryUtilsMemMoveNonRecursive(pDst, pSrc, size, csc_st_qword);
#endif
#endif
#endif
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsMemMoveBasic(pDst, pSrc, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsMemMove(pDst, pSrc, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsMemMove(pDst, pSrc, size, csc_st_dword);
#else
	return CSC_MemoryUtilsMemMove(pDst, pSrc, size, csc_st_qword);
#endif
#endif
#endif
#endif
#endif
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_BYTE value, _In_ CONST CSC_SIZE_T size)
{
	if (!pDst || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

#if CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION == FALSE
	return CSC_MemoryUtilsMemSetBasic(pDst, value, size);
#else
#if CSC_MEMORY_UTILS_USE_RECURSION == FALSE
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsMemSetBasic(pDst, value, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsMemSetNonRecursive(pDst, value, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsMemSetNonRecursive(pDst, value, size, csc_st_dword);
#else
	return CSC_MemoryUtilsMemSetNonRecursive(pDst, value, size, csc_st_qword);
#endif
#endif
#endif
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsMemSetBasic(pDst, value, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsMemSet(pDst, value, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsMemSet(pDst, value, size, csc_st_dword);
#else
	return CSC_MemoryUtilsMemSet(pDst, value, size, csc_st_qword);
#endif
#endif
#endif
#endif
#endif
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetZeroMemory(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_SIZE_T size)
{
	if (!pDst || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}
	else
	{
		return CSC_MemoryUtilsSetMemory(pDst, (CSC_BYTE)0, size);
	}
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSetArrayValue(_Out_ CONST CSC_PVOID pDst, _In_ CONST CSC_PCVOID pValue, _In_ CONST CSC_SIZE_T elementSize, _In_ CONST CSC_SIZE_T elementCount)
{
	CSC_STATUS status;
	CSC_SIZE_T iterator;

	if (!pDst || !pValue || !elementSize || !elementCount)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	for (iterator = (CSC_SIZE_T)0; iterator < elementCount; ++iterator)
	{
		status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)((CSC_BYTE* CONST)pDst + iterator* elementSize), pValue, elementSize);

		if (status != CSC_STATUS_SUCCESS)
		{
			return status;
		}
	}

	return CSC_STATUS_SUCCESS;
}


CSC_STATUS CSCAPI CSC_MemoryUtilsCompareMemory(_In_ CONST CSC_PCVOID pFirst, _In_ CONST CSC_PCVOID pSecond, _In_ CONST CSC_SIZE_T size)
{
	if (!pFirst || !pSecond || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}

#if CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION == FALSE
	return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);
#else
#if CSC_MEMORY_UTILS_USE_RECURSION == FALSE
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsCompareMemoryNonRecursive(pFirst, pSecond, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsCompareMemoryNonRecursive(pFirst, pSecond, size, csc_st_dword);
#else
	return CSC_MemoryUtilsCompareMemoryNonRecursive(pFirst, pSecond, size, csc_st_qword);
#endif
#endif
#endif
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_16BIT
	return CSC_MemoryUtilsCompareMemoryBasic(pFirst, pSecond, size);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_32BIT
	return CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, size, csc_st_word);
#else
#if CSC_DATA_BUS_WIDTH < CSC_DATA_BUS_WIDTH_64BIT
	return CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, size, csc_st_dword);
#else
	return CSC_MemoryUtilsCompareMemoryRecursive(pFirst, pSecond, size, csc_st_qword);
#endif
#endif
#endif
#endif
#endif
}


CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValues(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_IAllocator* CONST pIAllocator)
{
	if (!pFirst || !pSecond || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}
	
	if (size > (CSC_SIZE_T)0x100)
	{
		return CSC_MemoryUtilsSwapValuesHeap(pFirst, pSecond, size, pIAllocator);
	}

	return CSC_MemoryUtilsSwapValues256(pFirst, pSecond, size);
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValuesHeap(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size, _In_ CONST CSC_IAllocator* CONST pIAllocator)
{
	CSC_STATUS status;
	CSC_PVOID pBuffer = NULL;

	if (!pFirst || !pSecond || !size || !pIAllocator)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}
	
	if (!CSC_IAllocatorIsUsable(pIAllocator))
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	pBuffer = CSC_IAllocatorAlloc(pIAllocator, size);

	if (!pBuffer)
	{
		return CSC_STATUS_GENERAL_FAILURE;
	}

	status = CSC_MemoryUtilsCopyMemory(pBuffer, pFirst, size);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_IAllocatorFree(pIAllocator, pBuffer);
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory(pFirst, pSecond, size);

	if (status != CSC_STATUS_SUCCESS)
	{
		CSC_IAllocatorFree(pIAllocator, pBuffer);
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory(pSecond, pBuffer, size);

	CSC_IAllocatorFree(pIAllocator, pBuffer);
	return status;
}

CSC_STATUS CSCAPI CSC_MemoryUtilsSwapValues256(_Inout_ CONST CSC_PVOID pFirst, _Inout_ CONST CSC_PVOID pSecond, _In_ CONST CSC_SIZE_T size)
{
	CSC_BYTE buffer[(CSC_SIZE_T)0x100];
	CSC_STATUS status;

	if (!pFirst || !pSecond || !size || size > (CSC_SIZE_T)0x100)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pFirst == pSecond)
	{
		return CSC_STATUS_SUCCESS;
	}

	status = CSC_MemoryUtilsCopyMemory((CSC_PVOID)buffer, (CSC_PCVOID)pFirst, size);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	status = CSC_MemoryUtilsCopyMemory(pFirst, (CSC_PCVOID)pSecond, size);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	return CSC_MemoryUtilsCopyMemory(pSecond, (CSC_PCVOID)buffer, size);
}

CSC_STATUS CSCAPI CSC_MemoryUtilsMoveValue(_Out_ CONST CSC_PVOID pDst, _Inout_ CONST CSC_PVOID pSrc, _In_ CONST CSC_SIZE_T size)
{
	CSC_STATUS status;
	CSC_PVOID pDeletionBase = pSrc;
	CSC_SIZE_T deletionSize = size;

	if (!pDst || !pSrc || !size)
	{
		return CSC_STATUS_INVALID_PARAMETER;
	}

	if (pDst == pSrc)
	{
		return CSC_STATUS_SUCCESS;
	}

	status = CSC_MemoryUtilsCopyMemory(pDst, (CSC_PCVOID)pSrc, size);

	if (status != CSC_STATUS_SUCCESS)
	{
		return status;
	}

	if ((CONST CSC_BYTE* CONST)pDst > (CONST CSC_BYTE* CONST)pSrc && (CONST CSC_BYTE* CONST)pDst < (CONST CSC_BYTE* CONST)pSrc + size)
	{
		deletionSize = (CSC_SIZE_T)((CONST CSC_BYTE* CONST)pDst - (CONST CSC_BYTE* CONST)pSrc);
	}
	
	if ((CONST CSC_BYTE* CONST)pSrc > (CONST CSC_BYTE* CONST)pDst && (CONST CSC_BYTE* CONST)pSrc < (CONST CSC_BYTE* CONST)pDst + size)
	{
		deletionSize = (CSC_SIZE_T)((CONST CSC_BYTE* CONST)pSrc - (CONST CSC_BYTE* CONST)pDst);
		pDeletionBase = (CSC_PVOID)((CONST CSC_BYTE* CONST)pSrc + (size - deletionSize));
	}

	return CSC_MemoryUtilsSetZeroMemory(pDeletionBase, deletionSize);
}
