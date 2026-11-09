#include "StdAfx.h"
#include "SnowParticle.h"
#include "../EterLib/StateManager.h"
#include "../EterLib/Camera.h"
#include "../EterLib/ResourceManager.h"
#include "../EffectLib/EffectManager.h"
#include "../gamelib/FlyingObjectManager.h"
#include "../UserInterface/PythonBackground.h"
#include "../EterLib/GrpScreen.h"

const float c_fSnowDistance = 70000.0f;

std::vector<CSnowParticle*> CSnowParticle::ms_kVct_SnowParticlePool;

// --- SMART WEATHER CHECK ---
static bool IsRainMode()
{
    if (std::strcmp(Frustum::Instance().currentMapNameSave.c_str(), "map_n_snowm_01") == 0)
        return false; // Mount Sohan is ALWAYS snow
        
    return (Frustum::Instance().selectRainFrustun > 0);
}
// ---------------------------

void CSnowParticle::SetCameraVertex(const D3DXVECTOR3& rv3Up, const D3DXVECTOR3& rv3Cross)
{
    if (m_bChangedSize) // IF THIS IS A RAIN PARTICLE
    {
        D3DXVECTOR3 fixedUp(0.0f, 0.0f, 1.0f);
        m_v3Up = fixedUp * m_fHalfHeight;    // Rain uses Height for Vertical length
        m_v3Cross = rv3Cross * m_fHalfWidth; 
    }
    else // IF THIS IS A SNOW PARTICLE
    {
        // Authentic Snow uses the exact math from your original source
        m_v3Up = rv3Up * m_fHalfWidth;       // Snow uses Width for Vertical 
        m_v3Cross = rv3Cross * m_fHalfHeight; 
    }
}

bool CSnowParticle::IsActivate()
{
    return m_bActivate;
}

int snowFog = 0;
int rainController = 0;

