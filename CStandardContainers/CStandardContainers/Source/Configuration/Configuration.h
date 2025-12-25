#ifndef CSC_CONFIGURATION
#define CSC_CONFIGURATION

/*
Description:
The Configuration.h file contains the main configuration used by the library to adjust to the underlying system/architecture.
It contains a default configuration which can be copied and adjusted to implement a custom configuration.
The default configuration currently assumes a 64-Bit Von-Neumann architecture with a data- and address-width of both 64-Bit.
Configurations are activated through a #define preprocessor macro. WARNING: Only one configuration should be active!
Inactive configurations have their #define macro commented out.
*/

// Definition of the various data bus widths supported by the library.
#define CSC_DATA_BUS_WIDTH_8BIT 0x8
#define CSC_DATA_BUS_WIDTH_16BIT 0x10
#define CSC_DATA_BUS_WIDTH_32BIT 0x20
#define CSC_DATA_BUS_WIDTH_64BIT 0x40

// Definition of the various data bus address widths supported by the library.
#define CSC_ADDRESS_BUS_WIDTH_DATA_16BIT 0x10
#define CSC_ADDRESS_BUS_WIDTH_DATA_32BIT 0x20
#define CSC_ADDRESS_BUS_WIDTH_DATA_64BIT 0x40

// Definition of the various code bus address widths supported by the library.
#define CSC_ADDRESS_BUS_WIDTH_CODE_16BIT 0x10
#define CSC_ADDRESS_BUS_WIDTH_CODE_32BIT 0x20
#define CSC_ADDRESS_BUS_WIDTH_CODE_64BIT 0x40

// Define macros for the configurations available.
// Only one configuration can be active at a time.
#define CSC_CONFIG_DEFAULT
//#define CSC_CONFIG_WINDOWS_NATIVE

// Beginning of the default configuration.
// Can be used as a template to implement custom configurations.
#ifdef CSC_CONFIG_DEFAULT

// Macro definitions for selecting the calling conventions for methods and functions.
#define CSCAPI
#define CSCMETHOD

// Macro for the const keyword to match the Windows coding style of the library.
#define CONST const

// Macro definitions that can be used to implement annotions, if supported by the compiler.
#define _In_
#define _In_opt_
#define _Inout_
#define _Out_
#define _Out_opt_

#define _When_(x)

// Default configuration assumes a data width of 64-Bit.
#define CSC_DATA_BUS_WIDTH CSC_DATA_BUS_WIDTH_64BIT

// Decision logic for assigning the basic integer types supported by the library.
#if CSC_DATA_BUS_WIDTH == CSC_DATA_BUS_WIDTH_8BIT
typedef unsigned char CSC_UINT;
typedef signed char CSC_INT;
#elif CSC_DATA_BUS_WIDTH == CSC_DATA_BUS_WIDTH_16BIT
typedef unsigned short CSC_UINT;
typedef signed short CSC_INT;
#elif CSC_DATA_BUS_WIDTH == CSC_DATA_BUS_WIDTH_32BIT
typedef unsigned long CSC_UINT;
typedef signed long CSC_INT;
#elif CSC_DATA_BUS_WIDTH == CSC_DATA_BUS_WIDTH_64BIT
typedef unsigned long long CSC_UINT;
typedef signed long long CSC_INT;
#else
typedef unsigned long long CSC_UINT;
typedef signed long long CSC_INT;
#endif

// Default configuration assumes a Von-Neumann architecture with a 64-Bit address bus.
#define CSC_ADDRESS_BUS_WIDTH_CODE CSC_ADDRESS_BUS_WIDTH_CODE_64BIT

// Decision logic for assigning the integer type that can be used to hold code pointers.
#if CSC_ADDRESS_BUS_WIDTH_CODE == CSC_ADDRESS_BUS_WIDTH_CODE_16BIT
typedef unsigned short CSC_CODE_POINTER_TYPE;
#elif CSC_ADDRESS_BUS_WIDTH_CODE == CSC_ADDRESS_BUS_WIDTH_CODE_32BIT
typedef unsigned long CSC_CODE_POINTER_TYPE;
#elif CSC_ADDRESS_BUS_WIDTH_CODE == CSC_ADDRESS_BUS_WIDTH_CODE_64BIT
typedef unsigned long long CSC_CODE_POINTER_TYPE;
#else
typedef unsigned long long CSC_CODE_POINTER_TYPE;
#endif

// Default configuration assumes a Von-Neumann architecture with a 64-Bit address bus.
#define CSC_ADDRESS_BUS_WIDTH_DATA CSC_ADDRESS_BUS_WIDTH_DATA_64BIT

// Decision logic for assigning the integer type that can be used to hold data pointers.
#if CSC_ADDRESS_BUS_WIDTH_DATA == CSC_ADDRESS_BUS_WIDTH_DATA_16BIT
typedef unsigned short CSC_DATA_POINTER_TYPE;
#elif CSC_ADDRESS_BUS_WIDTH_DATA == CSC_ADDRESS_BUS_WIDTH_DATA_32BIT
typedef unsigned long CSC_DATA_POINTER_TYPE;
#elif CSC_ADDRESS_BUS_WIDTH_DATA == CSC_ADDRESS_BUS_WIDTH_DATA_64BIT
typedef unsigned long long CSC_DATA_POINTER_TYPE;
#else
typedef unsigned long long CSC_DATA_POINTER_TYPE;
#endif

