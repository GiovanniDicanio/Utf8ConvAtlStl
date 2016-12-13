////////////////////////////////////////////////////////////////////////////////
//
// Utf8ConvTest.cpp -- Copyright (C) by Giovanni Dicanio
//
// Unit Test for the UTF-8 encoding conversion functions.
//
////////////////////////////////////////////////////////////////////////////////


#include "Utf8Conv.h"   // UTF-8 conversion functions to test
#include <stdio.h>      // For console output
#include <exception>    // For std::exception

using namespace GiovanniDicanio;


//
// Tests with gigantic strings (whose lengths expressed in size_t can't fit
// into an int and can't be passed to the MultiByteToWideChar 
// and WideCharToMultiByte Win32 APIs), enabled by default in 64-bit builds.
//
// You can override that commenting out the following lines:
//
#ifdef _WIN64
#define TEST_GIGANTIC_STRINGS
#endif


namespace
{

//------------------------------------------------------------------------------
// Helper function to print a test error message.
// Don't use it in test case code (use the TEST_ERROR macro instead).
//------------------------------------------------------------------------------
void PrintTestError(const char* const file, const int line, const char* const msg)
{
    printf("[ERROR] %s (%d): %s\n", file, line, msg);
}

} // anonymous namespace


//------------------------------------------------------------------------------
// Macro to print test error messages, and increase the global error count.
// Use it in test cases to log failed tests.
//------------------------------------------------------------------------------
#define TEST_ERROR(msg)                             \
    do                                              \
    {                                               \
        ++g_testErrors;                             \
        PrintTestError(__FILE__, __LINE__, (msg));  \
    }                                               \
    __pragma(warning(suppress:4127)) while (0)


// Count of test errors
static int g_testErrors = 0;

// Entry point for tests
void RunTests();


//------------------------------------------------------------------------------
// Test console application's entry point
//------------------------------------------------------------------------------
int main()
{
    constexpr int kExitOk = 0;
    constexpr int kExitError = 1;
    int exitCode = kExitOk;

    try
    {
        printf("\nTesting UTF-8/UTF-16 STL/ATL Conversion Helpers\n");
        printf("           -- by Giovanni Dicanio --\n\n");
        RunTests();
    }
    catch (const CAtlException& e)
    {
        printf("\n*** FATAL: CAtlException; hr=0x%08X\n", static_cast<HRESULT>(e));
        exitCode = kExitError;
    }
    catch (const std::exception& e)
    {
        printf("\n*** FATAL: std::exception; what(): %s\n", e.what());
        exitCode = kExitError;
    }
    catch (...)
    {
        printf("\n*** FATAL: Unknown C++ exception.\n");
        exitCode = kExitError;
    }

    if (g_testErrors != 0)
    {
        printf("\n*** %d error(s) detected.\n", g_testErrors);
        exitCode = kExitError;
    }

    if (exitCode == kExitOk)
    {
        // All right!! :)
        printf("\n*** No errors detected! :) ***\n");
    }

    return exitCode;
}


//------------------------------------------------------------------------------
// Various Tests
//------------------------------------------------------------------------------

void TestBasicConversionsWithStlStrings()
{
    CStringW s1u16 = L"Hello world";
    std::string s1u8 = win32::Utf16ToUtf8(s1u16);
    CStringW s1u16back = win32::Utf8ToUtf16(s1u8);
    if (s1u16back != s1u16)
    {
        TEST_ERROR("Converting from UTF-16 to UTF-8 and back gives different string.");
    }

    std::string s2u8 = "Ciao ciao";
    CStringW s2u16 = win32::Utf8ToUtf16(s2u8);
    std::string s2u8back = win32::Utf16ToUtf8(s2u16);
    if (s2u8back != s2u8)
    {
        TEST_ERROR("Converting from UTF-8 to UTF-16 and back gives different string.");
    }
}


void TestBasicConversionWithRawPointers()
{
    const wchar_t* const s1u16 = L"Hello world";
    std::string s1u8 = win32::Utf16ToUtf8(s1u16);
    CStringW s1u16back = win32::Utf8ToUtf16(s1u8);
    if (s1u16back != s1u16)
    {
        TEST_ERROR("Converting raw pointers from UTF-16 to UTF-8 and back gives different string.");
    }

    const char* const s2u8 = "Ciao ciao";
    CStringW s2u16 = win32::Utf8ToUtf16(s2u8);
    std::string s2u8back = win32::Utf16ToUtf8(s2u16);
    if (s2u8back != s2u8)
    {
        TEST_ERROR("Converting raw pointers from UTF-8 to UTF-16 and back gives different string.");
    }
}


