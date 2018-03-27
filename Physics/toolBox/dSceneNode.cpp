//********************************************************************
// Newton Game dynamics 
// copyright 2000-2004
// By Julio Jerez
// VC: 6.0
// basic Hierarchical Scene Node Class
//********************************************************************

#include "stdafx.h"
//#include "OpenGlUtil.h"
#include "dSceneNode.h"
#include "dChunkParsel.h"

//////////////////////////////////////////////////////////////////////
// Construction/Destruction
//////////////////////////////////////////////////////////////////////


//class dTGFileParcel: public dChunkParsel
class dTGFileParcel
{
	public:
	dTGFileParcel(FILE *file)
//		:dChunkParsel (file)
	{
//		RegisterChunk ('MODL', ParceMainChunk1, NULL);

	}

	dInt32 ParceMainChunk1() const;
//	dInt32 ReadModel() const
//	{
//		return 0;
//	}
	

};

// file format chunk ids
enum dgMeshBlockTypes
{
	dgMainBlock													= 'MAIN',
		dgCollisionBlock										= 'COLL',
		dgModelBlock											= 'MODL',
			dgGlobalBBox  										= 'GBOX',
			dgTextureListBlock	 								= 'TEXT',
			dgdMaterialListBlock								= 'MTRL',
				dgMaterialBlock	 								= 'MATE',
			dgMeshBlock											= 'MESH',
				dgNameBlock										= 'NAME',
				dgParentNameBlock								= 'PRNT',
				dgMeshType										= 'MTYP',
				dgMeshHiddenFlag								= 'HIDN',
				dgMeshMatrix									= 'TRNS',
				dgMeshGeometryBlock								= 'GEOM',
					dgMeshGeometryBBox					 		= 'BBOX',
					dgMeshGeometrySegmentBlock					= 'SEGM',
						dgMeshSegmentInfoBlock					= 'INFO',	
							dgMeshSegmentMaterialIndexBlock		= 'MIND',
							dgMeshSegmentTriangleCountBlock 	= 'TRGI',
							dgMeshSegmentStripIndexCountBlock 	= 'STRI',
							dgMeshSegmentVertexCountBlock		= 'VCNT',
							dgMeshSegmentVertexFlagsBlock		= 'VFLG',
							dgMeshSegmentPolygonsBlock			= 'GEOM',	
							dgMeshSegmentIndexArrayBlock 		= 'INDX',
							dgMeshSegmentVertexArrayBlock		= 'VERT',
							dgMeshSegmentNormalArrayBlock		= 'NORM',
							dgMeshSegmentColorArrayBlock		= 'COLR',
							dgMeshSegmentUV0ArrayBlock			= 'UV_0',
							dgMeshSegmentUV1ArrayBlock			= 'UV_1',
							dgMeshSegmentWeightsArrayBlock		= 'WGHT',
	
	FORCE32BIT													= 0x7fffffff
};



// file chunk reader
class dChunkHeader
{
	public:
	dgMeshBlockTypes id;
	unsigned size;

	// read file chunk id and size
	dChunkHeader (FILE* file)
	{
		fread (&id, sizeof (unsigned), 1, file);
		fread (&size, sizeof (unsigned), 1, file);
	}

	// read file chunk id and size, and deduct chunk size
	dChunkHeader	ReadBlock (FILE* file) 
	{
		dChunkHeader block (file);
		size -= sizeof(dChunkHeader);
		size -= block.size;
		return block;
	}

	// skip unknown block chunk
	void SkipBlock (FILE* file) 
	{
		fseek (file, size, SEEK_CUR);
	}
};



class TextureList: public dList<GLuint>
{

	public: 
	TextureList()
		:dList<GLuint>()
	{
	}

	GLuint GetTexture  (int index)  const
	{
		int i;
		dListNode* node;
		
		i = 0;
		for (node = GetFirst(); node; node = node->GetNext()) {
			if (i == index) {
				return node->GetInfo();
			}
			i ++;
		}
		return 0;
	}
};


struct Material
{
	GLuint m_texture;
};

class dMaterialList: public dList<Material>
{

