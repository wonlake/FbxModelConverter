#pragma once
#include "MFSubMesh.h"

struct BonePose
{
	std::string m_strName;
	float transform[10];
};

struct MFVertex
{
	float pos[3];
	float normal[3];
	float tangent[3];
	float uv[2];
	int   boneIdx[4];
	float weight[4];
	int   vertexId;

	MFVertex()
	{
		memset(this, 0, sizeof(MFVertex));
	}
};

class MFMesh
{
public:
	MFMesh(void);
	~MFMesh(void);

	void Parse(FbxNode* pNode, MFMesh* pMergeMesh);
	void Serialize(std::fstream& fs);

	std::vector<MFSubMesh*> m_vecSubMesh;
	std::vector<std::tuple<std::string, FbxAMatrix, FbxCluster*> > m_vecBonePoseInverse;
	std::map<std::string, int> m_mapBoneUsed;

	FbxAMatrix m_transform;
	std::string m_strName;
};

