#include "../settings/settings.h"
#include "../helpers/utils.h"

namespace features
{
	void Deranker() //Credits: Deerpfy
	{
		if (!settings::misc::deranker)
			return;

		static long long last_disconnect_time = utils::GetEpochTime();
		static long long last_connected_time = utils::GetEpochTime();

        if (g::engine_client->IsInGame() && g::engine_client->IsConnected())
        {
            if (utils::GetEpochTime() - last_connected_time > 7500) //7,5 seconds
            {
                g::engine_client->ClientCmd("disconnect");
                last_disconnect_time = utils::GetEpochTime();
            }
        }

        if (!g::engine_client->IsInGame())
        {
            if (utils::GetEpochTime() - last_disconnect_time > 15000) //15 seconds
            {
                g::engine_client->ClientCmd("retry");
                last_connected_time = utils::GetEpochTime();
            }
        }
	}
}