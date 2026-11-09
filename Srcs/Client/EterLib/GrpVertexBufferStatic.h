#pragma once

#include "GrpVertexBuffer.h"

class CStaticVertexBuffer : public CGraphicVertexBuffer
{
	public:
		CStaticVertexBuffer();
		virtual ~CStaticVertexBuffer();

		bool Create(int vtxCount, DWORD fvf, bool isManaged=true);
};
//martysama0134's fb0d8d5d0566583187a94f9a823b2075
