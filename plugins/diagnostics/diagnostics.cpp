
#include <stackwell/plugin/plugin_api.h>

#include <cstdint>

namespace
{
    const StackwellHostApi* g_host = nullptr;

    int STACKWELL_PLUGIN_CALL initialize(
        const StackwellHostApi* host
    )
    {
        if (host == nullptr)
        {
            return 1;
        }

        if (host->api_version != STACKWELL_PLUGIN_API_VERSION)
        {
            return 2;
        }

        if (host->struct_size < sizeof(StackwellHostApi))
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

    void STACKWELL_PLUGIN_CALL shutdown()
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

    const StackwellPluginDescriptor descriptor{
        static_cast<std::uint32_t>(
            sizeof(StackwellPluginDescriptor)
        ),

        STACKWELL_PLUGIN_API_VERSION,

        "diagnostics",
        "0.3.0",

        initialize,
        shutdown,

        "Stackwell Team",
        "System diagnostics plugin for Stackwell",
        "0.0.1"
    };
}

extern "C"
STACKWELL_PLUGIN_EXPORT
const StackwellPluginDescriptor*
STACKWELL_PLUGIN_CALL stackwell_plugin_query()
{
    return &descriptor;
}
