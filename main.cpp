#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <readline/readline.h>
#include <readline/history.h>

#include <steam_api.h>

#define SCRATCH_IMPL
#define SCRATCH_NAMESPACE s
#include <Scratch.hpp>

class Utils_SteamCallbacks
{
public:
	bool m_waiting;

	void Prepare()
	{
		m_waiting = true;
	}

	void Wait()
	{
		while (m_waiting) {
			usleep(1000);
			SteamAPI_RunCallbacks();
		}
	}

	void Done()
	{
		m_waiting = false;
	}

public:
	s::StackArray<CSteamID> m_arrLobbyList;

	CCallResult<Utils_SteamCallbacks, LobbyMatchList_t> m_CallResultLobbyMatchList;
	void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool IOFailure)
	{
		if (IOFailure) {
			printf("IO Failure!");
			Done();
			return;
		}

		int num = pCallback->m_nLobbiesMatching;
		if (num == 0) {
			printf("No lobbies.\n");
			Done();
			return;
		}

		m_arrLobbyList.Clear();

		printf("%d lobb%s:\n", num, num != 1 ? "ies" : "y");
		for (int i = 0; i < num; i++) {
			CSteamID id = SteamMatchmaking()->GetLobbyByIndex(i);
			int numMembers = SteamMatchmaking()->GetNumLobbyMembers(id);
			printf("  " TERMCOL_BOLDGREEN "%d" TERMCOL_RESET ": " TERMCOL_BOLDWHITE "%llu" TERMCOL_RESET " (%d member%s)\n", i, id.ConvertToUint64(), numMembers, numMembers != 1 ? "s" : "");

			m_arrLobbyList.Push() = id;
		}

		Done();
	}
};

int main(int argc, char* args[]) {
	if (argc != 2) {
		printf("Usage: steamutils <appid>\n");
		return 1;
	}

	setenv("SteamAppId", args[1], 1);
	if (!SteamAPI_Init()) {
		printf("Failed to initialize Steam API.\n");
		return 1;
	}

	Utils_SteamCallbacks callbacks;

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
			if (parse.Count() == 1) {
				printf("Available commands for lobby:\n");
				printf("  list [limit]\n");

			} else if (parse[1] == "list") {
				if (parse.Count() == 3) {
					int limit = atoi(parse[2]);
					printf("Requesting lobby list with limit %d...\n", limit);
					SteamMatchmaking()->AddRequestLobbyListResultCountFilter(limit);
				} else {
					printf("Requesting lobby list...\n");
				}

				callbacks.Prepare();
				SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
				callbacks.m_CallResultLobbyMatchList.Set(hSteamAPICall, &callbacks, &Utils_SteamCallbacks::OnLobbyMatchList);
				callbacks.Wait();

			} else if (parse[1] == "data") {
				if (parse.Count() == 2) {
					printf("Usage:\n");
					printf("  lobby data <index> [key]\n");
					continue;
				}

				int index = atoi(parse[2]);
				int numLobbies = callbacks.m_arrLobbyList.Count();
				if (index < 0 || index >= numLobbies) {
					printf("Index %d is out of range. We have %d listed lobb%s.\n", index, numLobbies, numLobbies != 1 ? "ies" : "y");
					continue;
				}

				CSteamID &id = callbacks.m_arrLobbyList[index];
				int numKeys = SteamMatchmaking()->GetLobbyDataCount(id);
				printf("Lobby %llu has %d metadata key%s.\n", id.ConvertToUint64(), numKeys, numKeys != 1 ? "s" : "");

				if (parse.Count() >= 4) {
					const char* key = parse[3];
					const char* value = SteamMatchmaking()->GetLobbyData(id, key);
					printf("  " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": '" TERMCOL_BOLDWHITE "%s" TERMCOL_RESET "'\n", key, value);
				} else {
					for (int i = 0; i < numKeys; i++) {
						char key[128];
						char value[128];
						SteamMatchmaking()->GetLobbyDataByIndex(id, i, key, 128, value, 128);
						printf("  " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": '" TERMCOL_BOLDWHITE "%s" TERMCOL_RESET "'\n", key, value);
					}
				}
			}
		}
	}
	printf("\n");
	return 0;
}