	public: 
	dMaterialList()
		:dList<Material>()
	{
	}

	Material* GetMaterial (int index) const
	{
		int i;
		dListNode* node;
		
		i = 0;
		for (node = GetFirst(); node; node = node->GetNext()) {
			if (i == index) {
				return &node->GetInfo();
			}
			i ++;
		}
		return NULL;
	}
};



dLoaderContext::dLoaderContext ()
{
}

dLoaderContext::~dLoaderContext ()
{
}


// used thais to craate a basic object nodel
dSceneNode* dLoaderContext::CreateNode(char* name, dSceneNode* parent) 
{
	return new dSceneNode();
}





// create and empty model
dSceneNode::dSceneNode()
	:dHierarchy<dSceneNode>(), m_matrix (GetIdentityMatrix())
{

}


// load Model from file
void dSceneNode::LoadFromFile(const char *fileName, dLoaderContext& context)
{
	FILE * file;

	// open file 
	file = fopen (fileName, "rb");
	if (file) {
		dTGFileParcel parcel(file);

		//if the file exist, readt the graphic model
//		Load (file, context);

		// close file when done
//		fclose (file);

		// make sure the matrixes are correct
//		SetMatrix (GetIdentityMatrix());
	}

}

dSceneNode::~dSceneNode()
{
}


dSceneNode::GeometrySegment::GeometrySegment ()
{
	m_uv = NULL;
	m_vertex = NULL;
	m_normal = NULL;
	m_indexes = NULL;
	m_indexCount = 0;
	m_vertexCount = 0;
	m_texture = 0;
};


dSceneNode::GeometrySegment::~GeometrySegment ()
{
	_ASSERTE (0);
}


void dSceneNode::GeometrySegment::AllocData (int vertexCount, int indexCount)
{

	m_indexCount = indexCount;
	m_vertexCount = vertexCount;

	m_vertex = (GLfloat*) malloc (3 * m_vertexCount * sizeof (GLfloat)); 
	m_normal = (GLfloat*) malloc (3 * m_vertexCount * sizeof (GLfloat)); 
	m_uv = (GLfloat*) malloc (2 * m_vertexCount * sizeof (GLfloat)); 
	m_indexes = (GLushort *) malloc (m_indexCount * sizeof (GLushort )); 

	memset (m_uv, 0, 2 * m_vertexCount * sizeof (GLfloat));
}

