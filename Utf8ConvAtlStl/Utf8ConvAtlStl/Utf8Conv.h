#ifndef GIOVANNI_DICANIO_INCLUDE_UTF8CONV_H
#define GIOVANNI_DICANIO_INCLUDE_UTF8CONV_H

////////////////////////////////////////////////////////////////////////////////
//
//          Unicode UTF-8 <-> UTF-16 String Conversion Functions
//          ====================================================
// 
//                  Copyright (C) by Giovanni Dicanio 
//                   <giovanni.dicanio AT gmail.com>
// 
// Header-only module for UTF-8 (std::string) <-> UTF-16 (CStringW) conversions.
//
// It makes sense to use CStringW at the Windows ATL/MFC level for UTF-16 text,
// and std::string e.g. in cross-platform C++ code storing UTF-8 text.
// This header-only module provides a couple of conversion functions between
// these two types of strings.
// 
// Conversion errors are signaled using an /ad hoc/ exception class:
// Utf8ConversionException (derived from std::runtime_error).
// 
// For more details on the conversion process, please read my MSDN Magazine article:
// 
//      C++ - Unicode Encoding Conversions with STL Strings and Win32 APIs
//      https://msdn.microsoft.com/magazine/mt763237
// 
// The code presented here is based on that article, but uses CStringW instead
// of std::wstring for UTF-16 strings at the Windows platform level.
// 
// In addition, it's also possible to specify views of input source strings
// using an STL-style [start, finish) range.
// 
// Code developed using Visual Studio 2015.
// Compiles cleanly at /W4 in both 32-bit builds and 64-bit builds.
// 
////////////////////////////////////////////////////////////////////////////////


//
// Note: I suggest to disable implicit CString conversions between ANSI and Unicode strings,
// defining the _CSTRING_DISABLE_NARROW_WIDE_CONVERSION preprocessor macro:
// 
#ifndef _CSTRING_DISABLE_NARROW_WIDE_CONVERSION
#define _CSTRING_DISABLE_NARROW_WIDE_CONVERSION
#endif // _CSTRING_DISABLE_NARROW_WIDE_CONVERSION


#include <Windows.h>    // Win32 Platform SDK main header        

#include <limits>       // For std::numeric_limits
#include <stdexcept>    // For std::runtime_error
#include <string>       // For std::string (UTF-8)

#include <atldef.h>     // For ATLASSERT
#include <atlstr.h>     // For CStringW (UTF-16)


namespace GiovanniDicanio
{

namespace win32
{

//==============================================================================
//                  Forward Declarations and Prototypes
//==============================================================================

class Utf8ConversionException;

CStringW    Utf16FromUtf8(const std::string& utf8);
CStringW    Utf16FromUtf8(const char* utf8Start, const char* utf8Finish);
std::string Utf8FromUtf16(const CStringW& utf16);
std::string Utf8FromUtf16(const wchar_t* utf16Start, const wchar_t* utf16Finish);


//==============================================================================
//                          Implementations
//==============================================================================


//------------------------------------------------------------------------------
// Exception class representing a conversion error between UTF-8 and UTF-16.
//------------------------------------------------------------------------------
class Utf8ConversionException
    : public std::runtime_error
{
public:

    // Create the exception object from error message and error code.
    Utf8ConversionException(const char* message, DWORD errorCode)
        : std::runtime_error(message)
        , m_errorCode(errorCode)
    {}

    // Create the exception object from error message and error code.
    Utf8ConversionException(const std::string& message, DWORD errorCode)
        : std::runtime_error(message)
        , m_errorCode(errorCode)
    {}

    // Conversion error code (as returned by GetLastError).
    DWORD ErrorCode() const
    {
        return m_errorCode;
    }


    // *** PRIVATE IMPLEMENTATION ***

private:
    // Error code as returned by GetLastError
    DWORD m_errorCode;
};


//------------------------------------------------------------------------------
// Convert form UTF-8 to UTF-16.
//
// UTF-8 strings are specified using an STL-style [start, finish) range.
// UTF-16 strings are stored in CStringW.
// 
// On conversion errors (e.g. invalid UTF-8 sequence in input string), throws
// Utf8ConversionException.
//------------------------------------------------------------------------------
inline CStringW Utf16FromUtf8(const char* utf8Start, const char* utf8Finish)
{
    // Check input range parameters in debug builds
    ATLASSERT(utf8Start <= utf8Finish);

    // Special case of empty input
    if (utf8Start == utf8Finish)
    {
        // Empty input ==> empty result
        return CStringW();
    }


    // Result of the conversion
    CStringW utf16;

    // Safely fail if an invalid UTF-8 character sequence is encountered
    constexpr DWORD kFlags = MB_ERR_INVALID_CHARS;

    // Safely cast the length of the source UTF-8 string (expressed in chars)
    // from size_t to int for the MultiByteToWideChar API.
    // If the size_t value is too big to be stored into an int, 
    // throw an exception to prevent conversion errors (bugs) like huge size_t values 
    // converted to *negative* integers.
    const size_t utf8LengthUsingSizet = utf8Finish - utf8Start;
    if (utf8LengthUsingSizet > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw Utf8ConversionException(
            "Input string too long: size_t-length doesn't fit into int.\n",
            ERROR_INVALID_PARAMETER);
    }

    const int utf8Length = static_cast<int>(utf8LengthUsingSizet);

    // Get the size of the destination UTF-16 string
    const int utf16Length = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8Start,     // source UTF-8 string pointer
        utf8Length,    // length of the source UTF-8 string, in chars
        nullptr,       // unused - no conversion done in this step
        0              // request size of destination buffer, in wchar_ts
    );
    if (utf16Length == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf8ConversionException(
            "Error in converting from UTF-8 to UTF-16.\n",
            error);
    }

