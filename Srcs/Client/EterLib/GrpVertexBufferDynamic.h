#pragma once

#include "GrpVertexBuffer.h"

class CDynamicVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CDynamicVertexBuffer();
		virtual ~CDynamicVertexBuffer();

		bool Create(int vtxCount, int fvf);

	protected:
		int m_vtxCount;
		int m_fvf;
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
