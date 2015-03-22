#pragma once

class MFMesh;

class MFClip
{
public:
	MFClip(MFMesh* pMesh);
	~MFClip(void);

	void Serialize(std::fstream& fs);

	MFMesh* m_pMesh;
};

