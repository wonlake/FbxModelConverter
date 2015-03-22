#pragma once

class MFMesh;

class MFSkeleton
{
public:
	MFSkeleton(MFMesh* pMesh);
	~MFSkeleton(void);

	void Serialize(std::fstream& fs);

	MFMesh* m_pMesh;
};

