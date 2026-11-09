#pragma once
#ifdef CShield_EXPORTS
#define CShield_API __declspec(dllexport)
#else
#define CShield_API __declspec(dllimport)
#endif

#include "CShieldPointerEncryptionPublic.hpp"

typedef struct _CShieldData {
	DWORD dwThreadId;
	HANDLE hThread;
	DWORD_PTR dwStartAddress;
	DWORD_PTR dwUser32Low;
	DWORD_PTR dwUser32Hi;
	DWORD dwMemType;
	DWORD_PTR dwMemAllocationBase;
} CShieldData;

extern CShieldData myCShieldData;
extern CShield_API DWORD cshieldClientCode;
extern CShield_API BYTE cshieldWin;
extern CShield_API std::string cshieldLanguage;
extern CShield_API std::string cshieldReport;
extern CShield_API std::string cshieldCharName;

CShield_API CShieldData InitializeCShield(const std::vector<std::string> serverAddresses, const unsigned long paddingBytes = 16);
CShield_API bool CheckAttackspeed(float attackSpeed, float range, bool twoHanded, bool twoHandedHorse);
CShield_API bool CheckMovespeed(float moveSpeed);
CShield_API bool CheckValues();
CShield_API bool CheckValuesCython();
CShield_API bool CheckMove(bool active, bool mov, float xPos, float yPos);
CShield_API std::string GenKey(bool isGamePhase, DWORD num);
CShield_API std::string GenData();
CShield_API std::string GetCShieldLoginKey(const char* name);
CShield_API std::string GetCShieldCaptcha(const char* captcha);
CShield_API std::string eData(const void* data, size_t size); // deprecated
template<typename T> CShield_API T dData(const std::string& eData); // deprecated
template CShield_API LONG dData<LONG>(const std::string&); // deprecated
template CShield_API DWORD dData<DWORD>(const std::string&); // deprecated
template CShield_API time_t dData<time_t>(const std::string&); // deprecated
CShield_API std::string GetH();
CShield_API void bPlayer();
