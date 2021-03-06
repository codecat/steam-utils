#include "Lobby.h"
#include "../Callbacks.h"

class LobbyCallbacks : public UtilsCallbacks
{
public:
	s::StackArray<CSteamID> m_arrLobbyList;

	CCallResult<LobbyCallbacks, LobbyMatchList_t> m_CallResultLobbyMatchList;
	void OnLobbyMatchList(LobbyMatchList_t* pCallback, bool bIOFailure)
	{
		Done();

		if (bIOFailure) {
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

	bool m_inLobbyHost;
	CSteamID m_inLobby;

	CCallResult<LobbyCallbacks, LobbyCreated_t> m_CallResultLobbyCreated;
	void OnLobbyCreated(LobbyCreated_t* pCallback, bool bIOFailure)
	{
		Done();

		if (pCallback->m_eResult != k_EResultOK) {
			printf("Lobby creation failed. Error %d.\n", pCallback->m_eResult);
			return;
		}

		m_inLobbyHost = true;
		m_inLobby = pCallback->m_ulSteamIDLobby;
		printf("Lobby created: %llu\n", pCallback->m_ulSteamIDLobby);
	}

	CCallResult<LobbyCallbacks, LobbyEnter_t> m_CallResultLobbyEnter;
	void OnLobbyEnter(LobbyEnter_t* pCallback, bool bIOFailure)
	{
		Done();

		if (pCallback->m_EChatRoomEnterResponse > k_EChatRoomEnterResponseSuccess) {
			printf("Failed to join lobby. Room enter response %d.\n", pCallback->m_EChatRoomEnterResponse);
			return;
		}

		m_inLobbyHost = false;
		m_inLobby = pCallback->m_ulSteamIDLobby;
		printf("Lobby joined:\n");
		printf("  " TERMCOL_BOLDGREEN "Chat permissions" TERMCOL_RESET ": " TERMCOL_BOLDWHITE "%u" TERMCOL_RESET "\n", pCallback->m_rgfChatPermissions);
		printf("  " TERMCOL_BOLDGREEN "Locked" TERMCOL_RESET ": " TERMCOL_BOLDWHITE "%s" TERMCOL_RESET "\n", pCallback->m_bLocked ? "true" : "false");
	}
};

static LobbyCallbacks g_callbacks;

static void Help()
{
	printf("Available commands for lobby:\n");
	printf("  list [limit]\n");
	printf("  data <index> [key]\n");
	printf("\n");
	printf("  create [type] [maxplayers]\n");
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
	bool inLobby = g_callbacks.m_inLobby.IsValid();

	if (params.Count() == 2 && !inLobby) {
		printf("Usage:\n");
		printf("  lobby data <index>\n");
		return;
	}

	int index = inLobby ? -1 : atoi(params[2]);

	CSteamID id;
	if (index == -1 && inLobby) {
		id = g_callbacks.m_inLobby;
	} else {
		int numLobbies = g_callbacks.m_arrLobbyList.Count();
		if (index < 0 || index >= numLobbies) {
			printf("Index %d is out of range. We have %d listed lobb%s.\n", index, numLobbies, numLobbies != 1 ? "ies" : "y");
			return;
		}
		id = g_callbacks.m_arrLobbyList[index];
	}

	int numKeys = SteamMatchmaking()->GetLobbyDataCount(id);
	printf("Lobby %llu has %d metadata key%s:\n", id.ConvertToUint64(), numKeys, numKeys != 1 ? "s" : "");

	for (int i = 0; i < numKeys; i++) {
		char key[128];
		char value[128];
		SteamMatchmaking()->GetLobbyDataByIndex(id, i, key, 128, value, 128);
		printf("  " TERMCOL_BOLDGREEN "%s" TERMCOL_RESET ": '" TERMCOL_BOLDWHITE "%s" TERMCOL_RESET "'\n", key, value);
	}
}

static void Create(Params &params)
{
	ELobbyType lobbyType = k_ELobbyTypePublic;
	s::String strLobbyType = "public";

	int numMaxMembers = 8;

	if (params.Count() >= 3) {
		strLobbyType = params[2];
		if (strLobbyType == "private") {
			lobbyType = k_ELobbyTypePrivate;
		} else if (strLobbyType == "friendsonly") {
			lobbyType = k_ELobbyTypeFriendsOnly;
		} else if (strLobbyType == "public") {
			lobbyType = k_ELobbyTypePublic;
		} else if (strLobbyType == "invisible") {
			lobbyType = k_ELobbyTypeInvisible;
		} else {
			printf("Unknown lobby type %s. Try one of the following: private, friendsonly, public, invisible\n", (const char*)strLobbyType);
			return;
		}
	}

	if (params.Count() >= 4) {
		numMaxMembers = atoi(params[3]);
	}

	printf("Creating %s lobby with %d members max...\n", (const char*)strLobbyType, numMaxMembers);

	g_callbacks.Prepare();
	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->CreateLobby(lobbyType, numMaxMembers);
	g_callbacks.m_CallResultLobbyCreated.Set(hSteamAPICall, &g_callbacks, &LobbyCallbacks::OnLobbyCreated);
	g_callbacks.Wait();
}

static void Join(Params &params)
{
	//TODO: Joining private lobbies via ID

	if (params.Count() == 2) {
		printf("Usage:\n");
		printf("  lobby join <index>\n");
		return;
	}

	int index = atoi(params[2]);
	int numLobbies = g_callbacks.m_arrLobbyList.Count();
	if (index < 0 || index >= numLobbies) {
		printf("Index %d is out of range. We have %d listed lobb%s.\n", index, numLobbies, numLobbies != 1 ? "ies" : "y");
		return;
	}

	CSteamID &id = g_callbacks.m_arrLobbyList[index];

	printf("Joing lobby with ID %llu\n", id.ConvertToUint64());

	g_callbacks.Prepare();
	SteamAPICall_t hSteamAPICall = SteamMatchmaking()->JoinLobby(id);
	g_callbacks.m_CallResultLobbyEnter.Set(hSteamAPICall, &g_callbacks, &LobbyCallbacks::OnLobbyEnter);
	g_callbacks.Wait();
}

static void Leave()
{
	if (!g_callbacks.m_inLobby.IsValid()) {
		printf("Not currently in a lobby.\n");
		return;
	}

	SteamMatchmaking()->LeaveLobby(g_callbacks.m_inLobby);
	g_callbacks.m_inLobby.Clear();

	printf("Lobby left.\n");
}

void LobbyPrompt(s::String &prompt)
{
	if (!g_callbacks.m_inLobby.IsValid()) {
		return;
	}

	if (g_callbacks.m_inLobbyHost) {
		prompt += TERMCOL_BOLDYELLOW;
	} else {
		prompt += TERMCOL_BOLDGREEN;
	}
	prompt += s::strPrintF("(lobby:%llu) " TERMCOL_RESET, g_callbacks.m_inLobby.ConvertToUint64());
}

void LobbyCommands(Params &params)
{
	if (params.Count() == 1) {
		Help();
	} else if (params[1] == "list") {
		List(params);
	} else if (params[1] == "data") {
		Data(params);
	} else if (params[1] == "create") {
		Create(params);
	} else if (params[1] == "join") {
		Join(params);
	} else if (params[1] == "leave") {
		Leave();
	}
}
