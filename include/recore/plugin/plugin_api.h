#pragma once

#include <stdint.h>

#define RECORE_PLUGIN_API_VERSION 1

#if defined(_WIN32)
    #define RECORE_PLUGIN_CALL __cdecl

    #if defined(RECORE_PLUGIN_BUILD)
        #define RECORE_PLUGIN_EXPORT __declspec(dllexport)
    #else
        #define RECORE_PLUGIN_EXPORT
    #endif
#else
    #define RECORE_PLUGIN_CALL

    #if defined(RECORE_PLUGIN_BUILD)
        #define RECORE_PLUGIN_EXPORT \
            __attribute__((visibility("default")))
    #else
        #define RECORE_PLUGIN_EXPORT
    #endif
#endif

#ifdef __cplusplus
extern "C" {
#endif

typedef void (RECORE_PLUGIN_CALL *RecoreLogFn)(
    const char* message
);

typedef struct RecoreHostApi
{
    uint32_t api_version;
    uint32_t struct_size;

    RecoreLogFn log_info;
    RecoreLogFn log_error;

} RecoreHostApi;

typedef int (RECORE_PLUGIN_CALL *RecorePluginInitializeFn)(
    const RecoreHostApi* host
);

typedef void (RECORE_PLUGIN_CALL *RecorePluginShutdownFn)(void);

typedef struct RecorePluginDescriptor
{
    /* Required fields: API v1 */
    uint32_t struct_size;
    uint32_t api_version;

    const char* name;
    const char* version;

    RecorePluginInitializeFn initialize;
    RecorePluginShutdownFn shutdown;

    /*
     * Optional metadata.
     *
     * These fields were appended to preserve compatibility
     * with older API v1 descriptors.
     *
     * The host must check struct_size before accessing them.
     */
    const char* author;
    const char* description;
    const char* min_host_version;

} RecorePluginDescriptor;

typedef const RecorePluginDescriptor*
    (RECORE_PLUGIN_CALL *RecorePluginQueryFn)(void);

#if defined(RECORE_PLUGIN_BUILD)

RECORE_PLUGIN_EXPORT
const RecorePluginDescriptor*
RECORE_PLUGIN_CALL recore_plugin_query(void);

#endif

#ifdef __cplusplus
}
#endif