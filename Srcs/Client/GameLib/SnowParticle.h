#pragma once

struct SParticleVertex
{
	D3DXVECTOR3 v3Pos;
	float u, v;
};

struct BlurVertex
{
	D3DXVECTOR3 pos;
	FLOAT       rhw;
	DWORD       color;
	FLOAT		tu, tv;

	static const DWORD FVF;

	BlurVertex(D3DXVECTOR3 p, float w, DWORD c, float u, float v) :pos(p), rhw(w), color(c), tu(u), tv(v) {}
	~BlurVertex() {};
};

class CSnowParticle
{
public:
	CSnowParticle();
	~CSnowParticle();

	static CSnowParticle* New();
	static void Delete(CSnowParticle* pSnowParticle);
	static void DestroyPool();

	void Init(const D3DXVECTOR3& c_rv3Pos);

	void SetCameraVertex(const D3DXVECTOR3& rv3Up, const D3DXVECTOR3& rv3Cross);
	bool IsActivate();

	void Update(float fElapsedTime, const D3DXVECTOR3& c_rv3Pos);
	void GetVerticies(SParticleVertex& rv3Vertex1, SParticleVertex& rv3Vertex2,
		SParticleVertex& rv3Vertex3, SParticleVertex& rv3Vertex4);

	float randomPositionZEclaire(float z);
	float randomPositionXEclaire(float x);
	float randomPositionYEclaire(float y);

	void UpdateFoudrePhase2(D3DXVECTOR3 m_v3Position, bool executeEclaire);

	const char* randomSonEclaire();

protected:
	bool m_bActivate;
	bool m_bChangedSize;
	float m_fHalfWidth;
	float m_fHalfHeight;

	D3DXVECTOR3 m_v3Velocity;
	D3DXVECTOR3 m_v3Position;

	D3DXVECTOR3 m_v3Up;
	D3DXVECTOR3 m_v3Cross;

	float m_fPeriod;
	float m_fcurRadian;
	float m_fAmplitude;
	float m_fRotationSpeed;
	float m_fAngle = 30.0f;

public:
	static std::vector<CSnowParticle*> ms_kVct_SnowParticlePool;
};


//martysama0134's fb0d8d5d0566583187a94f9a823b2075
