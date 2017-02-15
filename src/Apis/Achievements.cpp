#include "Achievements.h"

static void Help()
{
	printf("Available commands for achievements:\n");
	printf("  list\n");
	printf("  set <id>\n");
	printf("  clear <id>\n");
}

static void List(Params &params)
{
	uint32 num = SteamUserStats()->GetNumAchievements();
	printf("%u achievement%s: (? = hidden, ! = achieved)\n", num, num != 1 ? "s" : "");
	for (uint32 i = 0; i < num; i++) {
		const char* id = SteamUserStats()->GetAchievementName(i);
		const char* name = SteamUserStats()->GetAchievementDisplayAttribute(id, "name");
		const char* desc = SteamUserStats()->GetAchievementDisplayAttribute(id, "desc");
		bool hidden = (strcmp(SteamUserStats()->GetAchievementDisplayAttribute(id, "hidden"), "1") == 0);
		bool achieved = false;
		SteamUserStats()->GetAchievement(id, &achieved);

		printf("  ");
		printf("%s", hidden ? "?" : " ");
		printf("%s", achieved ? "!" : " ");
		printf(" " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": " TERMCOL_BOLDYELLOW "%s" TERMCOL_RESET " (%s)\n", id, name, desc);
	}
}

static void Set(Params &params)
{
	if (params.Count() != 3) {
		printf("Usage:\n");
		printf("  achievements set <id>\n");
		return;
	}

	s::String id = params[2];

	if (!SteamUserStats()->SetAchievement(id)) {
		printf("Couldn't set achievement.\n");
		return;
	}

	if (!SteamUserStats()->StoreStats()) {
		printf("Couldn't store stats.\n");
		return;
	}

	printf(TERMCOL_BOLDGREEN "Achievement set!" TERMCOL_RESET "\n");
}

static void Clear(Params &params)
{
	if (params.Count() != 3) {
		printf("Usage:\n");
		printf("  achievements clear <id>\n");
		return;
	}

	s::String id = params[2];

	if (!SteamUserStats()->ClearAchievement(id)) {
		printf("Couldn't clear achievement.\n");
		return;
	}

	if (!SteamUserStats()->StoreStats()) {
		printf("Couldn't store stats.\n");
		return;
	}

	printf(TERMCOL_BOLDGREEN "Achievement cleared!" TERMCOL_RESET "\n");
}

void AchievementsCommands(Params &params)
{
	if (params.Count() == 1) {
		Help();
	} else if (params[1] == "list") {
		List(params);
	} else if (params[1] == "set") {
		Set(params);
	} else if (params[1] == "clear") {
		Clear(params);
	}
}
