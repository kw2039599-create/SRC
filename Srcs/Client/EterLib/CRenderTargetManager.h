#pragma once
#include <unordered_map>
#include <cstdint>
#include <memory>
#include "../EterBase/Singleton.h"
#include "CRenderTarget.h"


class CRenderTargetManager : public CSingleton<CRenderTargetManager>
{
	public:
		CRenderTargetManager();
		virtual ~CRenderTargetManager();

		std::shared_ptr<CRenderTarget> GetRenderTarget(uint8_t index);
		bool CreateRenderTarget(uint8_t index, int width, int height);

		void CreateRenderTargetTextures();
		void ReleaseRenderTargetTextures();

		void Destroy();
		void DeformModels();
		void UpdateModels();
		void RenderBackgrounds();
		void RenderModels();
	protected:
		std::unordered_map<uint8_t, std::shared_ptr<CRenderTarget>> m_renderTargets;
};