/*
void dSceneNode::LoadModel (FILE* file, dChunkHeader& parentBlock, dLoaderContext& context)
{
	GLuint texture;
	TextureList textureList;
	dMaterialList dMaterialList;
	char textureName[128];
	char materialName[128];

	while (parentBlock.size) {
		dChunkHeader block (parentBlock.ReadBlock(file));
		switch (block.id) 
		{

			case dgTextureListBlock:
			{
				int i;
				int count;
//				dgSurface *texture;
				
				fread (&count, sizeof (int), 1, file);
				for (i = 0; i < count; i ++) {
//					//dChunkHeader materialBlock (block. ReadBlock(file));
					fread (textureName, 32, 1, file);
					LoadTexture (textureName, texture);
					textureList.Append (texture);
				}
				break;
			}


			case dgdMaterialListBlock:
			{
				int i;
				int j;
				int count;
				int texCount;
				int materialType;
				dFloat specularPower;

				dVector diffuse;
				dVector ambience;
				dVector emissive;
				dVector specular;
				int texChannel[8];
//				GLuint texArray[8];
//				dgMaterial *material;

				fread (&count, sizeof (int), 1, file);
				for (i = 0; i < count; i ++) {
					dChunkHeader materialBlock (block. ReadBlock(file));
					fread (&materialName, 32, 1, file);
					fread (&materialType, sizeof (int), 1, file);

					fread (&ambience, sizeof (dVector), 1, file);
					fread (&diffuse, sizeof (dVector), 1, file);
					fread (&emissive, sizeof (dVector), 1, file);
					fread (&specular, sizeof (dVector), 1, file);
					fread (&specularPower, sizeof (dFloat), 1, file);
					fread (&texCount, sizeof (int), 1, file);

					Material material;
					for (j = 0; j < texCount; j ++) {
						int index;
						fread (&texChannel[j], sizeof (int), 1, file);
						fread (&index, sizeof (int), 1, file);
//						texArray[j] = textureList.GetTexture(index);
						material.m_texture = textureList.GetTexture(index);
					}
//					material = dLoaderContext.CreateMaterial (dgMaterialType (materialType), texCount, texArray);

					dMaterialList.Append (material);

//					dMaterialList.Insert (material, i);
				}
				break;
			}

			case dgMeshBlock:
			{
//				LoadMesh (file, block, root, dLoaderContext, dMaterialList);
				LoadMesh (file, block, this, dMaterialList, context);
				break;
			}


			case dgGlobalBBox:
			{
//				dgSphere	bbox (GetIdentitySphere());
//				file.Read (&bbox.m_size, sizeof (dgVector), 1);
//				break;
			}

			default:
			{
				block.SkipBlock (file);
				break;
			}
		}
	}
}



void dSceneNode::LoadMesh (FILE* file, dChunkHeader& parentBlock, dSceneNode *parent, dMaterialList& materials, dLoaderContext& context)
{
	_ASSERTE (0);

	unsigned type;
	unsigned isInvisible;
	dSceneNode *mesh;
	char name[128];

	dMatrix matrix (GetIdentityMatrix());

	type = 0;
	name[0] = 0;
	isInvisible = FALSE;
	mesh = NULL;

	while (parentBlock.size) {
		dChunkHeader block (parentBlock.ReadBlock(file));
		switch (block.id)  
		{
  			case dgNameBlock:
			{
				fread (name, block.size, 1, file);
				_ASSERTE (name[0]);
				break;
			}

			case dgParentNameBlock:
			{
				char name[256];
				dSceneNode *newParent;
				fread (name, block.size, 1, file);

				_ASSERTE (name[0]);
				newParent = parent->Find (name);
				if (newParent) {
					parent = newParent;
				}
				break;
			}

			case dgMeshType:
			{
				fread (&type, sizeof(int), 1, file);
//				mesh = dLoaderContext.CreateObject(name, type);
//				mesh = new dSceneNode();
				mesh = context.CreateNode (name, parent);
				break;
			}

			case dgMeshHiddenFlag:
			{
				fread (&isInvisible, sizeof(int), 1, file);
				break;
			}

			case dgMeshMatrix:
			{
				fread (&matrix[0][0], sizeof(dFloat), 16, file);
				break;
			}

			case dgMeshGeometryBlock:
			{
//				dgGeometry *geometry;
//				_ASSERTE (mesh);
//				LoadGeometry (file, block, type, dLoaderContext, dMaterialList);
				LoadGeometry (file, block, mesh, materials);
				break;
			}

			default:	
			{
				block.SkipBlock (file);
				break;
			}
		}
	}


	mesh->SetNameID(name);
	mesh->m_localMatrix = matrix;

	if (isInvisible) {
		_ASSERTE (0);
//		dgGeometry *geometry;
//		geometry = mesh->GetGeometry();
//		if (geometry) {
//			geometry->SetVisibilityState(false);
//		}
	}

	mesh->Attach (parent);
//	mesh->Release();

}


void dSceneNode::LoadGeometry (FILE* file, dChunkHeader& parentBlock, dSceneNode* mesh, dMaterialList& materials)
//	FILE& file, 
//	dChunkHeader& parentBlock, 
//	unsigned objectType,
//	const dSceneNodeContext& dLoaderContext,
//	const dMaterialList& dMaterialList) 
{
//	dgGeometry *geometry;
//	geometry = dLoaderContext.CreateGeometry (objectType);

	while (parentBlock.size) {
		dChunkHeader block (parentBlock.ReadBlock(file));
		switch (block.id) 
		{
			case dgMeshGeometrySegmentBlock:
			{
	//			LoadSegment (file, block, geometry, objectType, dLoaderContext, dMaterialList);
				LoadSegment (file, block, mesh, materials);
				break;
			}

			case dgMeshGeometryBBox:
			{
	//			dgSphere	bbox;
	//			file.Read (&bbox[0][0], sizeof (dMatrix),  1);
	//			file.Read (&bbox.m_size, sizeof (dgVector), 1);
	//			geometry->SetSphere (bbox);
	//			break;
			}

			default:	
			{
				block.SkipBlock (file);
				break;
			}
		}
	}

//	return geometry;
}




void dSceneNode::LoadSegment (FILE* file, dChunkHeader& parentBlock, dSceneNode* mesh, dMaterialList& materials)
//	FILE& file, 
//	dChunkHeader& parentBlock, 
//	dgGeometry *geometry,
//	unsigned objectType,
//	const dSceneNodeContext& dLoaderContext,
//	const dMaterialList& dMaterialList) 
{
//	int i;
	bool isStrip;
	int indexCount;
	int vertexFlags;
	int vertexCount;
	int materialIndex;
//	dgSegment* segment;
//	dgUnsigned16 *indexArray;

	isStrip = false;
	indexCount = 0;
	vertexCount = 0;
	vertexFlags	= 0;
	materialIndex = 0;

	// read the mesh chunk eader information
	{
		dChunkHeader infoBlock (parentBlock.ReadBlock(file));
		_ASSERTE (infoBlock.id == dgMeshSegmentInfoBlock);
		while (infoBlock.size) {
			dChunkHeader block (infoBlock.ReadBlock(file));

			switch (block.id) 
			{
				case dgMeshSegmentMaterialIndexBlock:
				{
					fread (&materialIndex, sizeof(int), 1, file);
					break;
				}

				case dgMeshSegmentTriangleCountBlock:
				{
					isStrip = false;
					fread (&indexCount, sizeof(int), 1, file);
					break;
				}

				case dgMeshSegmentStripIndexCountBlock:
				{
					isStrip = true;
					fread (&indexCount, sizeof(int), 1, file);
					break;
				}

				case dgMeshSegmentVertexCountBlock:
				{
					fread (&vertexCount, sizeof(int), 1, file);
					break;
				}

				case dgMeshSegmentVertexFlagsBlock:
				{
					fread (&vertexFlags, sizeof(int), 1, file);
					break;
				}

				default:	
				{
					block.SkipBlock (file);
					break;
				}
			}
		}
	}

	_ASSERTE (vertexCount);
	// add a new geometry segment
	mesh->m_renderList.Append ();

	GeometrySegment& geometry = mesh->m_renderList.GetLast()->GetInfo();
	geometry.AllocData (vertexCount, indexCount);


	geometry.m_texture = materials.GetMaterial(materialIndex)->m_texture;

	// load the vertices and indices
	{
		dChunkHeader geometryBlock (parentBlock.ReadBlock(file));
		_ASSERTE(geometryBlock.id == dgMeshSegmentPolygonsBlock);
		while (geometryBlock.size) {
			dChunkHeader block (geometryBlock.ReadBlock(file));

			switch (block.id) 
			{
				case dgMeshSegmentIndexArrayBlock:
				{
						//int r0;
						//int r1;
						//int r2;
						//int count;
						//bool counterClock;

						//dgStack<int>stripPool(indexCount);
						//file.Read (&stripPool[0], sizeof(int), indexCount);

						//r0 = 0;
						//r1 = 0;
						//count = 0;
						//counterClock = true;
						//for (i = 0; i < indexCount; i ++) {
						//	r2 = stripPool[i];
						//	if (r2 & 0x80000000) {
						//		r2 &= 0x7fffffff;
						//		counterClock = true;
						//	} else {
						//		if (counterClock) {
						//			face[count].m_wireFrame[0] = dgUnsigned16 (r0);
						//			face[count].m_wireFrame[1] = dgUnsigned16 (r1);
						//			face[count].m_wireFrame[2] = dgUnsigned16 (r2);
						//		} else {
						//			face[count].m_wireFrame[0] = dgUnsigned16 (r0);
						//			face[count].m_wireFrame[1] = dgUnsigned16 (r2);
						//			face[count].m_wireFrame[2] = dgUnsigned16 (r1);
						//		}
						//		count ++;
						//		counterClock = !counterClock;
						//	}
						//	r0 = r1; 
						//	r1 = r2;
						//}
						//_ASSERTE (count == triangleCount);

						fread (geometry.m_indexes, sizeof(GLushort), indexCount, file);
						break;
				}

				case dgMeshSegmentVertexArrayBlock:
				{
//						int step;
//						dFloat *ptr;

//						ptr = vertexRecord.vertex.ptr;
//						step = vertexRecord.vertex.stride;
// 						for (i = 0; i < vertexCount; i ++) {
//							dgTriplex val;
//							fread (&val,  sizeof(dgTriplex), 1, file);
//							ptr[0] = val.m_x;
//							ptr[1] = val.m_y;
//							ptr[2] = val.m_z;
//							ptr += step;
//						}
						fread (geometry.m_vertex, 3 * sizeof(GLfloat), vertexCount, file);
						break;
				}

				case dgMeshSegmentNormalArrayBlock:
				{
//					int step;
//					dFloat *ptr;
//					ptr = vertexRecord.normal.ptr;
//					step = vertexRecord.normal.stride;
//					for (i = 0; i < vertexCount; i ++) {
//						dgTriplex val;
//						fread (&val,  sizeof(dgTriplex), 1);
//						ptr[0] = val.m_x;
//						ptr[1] = val.m_y;
//						ptr[2] = val.m_z;
//						ptr += step;
//					}
					fread (geometry.m_normal, 3 * sizeof(GLfloat), vertexCount, file);
					break;
				}


				case dgMeshSegmentUV0ArrayBlock:
				{
//					int step;
//					dFloat *ptr;
//					ptr = vertexRecord.uv_0.ptr;
//					step = vertexRecord.uv_0.stride;
//					for (i = 0; i < vertexCount; i ++) {
//						dgUV val;
//						fread (&file.Read (&val,  sizeof(dgUV), 1);
//						ptr[0] = val.m_u;
//						ptr[1] = val.m_v;
//						ptr += step;
//					}
					fread (geometry.m_uv, 2 * sizeof(GLfloat), vertexCount, file);
					break;
				}

				case dgMeshSegmentUV1ArrayBlock:
//				{
//					int i;
//					dgUV uv;
//
//					vertexProperty.hasUV1 = true;
//					for (i = 0; i < vertexCount; i ++) {
//						Read (&uv.u, sizeof(dgUV), 1);
//						vertexProperty[i].uv0 = uv;
//					}
//					break;
//				}
//
				case dgMeshSegmentColorArrayBlock:
//				{
//					int i;
//					dgColor color;
//					vertexProperty.hasColor = true;
//
//					for (i = 0; i < vertexCount; i ++) {
//						Read (&color.val, sizeof(unsigned), 1);
//						dgVector c (color.rgba.r, color.rgba.g, color.rgba.b, color.rgba.a);
//						vertexProperty[i].color = c.Scale (1.0f / 255.0f);
//					}
//					break;
//				}

				default:	
				{
					_ASSERTE (0);
					block.SkipBlock (file);
					break;
				}
			}
		}
	}

}




// Parece and load dg file format
void dSceneNode::Load (FILE* file, dLoaderContext& context)
{

//	dSceneNode *mesh;
//	dgAABBPolygonSoup	*collision;
//	FILE file (name, "rb");
  
//	mesh = NULL;

	// get the main chunk 
	dChunkHeader	header (file);
	if (header.id == dgMainBlock) {

		// for each chunk read sub chunks
		while (header.size) {

			// Read sub chunk header 
			dChunkHeader block (header.ReadBlock(file));
			switch (block.id) 
			{
				case dgModelBlock:
				{
//					LoadModel (file, this, block, *dLoaderContext);
					LoadModel (file, block, context);
					break;
				}

				case dgCollisionBlock:
				{
					
					//collision = dLoaderContext->GetStaticCollisionPointer();
					//if (collision) {
					//	collision->Load (file);
					//} else {
					//	block.SkipBlock (file);
					//}
					//break;
				}

				default:
				{
					// skip unknown blocks
					block.SkipBlock (file);
					break;
				}
			}

		}
	}
}
*/

