# Utf8ConvAtlStl
### C++ helper functions to convert between Unicode UTF-8 and UTF-16

Giovanni Dicanio

This C++ project contains a **reusable** _header-only_ module implementing conversion code between **UTF-8 (`std::string`)** and **UTF-16 (`CStringW`)**.

It makes sense to use **ATL** `CStringW` in portions of C++ code at the _Windows ATL/MFC_ level for **UTF-16** text;
on the other hand, **STL** `std::string` makes sense e.g. in _cross-platform_ C++ code, storing **UTF-8** text.

This **reusable header-only module** ([`Utf8Conv.h`](https://github.com/GiovanniDicanio/Utf8ConvAtlStl/blob/master/Utf8ConvAtlStl/Utf8ConvAtlStl/Utf8Conv.h)) 
provides convenient conversion functions between these two Unicode encodings.
 
Conversion errors are signaled throwing instances of a custom exception class.
 
For more details, please read [my MSDN Magazine article](https://msdn.microsoft.com/magazine/mt763237):
 
>  C++ - Unicode Encoding Conversions with STL Strings and Win32 APIs

The code presented here is based on that article, but uses `CStringW` instead
of `std::wstring` for UTF-16 strings at the Windows platform level.

In addition, it's also possible to pass input source strings using an STL-style `[start, finish)` _range_; this is useful for converting portions, or _views_, of source strings.

This project contains also a **unit-test** ([`Utf8ConvTest.cpp`](https://github.com/GiovanniDicanio/Utf8ConvAtlStl/blob/master/Utf8ConvAtlStl/Utf8ConvAtlStl/Utf8ConvTest.cpp)) for the reusable Unicode conversion module.

Code developed using **Visual Studio 2015**.  
Compiles cleanly at `/W4` in both 32-bit builds and 64-bit builds.

**Note for Older VC++ Compilers**  
If you are using Visual Studio 2010, which doesn't support the C++11 `constexpr` keyword, you can still include this C++ code in your projects, simply substituting every instance of `constexpr` with `const`; this code will work just fine.