void CSnowParticle::Update(float fElapsedTime, const D3DXVECTOR3& c_rv3Pos)
{
    if (m_bChangedSize) // RAIN LOGIC
    {
        UpdateFoudrePhase2(c_rv3Pos, true);

        m_v3Position += m_v3Velocity * fElapsedTime;
        m_v3Position.x += m_v3Cross.x * sin(m_fcurRadian) / 10.0f;
        m_v3Position.y += m_v3Cross.y * sin(m_fcurRadian) / 10.0f;
        m_fcurRadian += m_fPeriod * fElapsedTime;

        if (m_v3Position.z < c_rv3Pos.z - c_fSnowDistance ||
            m_v3Position.x < c_rv3Pos.x - c_fSnowDistance ||
            m_v3Position.x > c_rv3Pos.x + c_fSnowDistance ||
            m_v3Position.y < c_rv3Pos.y - c_fSnowDistance ||
            m_v3Position.y > c_rv3Pos.y + c_fSnowDistance)
        {
            m_bActivate = false;
        }

        if (m_v3Position.z < c_rv3Pos.z - 500.0f)
        {
            float fGrountHeight = CFlyingManager::Instance().GetMapManagerPtr()->GetHeight(m_v3Position.x, m_v3Position.y);

            if (abs(m_v3Position.x - c_rv3Pos.x) > 200.0f || abs(m_v3Position.y - c_rv3Pos.y) > 200.0f)
            {
                if (rainController == 3)
                {
                    CEffectManager& rkEffMgr = CEffectManager::Instance();
                    DWORD dwEffectID = rkEffMgr.CreateEffect("d:/ymir work/effect/etc/ready/rain2.mse", D3DXVECTOR3(m_v3Position.x, m_v3Position.y, fGrountHeight), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
                    
                    if (Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25)
                    {
                        DWORD dwEffectID2 = rkEffMgr.CreateEffect("d:/ymir work/effect/background/fog.mse", D3DXVECTOR3(m_v3Position.x, m_v3Position.y, fGrountHeight), D3DXVECTOR3(0.0f, 0.0f, 0.0f));
                    }
                }
            }
            m_bActivate = false;
        }
        else if (abs(m_v3Position.x - c_rv3Pos.x) > c_fSnowDistance)
            m_bActivate = false;
        else if (abs(m_v3Position.y - c_rv3Pos.y) > c_fSnowDistance)
            m_bActivate = false;

        snowFog++;
        if (snowFog == 5) snowFog = 0;

        rainController++;
        if (rainController == 10) rainController = 0;
    }
    else // SNOW LOGIC
    {
        m_v3Position += m_v3Velocity * fElapsedTime;
        m_v3Position.x += m_v3Cross.x * sin(m_fcurRadian) / 10.0f;
        m_v3Position.y += m_v3Cross.y * sin(m_fcurRadian) / 10.0f;
        m_fcurRadian += m_fPeriod * fElapsedTime;

        if (m_v3Position.z < c_rv3Pos.z - 500.0f)
            m_bActivate = false;
        else if (abs(m_v3Position.x - c_rv3Pos.x) > c_fSnowDistance)
            m_bActivate = false;
        else if (abs(m_v3Position.y - c_rv3Pos.y) > c_fSnowDistance)
            m_bActivate = false;
    }
}

void CSnowParticle::GetVerticies(SParticleVertex& rv3Vertex1, SParticleVertex& rv3Vertex2, SParticleVertex& rv3Vertex3, SParticleVertex& rv3Vertex4)
{
    rv3Vertex1.v3Pos = m_v3Position - m_v3Cross - m_v3Up;
    rv3Vertex1.u = 0.0f;
    rv3Vertex1.v = 0.0f;

    rv3Vertex2.v3Pos = m_v3Position + m_v3Cross - m_v3Up;
    rv3Vertex2.u = 1.0f;
    rv3Vertex2.v = 0.0f;

    rv3Vertex3.v3Pos = m_v3Position - m_v3Cross + m_v3Up;
    rv3Vertex3.u = 0.0f;
    rv3Vertex3.v = 1.0f;

    rv3Vertex4.v3Pos = m_v3Position + m_v3Cross + m_v3Up;
    rv3Vertex4.u = 1.0f;
    rv3Vertex4.v = 1.0f;
}

float CSnowParticle::randomPositionZEclaire(float z) { srand(time(NULL)); return z + (rand() % 100 + (-100)); }
float CSnowParticle::randomPositionXEclaire(float x) { srand(time(NULL)); return x + (rand() % 100 + (-100)); }
float CSnowParticle::randomPositionYEclaire(float y) { srand(time(NULL)); return y + (rand() % 100 + (-100)); }

int rainEffectChanged = false;
void CSnowParticle::UpdateFoudrePhase2(D3DXVECTOR3 m_v3Position, bool executeEclaire)
{
    if (m_bChangedSize) 
    {
        time_t curr_time = time(NULL);
        tm* tm_local = localtime(&curr_time);

        if (Frustum::Instance().selectRainFrustun == 16 || Frustum::Instance().selectRainFrustun == 25)
        {
            Frustum::Instance().rainEffect++;

            if ((tm_local->tm_sec == 10 || tm_local->tm_sec == 30 || tm_local->tm_sec == 50))
            {
                if (!rainEffectChanged)
                {
                    Frustum::Instance().rainEffect = 0;
                    rainEffectChanged = true;
                }
            }
            else
            {
                rainEffectChanged = false;
            }

            if (Frustum::Instance().rainEffect == 0)
            {
                float x = randomPositionXEclaire(m_v3Position.x);
                float y = randomPositionYEclaire(m_v3Position.y);
                float z = randomPositionZEclaire(m_v3Position.z);
            }
        }
    }
}

const char* CSnowParticle::randomSonEclaire() { return "sound/ambience/thunder1.mp3"; }

void CSnowParticle::Init(const D3DXVECTOR3& c_rv3Pos)
{
    float fRot = frandom(0.0f, 36000.0f) / 100.0f;
    float fDistance = frandom(0.0f, c_fSnowDistance) / 10.0f;
    
    m_v3Position.x = c_rv3Pos.x + fDistance * sin((double)D3DXToRadian(fRot));
    m_v3Position.y = c_rv3Pos.y + fDistance * cos((double)D3DXToRadian(fRot));

    if (IsRainMode())
    {
        m_v3Position.z = c_rv3Pos.z + frandom(1500.0f, 2000.0f); 
        m_v3Velocity.x = 0.0f;
        m_v3Velocity.y = 0.0f;
        if (Frustum::Instance().selectRainFrustun == 8)
            m_v3Velocity.z = frandom(-2500.0f, -4500.0f);
        else
            m_v3Velocity.z = frandom(-3500.0f, -5000.0f);

        m_fHalfWidth = frandom(25.0f, 40.0f);
        if (Frustum::Instance().selectRainFrustun == 8)
            m_fHalfHeight = m_fHalfWidth * 1.4f;
        else
            m_fHalfHeight = m_fHalfWidth * 2.2f;

        m_bActivate = true;
        m_bChangedSize = true; // MARKS AS RAIN
        m_fPeriod = 0.1f;
        m_fcurRadian = frandom(-1.6f, 1.6f);
        m_fAmplitude = frandom(1.0f, 3.0f);
    }
    else
    {
        m_v3Position.z = c_rv3Pos.z + frandom(100.0f, 2000.0f); 
        m_v3Velocity.x = 0.0f;
        m_v3Velocity.y = 0.0f;
        m_v3Velocity.z = frandom(-50.0f, -200.0f); 
        
        m_fHalfWidth = frandom(2.0f, 7.0f); 
        m_fHalfHeight = m_fHalfWidth;
        
        m_bActivate = true;
        m_bChangedSize = false; // MARKS AS SNOW
        m_fPeriod = frandom(1.5f, 5.0f);
        m_fcurRadian = frandom(-1.6f, 1.6f);
        m_fAmplitude = frandom(1.0f, 3.0f);
    }
}

CSnowParticle* CSnowParticle::New()
{
    if (ms_kVct_SnowParticlePool.empty())
        return new CSnowParticle;
        
    CSnowParticle* pParticle = ms_kVct_SnowParticlePool.back();
    ms_kVct_SnowParticlePool.pop_back();
    return pParticle;
}

void CSnowParticle::Delete(CSnowParticle* pSnowParticle) { ms_kVct_SnowParticlePool.push_back(pSnowParticle); }
void CSnowParticle::DestroyPool() { stl_wipe(ms_kVct_SnowParticlePool); }
CSnowParticle::CSnowParticle() {}
CSnowParticle::~CSnowParticle() {}