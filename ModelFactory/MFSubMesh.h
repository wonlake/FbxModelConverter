#pragma once
class MFSubMesh
{
public:
	MFSubMesh(void);
	~MFSubMesh(void);

	int m_numTriangles;

	std::vector<unsigned int> m_vecIndices;
	std::vector<float> m_vecPos;
	std::vector<float> m_vecNormal;		
	std::vector<float> m_vecTangent;
	std::vector<float> m_vecUV;
	std::vector<int> m_vecBoneIndex;
	std::vector<float> m_vecBoneWeight;

	std::string m_strDiffuseMap;
	std::string m_strNormalMap;
	std::string m_strSpecularMap;
};