void TestEmptyStringConversions()
{
    const CStringW u16empty;
    const std::string u8empty;

    if (!win32::Utf16ToUtf8(u16empty).empty())
    {
        TEST_ERROR("Empty UTF-16 string is not converted to an empty UTF-8.");
    }

    if (!win32::Utf8ToUtf16(u8empty).IsEmpty())
    {
        TEST_ERROR("Empty UTF-8 string is not converted to an empty UTF-16.");
    }

    if (!win32::Utf16ToUtf8(L"").empty())
    {
        TEST_ERROR("Empty UTF-16 raw string ptr is not converted to an empty UTF-8.");
    }

    if (!win32::Utf8ToUtf16("").IsEmpty())
    {
        TEST_ERROR("Empty UTF-8 raw string ptr is not converted to an empty UTF-16.");
    }
}


void TestJapaneseKin()
{
    //
    // Test "kin"
    // UTF-16: 91D1
    // UTF-8:  E9 87 91
    //

    const std::string kinU8 = "\xE9\x87\x91";
    const CStringW kinU16 = L"\x91D1";
    if (win32::Utf8ToUtf16(kinU8) != kinU16)
    {
        TEST_ERROR("Converting Japanese 'kin' from UTF-8 to UTF-16 failed.");
    }

    if (win32::Utf16ToUtf8(kinU16) != kinU8)
    {
        TEST_ERROR("Converting Japanese 'kin' from UTF-16 to UTF-8 failed.");
    }
}


void TestInvalidUnicodeSequences()
{
    try
    {
        // String containing invalid UTF-8
        const std::string invalidUtf8 = "Invalid UTF-8 follows: \xC0\x76\x77";

        // The following line should throw because of invalid UTF-8 sequence
        // in input string
        CStringW invalidUtf16 = win32::Utf8ToUtf16(invalidUtf8);

        // Correct throwing code should *not* get here:
        TEST_ERROR("CAtlException not thrown in presence of invalid UTF-8.");
    }
    catch (const CAtlException& e)
    {
        const HRESULT expectedErrorCode = HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
        if (static_cast<HRESULT>(e) != expectedErrorCode)
        {
            TEST_ERROR("Error code different than ERROR_NO_UNICODE_TRANSLATION.");
        }
    }

    try
    {
        // String containing invalid UTF-16
        const CStringW invalidUtf16 = L"Invalid UTF-16: \xD800\x0100";

        // The following line should throw because of invalid UTF-16 sequence
        // in input string
        std::string invalidUtf8 = win32::Utf16ToUtf8(invalidUtf16);

        // Correct throwing code should *not* get here:
        TEST_ERROR("CAtlException not thrown in presence of invalid UTF-16.");
    }
    catch (const CAtlException& e)
    {
        const HRESULT expectedErrorCode = HRESULT_FROM_WIN32(ERROR_NO_UNICODE_TRANSLATION);
        if (static_cast<HRESULT>(e) != expectedErrorCode)
        {
            TEST_ERROR("Error code different than ERROR_NO_UNICODE_TRANSLATION.");
        }
    }
}


#ifdef TEST_GIGANTIC_STRINGS
void TestGiganticStrings()
{
    try
    {
        // Build a gigantic std::string
        constexpr size_t giga = 1ULL * 1024 * 1024 * 1024;
        const std::string hugeUtf8(5 * giga, 'C');
        
        // This code should throw because of the gigantic std::string 
        CStringW hugeUtf16 = win32::Utf8ToUtf16(hugeUtf8);

        // Correct code should *not* get here:
        TEST_ERROR("CAtlException not thrown in presence of UTF-8 string whose length can't fit into an int.");
    }
    catch (const CAtlException& e)
    {
        // All right
        printf("\nHuge UTF-8 string throwing CAtlException as expected; hr=0x%08X\n", 
            static_cast<HRESULT>(e));
    }

    // NOTE:
    // Can't have a huge CStringW, as CString's lengths are expressed using int 
    // (i.e. signed integers; not size_t).
}
#endif // TEST_GIGANTIC_STRINGS


// Run all tests
void RunTests()
{
    TestBasicConversionsWithStlStrings();
    TestBasicConversionWithRawPointers();
    TestEmptyStringConversions();
    TestJapaneseKin();
    TestInvalidUnicodeSequences();

#ifdef TEST_GIGANTIC_STRINGS
    TestGiganticStrings();
#endif // TEST_GIGANTIC_STRINGS
}

