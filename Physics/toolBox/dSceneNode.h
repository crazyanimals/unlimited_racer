//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// basic Hierarchical Scene Node Class  
//********************************************************************


#if !defined(AFX_SIMPLESCENENODE_H__B1AC6903_8356_4935_882F_000492C28410__INCLUDED_)
#define AFX_SIMPLESCENENODE_H__B1AC6903_8356_4935_882F_000492C28410__INCLUDED_

#include "stdafx.h"
#include "dBaseHierarchy.h"

class dChunkHeader;
class dMaterialList;
class dSceneNode;

class dLoaderContext
{
	public:
	dLoaderContext ();
	virtual ~dLoaderContext ();
	virtual dSceneNode* CreateNode(char* name, dSceneNode* parent);
};

class dSceneNode: public dHierarchy<dSceneNode>
{
	struct GeometrySegment
	{
		GeometrySegment ();
		~GeometrySegment ();
		void AllocData (int vertexCount, int indexCount);

		GLuint m_texture;
		int m_indexCount;
		int m_vertexCount;
		GLfloat *m_uv;
		GLfloat *m_vertex;
		GLfloat *m_normal;
		GLushort *m_indexes;
	};

	public:
	dSceneNode();
	virtual ~dSceneNode();


	dMatrix& GetMatrix ();
	void SetMatrix (const dMatrix& matrix);

	int GetTotalVectexCount() const;
	void GetVertexArray(dVector* tmpArray) const;
	void CalcBoundingBox (dVector& min, dVector& max) const; 

	void LoadFromFile(const char *fileName, dLoaderContext& context);

	protected:
	virtual void Render() const;

//	void Load(const char* file, dLoaderContext& context);
//	void LoadModel (FILE* file, dChunkHeader& header, dLoaderContext& context);
//	void LoadMesh (FILE* file, dChunkHeader& header, dSceneNode* root, dMaterialList& materials, dLoaderContext& context);
//	void LoadGeometry (FILE* file, dChunkHeader& header, dSceneNode* mesh, dMaterialList& materials);
//	void LoadSegment (FILE* file, dChunkHeader& header, dSceneNode* mesh, dMaterialList& materials);

	dMatrix m_matrix;
	dList<GeometrySegment> m_renderList;
};


inline dMatrix& dSceneNode::GetMatrix ()
{
	return m_matrix;
}

inline void dSceneNode::SetMatrix (const dMatrix& matrix)
{
	m_matrix = matrix;	
}


#endif 
