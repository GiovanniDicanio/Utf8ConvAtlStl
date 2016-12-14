# Utf8ConvAtlStl
### C++ helper functions to convert between Unicode UTF-8 (`std::string`) and UTF-16 (`CStringW`)

Giovanni Dicanio

This C++ project contains a _header-only_ module for UTF-8 (`std::string`) <-> UTF-16 (`CStringW`) conversions.

It makes sense to use `CStringW` at the Windows ATL/MFC level for UTF-16 text,
and `std::string` e.g. in cross-platform C++ code storing UTF-8 text.

This reusable header-only module ([`Utf8Conv.h`](https://github.com/GiovanniDicanio/Utf8ConvAtlStl/blob/master/Utf8ConvAtlStl/Utf8ConvAtlStl/Utf8Conv.h)) 
provides a couple of convenient conversion functions between
these two types of strings.
 
Conversion errors are signaled using `AtlThrow()`, which [by default](https://msdn.microsoft.com/en-us/library/z325eyx0.aspx) translates
to throwing `CAtlException` instances.
 
For more details, please read [my MSDN Magazine article](https://msdn.microsoft.com/magazine/mt763237):
 
>  C++ - Unicode Encoding Conversions with STL Strings and Win32 APIs

The code presented here is based on that article, but uses `CStringW` instead
of `std::wstring` for UTF-16 strings at the Windows platform level.

This project contains also a unit-test for the reusable Unicode conversion module.

Code developed using Visual Studio 2015.  
Compiles cleanly at `/W4` in both 32-bit builds and 64-bit builds.

**Note for Older VC++ Compilers**  
If you are using Visual Studio 2010, which doesn't support the C++11 `constexpr` keyword, you can still include this C++ code in your projects, simply substituting every instance of `constexpr` with `const`; this code will work just fine.
