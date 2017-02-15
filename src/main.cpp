#include <readline/readline.h>
#include <readline/history.h>

#include "Common.h"
#include "Apis/Lobby.h"
#include "Apis/Stats.h"
#include "Apis/Achievements.h"

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

		s::String prompt;
		LobbyPrompt(prompt);
		prompt += TERMCOL_BOLDRED "> " TERMCOL_RESET;
		line = readline(prompt);

		if (line == nullptr) {
			break;
		}
		if (*line) {
			add_history(line);
		}

		if (strlen(line) == 0) {
			continue;
		}

		Params parse;
		s::String(line).CommandLineSplit(parse);

		if (parse[0] == "help") {
			printf("Available commands:\n");
			printf("  lobby\n");
			printf("  stats\n");
			printf("  achievements\n");

		} else if (parse[0] == "lobby") {
			LobbyCommands(parse);

		} else if (parse[0] == "stats") {
			StatsCommands(parse);

		} else if (parse[0] == "achievements") {
			AchievementsCommands(parse);

		}
	}

	printf("\n");
	return 0;
}
