#include <readline/readline.h>
#include <readline/history.h>

#include "Common.h"
#include "Apis/Lobby.h"

int main(int argc, char* args[])
{
	if (argc != 2) {
		printf("Usage: steamutils <appid>\n");
		return 1;
	}

	setenv("SteamAppId", args[1], 1);
	if (!SteamAPI_Init()) {
		printf("Failed to initialize Steam API.\n");
		return 1;
	}

	char* line = nullptr;
	while (true) {
		printf("\n");
		SteamAPI_RunCallbacks();

		if (line != nullptr) {
			free(line);
		}
		line = readline(TERMCOL_BOLDRED "> " TERMCOL_RESET);
		if (line == nullptr) {
			break;
		}
		if (*line) {
			add_history(line);
		}

		if (strlen(line) == 0) {
			continue;
		}

		s::StackArray<s::String> parse;
		s::String(line).CommandLineSplit(parse);

		if (parse[0] == "help") {
			printf("Available commands:\n");
			printf("  lobby\n");

		} else if (parse[0] == "lobby") {
			LobbyCommands(parse);
		}
	}

	printf("\n");
	return 0;
}