void SetLocalMatrix (const dMatrix& matrix)
{
	_ASSERTE (0);
//	m_localMatrix = matrix;
}


void dSceneNode::Render()const
{
	_ASSERTE (0);
/*
	dSceneNode *ptr;
	dList<GeometrySegment>::dListNode *nodes;

	// set the color of the cube's surface
	GLfloat cubeColor[] = { 1.0f, 1.0f, 1.0f, 1.0 };
	glMaterialfv(GL_FRONT, GL_SPECULAR, cubeColor);
	glMaterialfv(GL_FRONT, GL_AMBIENT_AND_DIFFUSE, cubeColor);
	glMaterialf(GL_FRONT, GL_SHININESS, 50.0);

	glEnable(GL_TEXTURE_2D);
	glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);


	glPushMatrix();
	glMultMatrixf(&m_matrix[0][0]);

	for (nodes = m_renderList.GetFirst(); nodes; nodes = nodes->GetNext()) {
		GeometrySegment& segment = nodes->GetInfo();

		glBindTexture(GL_TEXTURE_2D, segment.m_texture);

		glEnableClientState (GL_VERTEX_ARRAY);
		glEnableClientState (GL_NORMAL_ARRAY);
		glEnableClientState (GL_TEXTURE_COORD_ARRAY);
		glVertexPointer (3, GL_FLOAT, 0, segment.m_vertex);
		glNormalPointer (GL_FLOAT, 0, segment.m_normal);
		glTexCoordPointer (2, GL_FLOAT, 0, segment.m_uv);
		glDrawElements (GL_TRIANGLES, segment.m_indexCount, GL_UNSIGNED_SHORT, segment.m_indexes);
	}

	glPopMatrix();

	for (ptr = GetChild(); ptr; ptr = ptr->GetSibling()) {
		ptr->Render();
	}
*/
}




