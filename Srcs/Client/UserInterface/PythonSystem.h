#pragma once

class CItemData;

class CPythonSystem : public CSingleton<CPythonSystem>
{
	public:
		enum EWindow
		{
			WINDOW_STATUS,
			WINDOW_INVENTORY,
			WINDOW_ABILITY,
			WINDOW_SOCIETY,
			WINDOW_JOURNAL,
			WINDOW_COMMAND,

			WINDOW_QUICK,
			WINDOW_GAUGE,
			WINDOW_MINIMAP,
			WINDOW_CHAT,

			WINDOW_MAX_NUM,
		};

		enum
		{
			FREQUENCY_MAX_NUM  = 30,
			RESOLUTION_MAX_NUM = 100
		};

		typedef struct SResolution
		{
			DWORD	width;
			DWORD	height;
			DWORD	bpp;		// bits per pixel (high-color = 16bpp, true-color = 32bpp)

			DWORD	frequency[20];
			BYTE	frequency_count;
		} TResolution;

		typedef struct SWindowStatus
		{
			int		isVisible;
			int		isMinimized;

			int		ixPosition;
			int		iyPosition;
			int		iHeight;
		} TWindowStatus;

		typedef struct SConfig
		{
			DWORD			width;
			DWORD			height;
			DWORD			bpp;
			DWORD			frequency;

			bool			is_software_cursor;
			bool			is_object_culling;
			int				iDistance;

			int				iShadowLevel;
#ifdef ENABLE_FOV_OPTION
            FLOAT            iFOVLevel;
#endif
			int				iShadowQualityLevel;
			int				iTerrainShadow;
			int				iTreeShadow;
			int				iRainLevel;
			int				iSpecularLevel;
			int				iBloodLevel;
			int				iGraphicsLevel;
			int				iGrassType;
			float			fSharpnessPower;

			FLOAT			music_volume;
			BYTE			voice_volume;

			int				gamma;
#ifdef ENABLE_OFFLINESHOP_SYSTEM
			float			shopnames_range;
#endif

			int				isSaveID;
			char			SaveID[20];

			bool			bWindowed;
			bool			bDecompressDDS;
			bool			bNoSoundCard;
			bool			bUseDefaultIME;
			BYTE			bSoftwareTiling;
			bool			bViewChat;
			bool			bAlwaysShowName;
			bool			bShowDamage;
			bool			bShowSalesText;
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
			float			shopRangeView = 5000.f;
#endif
#endif
#if defined(__BL_FOG_FIX__)
			bool			bIsFogModeEnabled;
#endif
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
			bool			bNightMode;
			bool			bSnowMode;
			bool			bSnowTextureMode;
			bool			bRainMode;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
			bool			bShowMobAIFlag;
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
			bool			bShowMobLevel;
#endif
		} TConfig;
#if defined(__BL_PICK_FILTER__)
		class CPickUpFilter final
		{
		public:
			CPickUpFilter();
			~CPickUpFilter();

			void	SetFilter(size_t sIndex, bool b);
			void	SetSize(size_t sIndex, bool b);
			void	SetRefine(BYTE min, BYTE max);
			void	SetLevel(long min, long max);
			void	SetModeAll(bool b);
			void	Save();

			void ClearExceptions();
			void AddWhitelist(const char* szName);
			void AddBlacklist(const char* szName);

			bool	CanPickItem(DWORD dwIID);

			bool	GetFilter(size_t sIndex) const;
			bool	GetSize(size_t sIndex) const;
			bool	IsModeAll() const;

			std::pair<BYTE, BYTE> GetRefine();
			std::pair<long, long> GetLevel();

		private:
			bool	CheckRefine(const CItemData* pItem) const;
			bool	CheckLevel(const CItemData* pItem) const;
			bool	CheckSize(const CItemData* pItem) const;
			bool	CheckType(const CItemData* pItem) const;


			std::set<std::string> m_setWhitelist;
			std::set<std::string> m_setBlacklist;

			static constexpr const char* cPickUpFilterFileName = "PickUp.dat";

		public:
		enum EPICKFILTER
			{
				/*WEAPON-SUB*/
				SUB_WEAPON_SWORD,
				SUB_WEAPON_DAGGER,
				SUB_WEAPON_BOW,
				SUB_WEAPON_TWO_HANDED,
				SUB_WEAPON_BELL,
				SUB_WEAPON_FAN,
				SUB_WEAPON_ARROW,
				SUB_WEAPON_CLAW,
				/*WEAPON-SUB*/

				/*ARMOR-SUB*/
				SUB_ARMOR_BODY,
				SUB_ARMOR_HEAD,
				SUB_ARMOR_SHIELD,
				SUB_ARMOR_WRIST,
				SUB_ARMOR_FOOTS,
				SUB_ARMOR_NECK,
				SUB_ARMOR_EAR,
				SUB_ARMOR_PENDANT,
				SUB_ARMOR_GLOVE,
				/*ARMOR-SUB*/

				/*OTHER & POTIONS*/
				TYPE_METIN,
				TYPE_YANG,
				TYPE_SKILLBOOK,
				TYPE_GIFTBOX,
				TYPE_BELT,
				TYPE_POLY,
				TYPE_RING,
				TYPE_MATERIAL,

				SUB_POTION_REGULAR,
				SUB_POTION_HAIRDYE,
				SUB_POTION_ABILITY,
				/*OTHER & POTIONS*/

				/* EVENTS */
				SUB_EVENT_BOXES,
				SUB_EVENT_ITEMS,
				/* EVENTS */

				EPICKFILTER_MAX
			};

			enum ESIZE
			{
				SMALL,
				MID,
				BIG,

				ESIZE_MAX
			};
			
