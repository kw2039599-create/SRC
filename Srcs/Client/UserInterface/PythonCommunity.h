#pragma once

#if defined(ENABLE_MESSENGER_RENEWAL)

class CPythonCommunity : public CSingleton<CPythonCommunity>
{
public:
	CPythonCommunity();
	virtual ~CPythonCommunity();

	void Destroy();

	void SetCommunityHandler(PyObject* poHandler);
	void SetMessengerHandler(PyObject* poHandler);
	void SetGuildHandler(PyObject* poHandler);
	void SetConfigHandler(PyObject* poHandler);

	void ClearCommunityHandler();
	void ClearMessengerHandler();
	void ClearGuildHandler();
	void ClearConfigHandler();

	PyObject* GetCommunityHandler() const { return m_poCommunityHandler; }
	PyObject* GetMessengerHandler() const { return m_poMessengerHandler; }
	bool HasMessengerHandler() const { return NULL != m_poMessengerHandler; }

	BYTE GetMainConnectionState() const { return m_bMainConnectionState; }
	void SetMainConnectionState(BYTE bState) { m_bMainConnectionState = bState; }

	DWORD GetConfigFlag() const { return m_dwConfigFlag; }
	void SetConfigFlag(DWORD dwFlag) { m_dwConfigFlag = dwFlag; }

	void OnFriendOnline(const char* c_szName);
	void OnFriendOffline(const char* c_szName);
	void OnFriendRemove(const char* c_szName);

	void OnFriendRenewalDetails(const char* c_szName, BYTE bLevel, BYTE bIsConqueror, BYTE bChannel, long lMapIndex);
	void OnFriendConnectionState(const char* c_szName, BYTE bConnectionState);
	void OnFriendStatusMessage(const char* c_szName, const char* c_szMessage);
	void NotifyMyStatusMessage(const char* c_szMessage);

#if defined(ENABLE_COMMUNITY_GUILD_RENEWAL)
	void OnGuildMemberLocation(DWORD dwPID, BYTE bChannel, long lMapIndex);
#endif

#if defined(ENABLE_MESSENGER_BLOCK)
	void OnBlockOnline(const char* c_szName);
	void OnBlockOffline(const char* c_szName);
	void OnBlockRemove(const char* c_szName);
#endif

private:
	PyObject* m_poCommunityHandler;
	PyObject* m_poMessengerHandler;
	PyObject* m_poGuildHandler;
	PyObject* m_poConfigHandler;

	DWORD m_dwConfigFlag;
	BYTE m_bMainConnectionState;
};

#endif