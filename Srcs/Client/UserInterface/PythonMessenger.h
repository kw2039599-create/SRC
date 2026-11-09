#pragma once

class CPythonMessenger : public CSingleton<CPythonMessenger>
{
	public:
		typedef std::set<std::string> TFriendNameMap;
		#ifdef ENABLE_MESSENGER_BLOCK
		typedef std::set<std::string> TBlockNameMap;
		#endif
		typedef std::map<std::string, BYTE> TGuildMemberStateMap;

		enum EMessengerGroupIndex
		{
			MESSENGER_GRUOP_INDEX_FRIEND,
			MESSENGER_GRUOP_INDEX_GUILD,
			#ifdef ENABLE_MESSENGER_BLOCK
			MESSENGER_GROUP_INDEX_BLOCK,
			#endif
		};

	public:
		CPythonMessenger();
		virtual ~CPythonMessenger();

		void Destroy();

		// Friend
		void RemoveFriend(const char * c_szKey);
		void OnFriendLogin(const char * c_szKey);
		void OnFriendLogout(const char * c_szKey);
		#ifdef ENABLE_MESSENGER_BLOCK
		void RemoveBlock(const char * c_szKey);
		void OnBlockLogin(const char * c_szKey);
		void OnBlockLogout(const char * c_szKey);
		BOOL IsBlockByKey(const char * c_szKey);
		BOOL IsBlockByName(const char * c_szName);
		#endif
		BOOL IsFriendByKey(const char * c_szKey);
		BOOL IsFriendByName(const char * c_szName);

		// Guild
		void AppendGuildMember(const char * c_szName);
		void RemoveGuildMember(const char * c_szName);
		void RemoveAllGuildMember();
		void LoginGuildMember(const char * c_szName);
		void LogoutGuildMember(const char * c_szName);
		void RefreshGuildMember();

		void SetMessengerHandler(PyObject* poHandler);
#ifdef ENABLE_MESSENGER_BLOCK
		const TBlockNameMap& GetBlockNameMap() const { return m_BlockNameMap; }
#endif
#if defined(__BL_MAILBOX__)
		TFriendNameMap m_FriendNameMap;
#endif

	protected:
#if !defined(__BL_MAILBOX__)
		TFriendNameMap m_FriendNameMap;
#endif
		#ifdef ENABLE_MESSENGER_BLOCK
		TBlockNameMap m_BlockNameMap;
		#endif
		TGuildMemberStateMap m_GuildMemberStateMap;

	private:
		PyObject * m_poMessengerHandler;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
