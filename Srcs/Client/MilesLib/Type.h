#pragma once

#include <vector>

namespace NSound
{
	extern std::string strResult;

	typedef struct SSoundData
	{
		float fTime;
		std::string strSoundFileName;
	} TSoundData;
	typedef struct SSoundInstance
	{
		DWORD dwFrame;
		std::string strSoundFileName;
	} TSoundInstance;
	typedef std::vector<TSoundData> TSoundDataVector;
	typedef std::vector<TSoundInstance> TSoundInstanceVector;

	bool			LoadSoundInformationPiece(const char * c_szFileName, TSoundDataVector & rSoundDataVector, const char * c_szPathHeader = NULL);
	bool			SaveSoundInformationPiece(const char * c_szFileName, TSoundDataVector & rSoundDataVector);
	void			DataToInstance(const TSoundDataVector & c_rSoundDataVector, TSoundInstanceVector * pSoundInstanceVector);

	const char *	GetResultString();
	void			SetResultString(const char * c_pszStr);
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