// Definition of the unsigned integer type that has the same width as a data pointer.
// Should not be used to hold or compare pointers itself, rather for offset calculations or indeces that have to match data pointer dimensions.
typedef CSC_DATA_POINTER_TYPE CSC_SIZE_T;

// Definition of types used to hold characters of ANSI, UCS-2 or UTF-16 strings.
typedef unsigned char CSC_CHAR;
typedef unsigned short CSC_WCHAR;

// Definition of fixed-size unsigned integer types corresponding to assembly instructions (x86-Style).
typedef unsigned char CSC_BYTE;
typedef unsigned short CSC_WORD;
typedef unsigned long CSC_DWORD;
typedef unsigned long long CSC_QWORD;

// Definitions for type agnostic pointers used by the library.
typedef void* CSC_PVOID;
typedef CONST void* CSC_PCVOID;

// Definition of return types used by the library.
typedef signed long CSC_STATUS;
typedef unsigned char CSC_BOOLEAN;

// Definition of various status codes for the CSC_STATUS type.
#define CSC_STATUS_SUCCESS (CSC_STATUS)0
#define CSC_STATUS_GENERAL_FAILURE (CSC_STATUS)-1
#define CSC_STATUS_INVALID_PARAMETER (CSC_STATUS)-2
#define CSC_STATUS_INVALID_HANDLE (CSC_STATUS)-3
#define CSC_STATUS_MEMORY_NOT_ALLOCATED (CSC_STATUS)-4

// Definition of the null pointer.
#define NULL (CSC_PVOID)0

// Definition of the values true and false for the CSC_BOOLEAN type.
#define FALSE (CSC_BOOLEAN)0
#define TRUE (CSC_BOOLEAN)1

// Definitions used to adjust the behaviour of the MemoryUtils.
#define CSC_MEMORY_UTILS_USE_RECURSION FALSE
#define CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION FALSE

#endif CSC_CONFIG_DEFAULT


// Beginning of the Windows native configuration.
// Supports x86-32 and x86-64 architectures, potentially also ARM32 architectures.
#ifdef CSC_CONFIG_WINDOWS_NATIVE

#include <Windows.h>
#include <ntstatus.h>

#define FASTCALL __fastcall

#define CSCAPI WINAPI
#define CSCMETHOD FASTCALL

#ifdef _WIN64
#define CSC_DATA_BUS_WIDTH CSC_DATA_BUS_WIDTH_64BIT
#define CSC_ADDRESS_BUS_WIDTH_CODE CSC_ADDRESS_BUS_WIDTH_CODE_64BIT
#define CSC_ADDRESS_BUS_WIDTH_DATA CSC_ADDRESS_BUS_WIDTH_DATA_64BIT
typedef QWORD CSC_DATA_POINTER_TYPE;
#else
#define CSC_DATA_BUS_WIDTH CSC_DATA_BUS_WIDTH_32BIT
#define CSC_ADDRESS_BUS_WIDTH_CODE CSC_ADDRESS_BUS_WIDTH_CODE_32BIT
#define CSC_ADDRESS_BUS_WIDTH_DATA CSC_ADDRESS_BUS_WIDTH_DATA_32BIT
typedef DWORD CSC_DATA_POINTER_TYPE;
#endif _WIN64

typedef CSC_DATA_POINTER_TYPE CSC_CODE_POINTER_TYPE;

typedef DWORD64 QWORD;

typedef UINT CSC_UINT;
typedef INT CSC_INT;

typedef CHAR CSC_CHAR;
typedef WCHAR CSC_WCHAR;

typedef BYTE CSC_BYTE;
typedef WORD CSC_WORD;
typedef DWORD CSC_DWORD;
typedef QWORD CSC_QWORD;

typedef PVOID CSC_PVOID;
typedef const void* CSC_PCVOID;

typedef SIZE_T CSC_SIZE_T;

typedef NTSTATUS CSC_STATUS;
typedef BOOLEAN CSC_BOOLEAN;

#define CSC_STATUS_SUCCESS STATUS_SUCCESS
#define CSC_STATUS_GENERAL_FAILURE STATUS_ACCESS_DENIED
#define CSC_STATUS_INVALID_PARAMETER STATUS_INVALID_PARAMETER
#define CSC_STATUS_INVALID_HANDLE STATUS_INVALID_HANDLE
#define CSC_STATUS_MEMORY_NOT_ALLOCATED STATUS_MEMORY_NOT_ALLOCATED

#define CSC_MEMORY_UTILS_USE_RECURSION FALSE
#define CSC_MEMORY_UTILS_USE_SIZE_BASED_OPTIMIZATION TRUE

#endif CSC_CONFIG_WINDOWS_NATIVE

#endif CSC_CONFIGURATION

