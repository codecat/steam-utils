#include "Stats.h"

static void Help()
{
	printf("Available commands for stats:\n");
	printf("  get <int|float> <key>\n");
	printf("  set <int|float> <key> <value>\n");
}

static void Get(Params &params)
{
	if (params.Count() != 4) {
		printf("Usage:\n");
		printf("  stats get <int|float> <key>\n");
		return;
	}

	s::String type = params[2];
	s::String key = params[3];

	if (type == "int") {
		int32 data = 0;
		if (!SteamUserStats()->GetStat(key, &data)) {
			printf("Couldn't get stat value.\n");
			return;
		}

		printf("%s " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": " TERMCOL_BOLDYELLOW "%d" TERMCOL_RESET " (%08X)\n", (const char*)type, (const char*)key, data, data);

	} else if (type == "float") {
		float data = 0;
		if (!SteamUserStats()->GetStat(key, &data)) {
			printf("Couldn't get stat value.\n");
			return;
		}

		printf("%s " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": " TERMCOL_BOLDYELLOW "%f" TERMCOL_RESET " (%08X)\n", (const char*)type, (const char*)key, data, (uint32)data);

	} else {
		printf("Unknown type: %s\n", (const char*)type);
		return;
	}
}

static void Set(Params &params)
{
	if (params.Count() != 5) {
		printf("Usage:\n");
		printf("  stats set <int|hexint|float> <key> <value>\n");
		return;
	}

	s::String type = params[2];
	s::String key = params[3];
	s::String value = params[4];

	if (type == "int") {
		int32 data = 0;
		if (sscanf(value, "%d", &data) != 1) {
			printf("Invalid integer input.\n");
			return;
		}

		if (!SteamUserStats()->SetStat(key, data)) {
			printf("Couldn't set stat value.\n");
			return;
		}

		if (!SteamUserStats()->StoreStats()) {
			printf("Couldn't store stats.\n");
			return;
		}

		printf(TERMCOL_BOLDGREEN "Stat set!" TERMCOL_RESET "\n");

	} else if (type == "hexint") {
		int32 data = 0;
		if (sscanf(value, "%x", &data) != 1 || sscanf(value, "%X", &data) != 1) {
			printf("Invalid hexadecimal integer input.\n");
			return;
		}

		if (!SteamUserStats()->SetStat(key, data)) {
			printf("Couldn't set stat value.\n");
			return;
		}

		if (!SteamUserStats()->StoreStats()) {
			printf("Couldn't store stats.\n");
			return;
		}

		printf(TERMCOL_BOLDGREEN "Stat set!" TERMCOL_RESET "\n");

	} else if (type == "float") {
		float data = 0;
		if (sscanf(value, "%f", &data) != 1) {
			printf("Invalid float input.\n");
			return;
		}

		if (!SteamUserStats()->SetStat(key, data)) {
			printf("Couldn't set stat value.\n");
			return;
		}

		if (!SteamUserStats()->StoreStats()) {
			printf("Couldn't store stats.\n");
			return;
		}

		printf(TERMCOL_BOLDGREEN "Stat set!" TERMCOL_RESET "\n");

	} else {
		printf("Unknown type: %s\n", (const char*)type);
		return;
	}
}

void StatsCommands(Params &params)
{
	if (params.Count() == 1) {
		Help();
	} else if (params[1] == "get") {
		Get(params);
	} else if (params[1] == "set") {
		Set(params);
	}
}
