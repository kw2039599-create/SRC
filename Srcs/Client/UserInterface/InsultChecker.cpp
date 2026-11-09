#include "StdAfx.h"
#include "InsultChecker.h"
#include <vector>
#include <cctype>

CInsultChecker& CInsultChecker::GetSingleton()
{
	static CInsultChecker s_kInsultChecker;
	return s_kInsultChecker;
}

CInsultChecker::CInsultChecker()
{
}

CInsultChecker::~CInsultChecker()
{
}

void CInsultChecker::Clear()
{
	m_kList_stInsult.clear();
}

void CInsultChecker::AppendInsult(const std::string& c_rstInsult)
{
	std::string strInsult = c_rstInsult;

	// إزالة المسافات الزائدة ورموز السطر الجديد من البداية والنهاية
	while (!strInsult.empty() && (strInsult.back() == '\r' || strInsult.back() == '\n' || strInsult.back() == ' ' || strInsult.back() == '\t'))
		strInsult.pop_back();
	while (!strInsult.empty() && (strInsult.front() == ' ' || strInsult.front() == '\t'))
		strInsult.erase(0, 1);

	if (strInsult.empty())
		return;

	// تحويل تلقائي من UTF-8 إلى Windows-1256 في حال كان الملف محفوظاً بترميز UTF-8
	int wlen = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strInsult.c_str(), -1, NULL, 0);
	if (wlen > 0)
	{
		std::vector<wchar_t> wstr(wlen);
		if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, strInsult.c_str(), -1, &wstr[0], wlen) > 0)
		{
			int alen = WideCharToMultiByte(1256, 0, &wstr[0], -1, NULL, 0, NULL, NULL);
			if (alen > 1)
			{
				std::vector<char> astr(alen);
				if (WideCharToMultiByte(1256, 0, &wstr[0], -1, &astr[0], alen, NULL, NULL) > 0)
				{
					strInsult = &astr[0];
				}
			}
		}
	}

	if (!strInsult.empty())
		m_kList_stInsult.push_back(strInsult);
}

bool CInsultChecker::__GetInsultLength(const char* c_szWord, UINT* puInsultLen)
{
	UINT uMaxLen = 0;

	for (std::list<std::string>::iterator i = m_kList_stInsult.begin(); i != m_kList_stInsult.end(); ++i)
	{
		std::string& rstInsult = *i;
		size_t insultLen = rstInsult.length();

		if (insultLen == 0 || insultLen <= uMaxLen)
			continue;

		bool bMatch = true;
		for (size_t j = 0; j < insultLen; ++j)
		{
			unsigned char c1 = (unsigned char)c_szWord[j];
			unsigned char c2 = (unsigned char)rstInsult[j];

			if (c1 == 0)
			{
				bMatch = false;
				break;
			}

			// مقارنة غير حساسة لحالة الأحرف للإنجليزية، ومطابقة مباشرة للعربية (Windows-1256)
			if (c1 < 128 && c2 < 128)
			{
				if (std::tolower(c1) != std::tolower(c2))
				{
					bMatch = false;
					break;
				}
			}
			else
			{
				if (c1 != c2)
				{
					bMatch = false;
					break;
				}
			}
		}

		if (bMatch)
		{
			uMaxLen = insultLen;
		}
	}

	if (uMaxLen > 0)
	{
		*puInsultLen = uMaxLen;
		return true;
	}

	return false;
}

bool CInsultChecker::__IsInsult(const char* c_szWord)
{
	UINT uInsultLen;
	return __GetInsultLength(c_szWord, &uInsultLen);
}

void CInsultChecker::FilterInsult(char* szLine, UINT uLineLen)
{
	const char INSULT_FILTER_CHAR = '*';
	for (UINT uPos = 0; uPos < uLineLen;)
	{
		UINT uInsultLen = 0;
		if (__GetInsultLength(szLine + uPos, &uInsultLen) && uInsultLen > 0)
		{
			memset(szLine + uPos, INSULT_FILTER_CHAR, uInsultLen);
			uPos += uInsultLen;
		}
		else
		{
			// التحرك بمقدار 1 بايت دائماً لدعم الحروف العربية (Windows-1256) والإنجليزية
			uPos++;
		}
	}
}

bool CInsultChecker::IsInsultIn(const char* c_szLine, UINT uLineLen)
{
	for (UINT uPos = 0; uPos < uLineLen; ++uPos)
	{
		if (__IsInsult(c_szLine + uPos))
			return true;
	}

	return false;
}