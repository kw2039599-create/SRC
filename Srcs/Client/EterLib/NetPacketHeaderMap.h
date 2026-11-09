#pragma once

#include <map>

class CNetworkPacketHeaderMap
{
	public:
#ifdef ENABLE_LARGE_DYNAMIC_PACKETS
		typedef struct SPacketType
		{
			SPacketType(int iSize = 0, int bType = 0)
			{
				iPacketSize = iSize;
				iPacketType = bType;
			}

			int iPacketSize;
			int iPacketType;
		} TPacketType;
#else
		typedef struct SPacketType
		{
			SPacketType(int iSize = 0, bool bFlag = false)
			{
				iPacketSize = iSize;
				isDynamicSizePacket = bFlag;
			}

			int iPacketSize;
			bool isDynamicSizePacket;
		} TPacketType;
#endif

	public:
		CNetworkPacketHeaderMap();
		virtual ~CNetworkPacketHeaderMap();

		void Set(int header, const TPacketType & rPacketType);
		bool Get(int header, TPacketType * pPacketType);

	protected:
		std::map<int, TPacketType> m_headerMap;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
