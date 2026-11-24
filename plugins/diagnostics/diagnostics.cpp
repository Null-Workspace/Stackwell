
#include <recore/plugin/plugin_api.h>

#include <cstdint>

namespace
{
    const RecoreHostApi* g_host = nullptr;

    int RECORE_PLUGIN_CALL initialize(
        const RecoreHostApi* host
    )
    {
        if (host == nullptr)
        {
            return 1;
        }

        if (host->api_version != RECORE_PLUGIN_API_VERSION)
        {
            return 2;
        }

        if (host->struct_size < sizeof(RecoreHostApi))
        {
            return 3;
        }

        if (host->log_info == nullptr ||
            host->log_error == nullptr)
        {
            return 4;
        }

        g_host = host;

        g_host->log_info(
            "Diagnostics module initialized"
        );

        g_host->log_info(
            "Diagnostics API compatibility verified"
        );

        return 0;
    }

    void RECORE_PLUGIN_CALL shutdown()
    {
        if (g_host != nullptr &&
            g_host->log_info != nullptr)
        {
            g_host->log_info(
                "Diagnostics module stopped"
            );
        }

        g_host = nullptr;
    }

    const RecorePluginDescriptor descriptor{
        static_cast<std::uint32_t>(
            sizeof(RecorePluginDescriptor)
        ),

        RECORE_PLUGIN_API_VERSION,

        "diagnostics",
        "0.3.0",

        initialize,
        shutdown,

        "ReCore Team",
        "System diagnostics plugin for ReCore",
        "0.0.01"
    };
}

extern "C"
RECORE_PLUGIN_EXPORT
const RecorePluginDescriptor*
RECORE_PLUGIN_CALL recore_plugin_query()
{
    return &descriptor;
}
