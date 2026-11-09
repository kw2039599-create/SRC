#if defined(ENABLE_NPC_LOCATION_HELPER)
#include "Packet.h"

class CPythonNPCLocationHelper : public CSingleton<CPythonNPCLocationHelper>
{
public:
	CPythonNPCLocationHelper();
	virtual ~CPythonNPCLocationHelper() = default;

	void Clear();
	void SetStatus(const TPacketGCNPCLocationHelper& packet);
	void SetWarpResult(const TPacketGCNPCLocationHelper& packet);

	bool IsActive() const;
	DWORD GetCooldownRemain() const;
	BYTE GetLastResult() const;

private:
	bool m_bActive;
	DWORD m_dwCooldownRemain;
	BYTE m_bLastResult;
};

void initNPCLocationHelper();
#endif
