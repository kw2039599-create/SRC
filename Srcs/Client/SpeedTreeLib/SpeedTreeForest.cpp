#include "StdAfx.h"
#include <vector>
#include "../eterBase/Filename.h"
#include "../eterBase/MappedFile.h"
#include "../eterPack/EterPackManager.h"
#include "SpeedTreeForest.h"
#include "SpeedTreeConfig.h"
#include <cfloat>
#include <time.h>


CSpeedTreeForest::CSpeedTreeForest() : m_afLighting{}, m_afFog{}, m_fWindStrength(0.0f), m_fAccumTime(0)
{
	CSpeedTreeRT::SetNumWindMatrices(c_nNumWindMatrices);

	m_afForestExtents[0] = m_afForestExtents[1] = m_afForestExtents[2] = FLT_MAX;
	m_afForestExtents[3] = m_afForestExtents[4] = m_afForestExtents[5] = -FLT_MAX;
}

CSpeedTreeForest::~CSpeedTreeForest()
{
	Clear();
}

void CSpeedTreeForest::Clear()
{
	if (m_pMainTreeMap.begin() != m_pMainTreeMap.end())
		m_pMainTreeMap.clear();
}

CSpeedTreeForest::SpeedTreeWrapperPtr CSpeedTreeForest::GetMainTree(DWORD dwCRC)
{
	TTreeMap::iterator itor = m_pMainTreeMap.find(dwCRC);

	if (itor == m_pMainTreeMap.end())
		return NULL;

	return itor->second;
}

BOOL CSpeedTreeForest::GetMainTree(DWORD dwCRC, SpeedTreeWrapperPtr& ppMainTree, const char* c_pszFileName)
{
	TTreeMap::iterator itor = m_pMainTreeMap.find(dwCRC);

	SpeedTreeWrapperPtr pTree;

	if (itor != m_pMainTreeMap.end())
		pTree = itor->second;
	else
	{
		CMappedFile file;
		LPCVOID c_pvData;

		// NOTE : 파일이 없을때는 return FALSE 아닌가요? - [levites]
		if (!CEterPackManager::Instance().Get(file, c_pszFileName, &c_pvData))
			return FALSE;

		pTree = std::make_shared<CSpeedTreeWrapper>();

		if (!pTree->LoadTree(c_pszFileName, (const BYTE*)c_pvData, file.Size()))
		{
			pTree.reset();
			return FALSE;
		}

		m_pMainTreeMap.emplace(dwCRC, pTree);

		file.Destroy();
	}

	ppMainTree = pTree;
	return TRUE;
}

CSpeedTreeForest::SpeedTreeWrapperPtr CSpeedTreeForest::CreateInstance(float x, float y, float z, DWORD dwTreeCRC, const char* c_szTreeName)
{
	SpeedTreeWrapperPtr pMainTree;
	if (!GetMainTree(dwTreeCRC, pMainTree, c_szTreeName))
		return NULL;

	SpeedTreeWrapperPtr pTreeInst = pMainTree->MakeInstance();
	pTreeInst->SetPosition(x, y, z);
	pTreeInst->RegisterBoundingSphere();
	return pTreeInst;
}

void CSpeedTreeForest::DeleteInstance(SpeedTreeWrapperPtr pInstance)
{
	if (!pInstance)
		return;

	SpeedTreeWrapperPtr pParentTree = pInstance->InstanceOf();

	if (!pParentTree)
		return;

	pParentTree->DeleteInstance(pInstance);
}

void CSpeedTreeForest::UpdateSystem(float fCurrentTime)
{
	static float fLastTime = fCurrentTime;
	float fElapsedTime = fCurrentTime - fLastTime;
	CSpeedTreeRT::SetTime(fElapsedTime);

	m_fAccumTime += fElapsedTime;
	SetupWindMatrices(m_fAccumTime);
}

void CSpeedTreeForest::AdjustExtents(float x, float y, float z)
{
	m_afForestExtents[0] = __min(m_afForestExtents[0], x);
	m_afForestExtents[1] = __min(m_afForestExtents[1], y);
	m_afForestExtents[2] = __min(m_afForestExtents[2], z);
	m_afForestExtents[3] = __max(m_afForestExtents[3], x);
	m_afForestExtents[4] = __max(m_afForestExtents[4], y);
	m_afForestExtents[5] = __max(m_afForestExtents[5], z);
}

void CSpeedTreeForest::SetWindStrength(float fStrength)
{
	m_fWindStrength = fStrength;

	TTreeMap::iterator itor = m_pMainTreeMap.begin();
	UINT uiCount;

	while (itor != m_pMainTreeMap.end())
	{
		SpeedTreeWrapperPtr pMainTree = (itor++)->second;
		std::list<CSpeedTreeForest::SpeedTreeWrapperWeakPtr> ppInstances = pMainTree->GetInstances(uiCount);

		for (auto it : ppInstances)
		{
			if (it.expired())
				continue;

			auto pInstance = it.lock();

			pInstance->GetSpeedTree()->SetWindStrength(Frustum::Instance().windSpeed);
		}
	}
}

