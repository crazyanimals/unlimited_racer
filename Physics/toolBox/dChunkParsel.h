// dChunkParsel.h: interface for the dChunkParsel class.
//
//////////////////////////////////////////////////////////////////////

#if !defined(AFX_DCHUNKPARSEL_H__9136D67A_D7DF_455D_B281_A430D389C265__INCLUDED_)
#define AFX_DCHUNKPARSEL_H__9136D67A_D7DF_455D_B281_A430D389C265__INCLUDED_

#include "stdafx.h"
#include "dTree.h"


class dChunkParsel;
typedef dInt32 (dChunkParsel::*ChunkPosses) () const;

struct dChunkDescriptor
{
	ChunkPosses m_readFnction;
	const char* m_descrition;
};

class dChunkParsel: public dTree<dChunkDescriptor, dUnsigned32>  
{
	public:
	dChunkParsel(FILE * file);
	virtual ~dChunkParsel();


	void ParceFile();

	protected:
	void RegisterChunk (dUnsigned32 m_chunkID, ChunkPosses fnt, const char* description); 


	dInt32 ParceMainChunk() const;

};

#endif 