		private:
			bool bPickFilter[EPICKFILTER::EPICKFILTER_MAX];
			bool bPickSize[ESIZE::ESIZE_MAX];

			bool bModeAll;
			
			BYTE m_bRefineMin;
			BYTE m_bRefineMax;
			
			long m_lLevelMin;
			long m_lLevelMax;
		} TPickUpFilter;
#endif

	public:
		CPythonSystem();
		virtual ~CPythonSystem();

		void Clear();
		void SetInterfaceHandler(PyObject * poHandler);
		void DestroyInterfaceHandler();

		// Config
		void							SetDefaultConfig();
		bool							LoadConfig();
		bool							SaveConfig();
		void							ApplyConfig();
		void							SetConfig(TConfig * set_config);
		TConfig *						GetConfig();
		void							ChangeSystem();

		// Interface
		bool							LoadInterfaceStatus();
		void							SaveInterfaceStatus();
		bool							isInterfaceConfig();
		const TWindowStatus &			GetWindowStatusReference(int iIndex);

		DWORD							GetWidth();
		DWORD							GetHeight();
		DWORD							GetBPP();
		DWORD							GetFrequency();
		bool							IsSoftwareCursor();
		bool							IsWindowed();
		bool							IsViewChat();
		bool							IsAlwaysShowName();
		bool							IsShowDamage();
		bool							IsShowSalesText();
		bool							IsUseDefaultIME();
		bool							IsNoSoundCard();
		bool							IsAutoTiling();
#ifdef ENABLE_OFFLINESHOP_SYSTEM
		void							SetShopNamesRange(float fRange) { m_Config.shopnames_range = fRange; }
		float							GetShopNamesRange() { return m_Config.shopnames_range; }
#endif
		bool							IsSoftwareTiling();
		void							SetSoftwareTiling(bool isEnable);
		void							SetViewChatFlag(int iFlag);
		void							SetAlwaysShowNameFlag(int iFlag);
		void							SetShowDamageFlag(int iFlag);
		void							SetShowSalesTextFlag(int iFlag);
#ifdef ENABLE_IKASHOP_RENEWAL
#ifdef EXTEND_IKASHOP_PRO
		void							SetShopRangeView(float distance) { m_Config.shopRangeView = MINMAX(600.0f, distance, 5000.f); }
		float							GetShopRangeView() const {return m_Config.shopRangeView;}
#endif
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBAIFLAG)
		bool							IsShowMobAIFlag();
		void							SetShowMobAIFlagFlag(int iFlag);
#endif
#if defined(WJ_SHOW_MOB_INFO) && defined(ENABLE_SHOW_MOBLEVEL)
		bool							IsShowMobLevel();
		void							SetShowMobLevelFlag(int iFlag);
#endif

		// Window
		void							SaveWindowStatus(int iIndex, int iVisible, int iMinimized, int ix, int iy, int iHeight);

		// SaveID
		int								IsSaveID();
		const char *					GetSaveID();
		void							SetSaveID(int iValue, const char * c_szSaveID);

		/// Display
		void							GetDisplaySettings();

		int								GetResolutionCount();
		int								GetFrequencyCount(int index);
		bool							GetResolution(int index, OUT DWORD *width, OUT DWORD *height, OUT DWORD *bpp);
		bool							GetFrequency(int index, int freq_index, OUT DWORD *frequncy);
		int								GetResolutionIndex(DWORD width, DWORD height, DWORD bpp);
		int								GetFrequencyIndex(int res_index, DWORD frequency);
		bool							isViewCulling();

		// Sound
		float							GetMusicVolume();
		int								GetSoundVolume();
		void							SetMusicVolume(float fVolume);
		void							SetSoundVolumef(float fVolume);

		int								GetDistance();
#if defined(__BL_ENVIRONMENT_EFFECT_OPTION__)
		void							SetNightModeOption(bool bEnable);
		bool							GetNightModeOption() const;
		
		void							SetSnowModeOption(bool bEnable);
		bool							GetSnowModeOption() const;

		void							SetSnowTextureModeOption(bool bEnable);
		bool							GetSnowTextureModeOption() const;
		void							SetRainModeOption(bool bEnable);
		bool							GetRainModeOption() const;
#endif

		int								GetShadowLevel();
		void							SetShadowLevel(unsigned int level);
#ifdef ENABLE_FOV_OPTION
        float                            GetFOVLevel();
        void                            SetFOVLevel(float fFOV);
#endif
#if defined(__BL_FOG_FIX__)
		void							SetFogMode(bool bEnable);
		bool							GetFogMode() const;
#endif

		float							GetSharpnessPower();
		void							SetSharpnessPower(float fsharpness);

		int								GetGraphicsLevel();
		void							SetGraphicsLevel(unsigned int level);

		int								GetShadowQualityLevel();
		void							SetShadowQualityLevel(unsigned int level);

		int								GetTerrainShadow();
		void							SetTerrainShadow(unsigned int level);

		int								GetTreeShadow();
		void							SetTreeShadow(unsigned int level);

		int								GetRainLevel();
		void							SetRainLevel(unsigned int level);

		int								GetSpecularLevel();
		void							SetSpecularLevel(unsigned int level);

		int								GetBloodLevel();
		void							SetBloodLevel(unsigned int level);

		int								GetGrassType();
		void							SetGrassType(unsigned int level);

	protected:
		TResolution						m_ResolutionList[RESOLUTION_MAX_NUM];
		int								m_ResolutionCount;

		TConfig							m_Config;
		TConfig							m_OldConfig;

		bool							m_isInterfaceConfig;
		PyObject *						m_poInterfaceHandler;
		TWindowStatus					m_WindowStatus[WINDOW_MAX_NUM];
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075