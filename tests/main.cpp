#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest/doctest.h"
#include <string>
#include <vector>
#if defined(__WIN32__) || defined(_MSC_VER)
#include <winsock2.h>
WSADATA wsaData;
#endif

int main(int argc, char **argv)
{
#if defined(__WIN32__) || defined(_MSC_VER)
    int iResult;

    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
#endif

    std::vector<std::string> adjusted_args;
    adjusted_args.reserve(argc);
    for (int i = 0; i < argc; ++i)
    {
        std::string current(argv[i]);
        if ((current == "--test-case" || current == "-tc") && i + 1 < argc)
        {
            std::string pattern(argv[i + 1]);
            if (pattern.find('*') == std::string::npos)
                pattern = "*" + pattern + "*";
            adjusted_args.push_back(current + "=" + pattern);
            ++i;
        }
        else
        {
            adjusted_args.push_back(std::move(current));
        }
    }

    std::vector<char *> argv_adjusted;
    argv_adjusted.reserve(adjusted_args.size());
    for (auto &arg : adjusted_args)
    {
        argv_adjusted.push_back(arg.data());
    }

    doctest::Context context;
    context.applyCommandLine(static_cast<int>(argv_adjusted.size()),
                             argv_adjusted.data());
    int res = context.run();  // run
    if (context.shouldExit()) // important - query flags (and --exit) rely on
                              // the user doing this
        return res;           // propagate the result of the tests
    int client_stuff_return_code = 0;
    return res + client_stuff_return_code; // the result from doctest is
                                           // propagated here as well
}