int dSceneNode::GetTotalVectexCount() const
{
	int count;
	dList<GeometrySegment>::dListNode *nodes;

	count = 0;
 	for (nodes = m_renderList.GetFirst(); nodes; nodes = nodes->GetNext()) {
		GeometrySegment& segment = nodes->GetInfo();
		count += segment.m_vertexCount;
	}
	return count;
 }

void dSceneNode::GetVertexArray(dVector* tmpArray) const
{
	int i;
	int count;
	dList<GeometrySegment>::dListNode *nodes;

	count = 0;
 	for (nodes = m_renderList.GetFirst(); nodes; nodes = nodes->GetNext()) {
		GeometrySegment& segment = nodes->GetInfo();
		for (i = 0; i < segment.m_vertexCount; i ++) {
			tmpArray[i].m_x = segment.m_vertex[i * 3 + 0];
			tmpArray[i].m_y = segment.m_vertex[i * 3 + 1];
			tmpArray[i].m_z = segment.m_vertex[i * 3 + 2];
		}
		tmpArray += count;
		count += segment.m_vertexCount;
	}
}

void dSceneNode::CalcBoundingBox (dVector& minBox, dVector& maxBox) const
{
	int i;
	dList<GeometrySegment>::dListNode *nodes;

	minBox = dVector (10.e10f, 1.e10f, 1.e10f); 
	maxBox = dVector (-10.e10f, -1.e10f, -1.e10f); 

 	for (nodes = m_renderList.GetFirst(); nodes; nodes = nodes->GetNext()) {
		GeometrySegment& segment = nodes->GetInfo();
		for (i = 0; i < segment.m_vertexCount; i ++) {
			minBox.m_x = min (minBox.m_x, segment.m_vertex[i * 3 + 0]);
			minBox.m_y = min (minBox.m_y, segment.m_vertex[i * 3 + 1]);
			minBox.m_z = min (minBox.m_z, segment.m_vertex[i * 3 + 2]);

			maxBox.m_x = max (maxBox.m_x, segment.m_vertex[i * 3 + 0]);
			maxBox.m_y = max (maxBox.m_y, segment.m_vertex[i * 3 + 1]);
			maxBox.m_z = max (maxBox.m_z, segment.m_vertex[i * 3 + 2]);
		}
	}
}