void CSpeedTreeForest::SetupWindMatrices(float fTimeInSecs)
{
	// matrix computational data
	static float afMatrixTimes[c_nNumWindMatrices] = { 0.0f };
	static float afFrequencies[c_nNumWindMatrices][2] =
	{
		{ 0.12f, 0.52f },
		{ 0.06f, 0.20f },
		{ 0.10f, 0.15f },
		{ 0.27f, 0.40f }
	};

	// compute time since last call
	static float fTimeOfLastCall = 0.0f;
	float fTimeSinceLastCall = fTimeInSecs - fTimeOfLastCall;
	fTimeOfLastCall = fTimeInSecs;

	// wind strength
	static float fOldStrength = m_fWindStrength;

#ifdef __ANDROID__
	if (m_fWindStrength <= 0.05f)
		m_fWindStrength = 0.35f;
#endif

	// increment matrix times
	for (int i = 0; i < c_nNumWindMatrices; ++i)
		afMatrixTimes[i] += 1.1f;

	// compute maximum branch throw
	float const fBaseAngle = m_fWindStrength * 40.0f;

	// build rotation matrices
	for (int j = 0; j < c_nNumWindMatrices; ++j)
	{
		// adjust time to prevent "jumping"
		if (m_fWindStrength != 0.0f)
			afMatrixTimes[j] = ((afMatrixTimes[j] * fOldStrength) / m_fWindStrength) / 1.0f;

		// compute percentages for each axis
		float const fBaseFreq = 0.01f;
		float const fXPercent = sinf(fBaseFreq * afFrequencies[j % c_nNumWindMatrices][0] * (afMatrixTimes[j]));
		float const fYPercent = cosf(fBaseFreq * afFrequencies[j % c_nNumWindMatrices][1] * (afMatrixTimes[j]));


		// build compound rotation matrix (rotate on 'x' then on 'y')
		const float c_fDeg2Rad = 57.2957795f;
		float fSinX = sinf(fBaseAngle * fXPercent / c_fDeg2Rad);
		float fSinY = sinf(fBaseAngle * fYPercent / c_fDeg2Rad);
		float fCosX = cosf(fBaseAngle * fXPercent / c_fDeg2Rad);
		float fCosY = cosf(fBaseAngle * fYPercent / c_fDeg2Rad);

		float afMatrix[16] = { 0.0f };
		afMatrix[0] = fCosY;
		afMatrix[2] = -fSinY;
		afMatrix[4] = fSinX * fSinY;
		afMatrix[5] = fCosX;
		afMatrix[6] = fSinX * fCosY;
		afMatrix[8] = fSinY * fCosX;
		afMatrix[9] = -fSinX;
		afMatrix[10] = fCosX * fCosY;
		afMatrix[15] = 1.0f;

#if defined(WRAPPER_USE_CPU_WIND) || defined(__ANDROID__)
		CSpeedTreeRT::SetWindMatrix(j, afMatrix);
#endif

#ifdef JTX_GPU_TREE_WIND
		// graphics API specific
		UploadWindMatrix(c_nVertexShader_WindMatrices + j * 4, afMatrix);

#endif
	}

	// track wind strength
	fOldStrength = m_fWindStrength;
}

void CSpeedTreeForest::SetLight(const float* afDirection, const float* afAmbient, const float* afDiffuse)
{
	m_afLighting[0] = afDirection[0];
	m_afLighting[1] = afDirection[1];
	m_afLighting[2] = afDirection[2];
	m_afLighting[3] = 1.0f;

	m_afLighting[4] = afAmbient[0] - 0.2;
	m_afLighting[5] = afAmbient[1] - 0.2;
	m_afLighting[6] = afAmbient[2] - 0.2;
	m_afLighting[7] = afAmbient[3] - 0.2;

	m_afLighting[8] = afDiffuse[0];
	m_afLighting[9] = afDiffuse[1];
	m_afLighting[10] = afDiffuse[2];
	m_afLighting[11] = afDiffuse[3];
}

void CSpeedTreeForest::SetFog(float fFogNear, float fFogFar)
{

	const float c_fFogLinearScale = (1.0f / (fFogFar - fFogNear));

	m_afFog[0] = fFogNear;
	m_afFog[1] = fFogFar;
	m_afFog[2] = c_fFogLinearScale;
	m_afFog[3] = 0.0f;
}

//martysama0134's fb0d8d5d0566583187a94f9a823b2075
