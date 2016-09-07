#include "Lobby.h"
#include "../Callbacks.h"

class LobbyCallbacks : public UtilsCallbacks
{
public:
	s::StackArray<CSteamID> m_arrLobbyList;

	CCallResult<LobbyCallbacks, LobbyMatchList_t> m_CallResultLobbyMatchList;
	void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool IOFailure)
	{
		Done();

		if (IOFailure) {
			printf("IO Failure.\n");
			return;
		}

		int num = pCallback->m_nLobbiesMatching;
		if (num == 0) {
			printf("No lobbies.\n");
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
	}
};

static LobbyCallbacks g_callbacks;

static void Help()
{
	printf("Available commands for lobby:\n");
	printf("  list [limit]\n");
}

static void List(Params &params)
{
	if (params.Count() == 3) {
		int limit = atoi(params[2]);
		printf("Requesting lobby list with limit %d...\n", limit);
		SteamMatchmaking()->AddRequestLobbyListResultCountFilter(limit);
	} else {
		printf("Requesting lobby list...\n");
	}

	g_callbacks.Prepare();
	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->RequestLobbyList();
	g_callbacks.m_CallResultLobbyMatchList.Set(hSteamAPICall, &g_callbacks, &LobbyCallbacks::OnLobbyMatchList);
	g_callbacks.Wait();
}

static void Data(Params &params)
{
	if (params.Count() == 2) {
		printf("Usage:\n");
		printf("  lobby data <index> [key]\n");
		return;
	}

	int index = atoi(params[2]);
	int numLobbies = g_callbacks.m_arrLobbyList.Count();
	if (index < 0 || index >= numLobbies) {
		printf("Index %d is out of range. We have %d listed lobb%s.\n", index, numLobbies, numLobbies != 1 ? "ies" : "y");
		return;
	}

	CSteamID &id = g_callbacks.m_arrLobbyList[index];
	int numKeys = SteamMatchmaking()->GetLobbyDataCount(id);
	printf("Lobby %llu has %d metadata key%s.\n", id.ConvertToUint64(), numKeys, numKeys != 1 ? "s" : "");

	if (params.Count() >= 4) {
		const char* key = params[3];
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

void LobbyCommands(Params &params)
{
	if (params.Count() == 1) {
		Help();
	} else if (params[1] == "list") {
		List(params);
	} else if (params[1] == "data") {
		Data(params);
	}
}
