// CRT compatibility shim for linking VS2012-era libraries with VS2022
// Provides symbols removed in the Universal CRT (VS2015+)

#include <cstdio>
#include <cstring>

// __iob_func was removed in VS2015. Old code (e.g. libpng) references it.
// Provide a shim that returns the stdio file pointers.
extern "C" FILE* __iob_func(void)
{
    // The old __iob_func returned an array of {stdin, stdout, stderr}.
    // In the Universal CRT, these are functions, not a contiguous array.
    // We return a static array mimicking the old layout.
    static FILE iob[3];
    iob[0] = *stdin;
    iob[1] = *stdout;
    iob[2] = *stderr;
    return iob;
}

// std::_Winerror_map was an internal MSVC runtime function used by
// std::system_category::message(). Old .lib files compiled with VS2012
// may reference it. Provide a minimal stub.
namespace std {
    const char* _Winerror_map(int) {
        return "";
    }
}