    // Make room in the destination string for the converted bits
    wchar_t * utf16Buffer = utf16.GetBuffer(utf16Length);
    ATLASSERT(utf16Buffer != nullptr);

    // Do the actual conversion from UTF-8 to UTF-16
    int result = ::MultiByteToWideChar(
        CP_UTF8,       // source string is in UTF-8
        kFlags,        // conversion flags
        utf8Start,     // source UTF-8 string pointer
        utf8Length,    // length of source UTF-8 string, in chars
        utf16Buffer,   // pointer to destination buffer
        utf16Length    // size of destination buffer, in wchar_ts           
    );
    if (result == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf8ConversionException(
            "Error in converting from UTF-8 to UTF-16.\n",
            error);
    }

    // Don't forget to release the internal CString's buffer
    utf16.ReleaseBuffer(utf16Length);

    // Return the converted result string
    return utf16;
}


//------------------------------------------------------------------------------
// Convert form UTF-8 to UTF-16.
//
// UTF-8 strings are stored using std::string.
// UTF-16 strings are stored in CStringW.
// 
// On conversion errors (e.g. invalid UTF-8 sequence in input string), throws
// Utf8ConversionException.
//------------------------------------------------------------------------------
inline CStringW Utf16FromUtf8(const std::string& utf8)
{
    // First, handle the special case of empty input string
    if (utf8.empty())
    {
        return CStringW();
    }

    // Delegate the conversion to the [start, finish) range overload
    const char * const utf8Start = utf8.data();
    const char * const utf8Finish = utf8Start + utf8.length();
    return Utf16FromUtf8(utf8Start, utf8Finish);
}


//------------------------------------------------------------------------------
// Convert form UTF-16 to UTF-8.
//
// UTF-16 strings are specified passing an STL-style [start, finish) range.
// UTF-8 strings are stored using std::string.
// 
// On conversion errors (e.g. invalid UTF-16 sequence in input string), throws
// Utf8ConversionException.
//------------------------------------------------------------------------------
inline std::string Utf8FromUtf16(const wchar_t* utf16Start, const wchar_t* utf16Finish)
{
    // Check input range parameters in debug builds
    ATLASSERT(utf16Start <= utf16Finish);

    // Special case of empty input
    if (utf16Start == utf16Finish)
    {
        // Empty input ==> empty result
        return std::string();
    }


    // Result of the conversion
    std::string utf8;

    // Safely fail if an invalid UTF-16 character sequence is encountered
    constexpr DWORD kFlags = WC_ERR_INVALID_CHARS;

    // Safely cast the length of the source UTF-16 string (expressed in wchar_ts)
    // from size_t to int for the WideCharToMultiByte API.
    // If the size_t value is too big to be stored into an int, 
    // throw an exception to prevent conversion errors (bugs) like huge size_t values 
    // converted to *negative* integers.
    const size_t utf16LengthUsingSizet = utf16Finish - utf16Start;
    if (utf16LengthUsingSizet > static_cast<size_t>((std::numeric_limits<int>::max)()))
    {
        throw Utf8ConversionException(
            "Input string too long: size_t-length doesn't fit into int.\n",
            ERROR_INVALID_PARAMETER);
    }

    // Length of source string view, in wchar_ts
    const int utf16Length = static_cast<int>(utf16LengthUsingSizet);

    // Get the length, in chars, of the resulting UTF-8 string
    const int utf8Length = ::WideCharToMultiByte(
        CP_UTF8,            // convert to UTF-8
        kFlags,             // conversion flags
        utf16Start,         // source UTF-16 string
        utf16Length,        // length of source UTF-16 string, in wchar_ts
        nullptr,            // unused - no conversion required in this step
        0,                  // request size of destination buffer, in chars
        nullptr, nullptr    // unused
    );
    if (utf8Length == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf8ConversionException(
            "Error in converting from UTF-16 to UTF-8.\n",
            error);
    }

    // Make room in the destination string for the converted bits
    utf8.resize(utf8Length);

    // Do the actual conversion from UTF-16 to UTF-8
    int result = ::WideCharToMultiByte(
        CP_UTF8,            // convert to UTF-8
        kFlags,             // conversion flags
        utf16Start,         // source UTF-16 string
        utf16Length,        // length of source UTF-16 string, in wchar_ts
        &utf8[0],           // pointer to destination buffer
        utf8Length,         // size of destination buffer, in chars
        nullptr, nullptr    // unused
    );
    if (result == 0)
    {
        // Conversion error: capture error code and throw
        const DWORD error = ::GetLastError();
        throw Utf8ConversionException(
            "Error in converting from UTF-16 to UTF-8.\n",
            error);
    }

    // Return the converted result string
    return utf8;
}


//------------------------------------------------------------------------------
// Convert form UTF-16 to UTF-8.
//
// UTF-16 strings are stored in CStringW.
// UTF-8 strings are stored using std::string.
// 
// On conversion errors (e.g. invalid UTF-16 sequence in input string), throws
// Utf8ConversionException.
//------------------------------------------------------------------------------
inline std::string Utf8FromUtf16(const CStringW& utf16)
{
    // First, handle the special case of empty input string
    if (utf16.IsEmpty())
    {
        // Empty input ==> Empty result
        return std::string();
    }

    // Delegate the conversion to the [start, finish) range overload
    const wchar_t * const utf16Start = utf16.GetString();
    const wchar_t * const utf16Finish = utf16Start + utf16.GetLength();
    return Utf8FromUtf16(utf16Start, utf16Finish);
}


} // namespace win32

} // namespace GiovanniDicanio

#endif // GIOVANNI_DICANIO_INCLUDE_UTF8CONV_H
