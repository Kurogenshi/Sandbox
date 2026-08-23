#pragma once

#define SANDBOX_CONCAT_IMPL(a, b) a##b
#define SANDBOX_CONCAT(a, b) SANDBOX_CONCAT_IMPL(a, b)

#if defined(__COUNTER__)
    #define SANDBOX_UNIQUE_NAME(prefix) SANDBOX_CONCAT(prefix, __COUNTER__)
#else
    #define SANDBOX_UNIQUE_NAME(prefix) SANDBOX_CONCAT(prefix, __LINE__)
#endif