#pragma once

class MFAnimation
{
public:
	MFAnimation(void);
	~MFAnimation(void);

	void ParseScene(FbxNode* pRootNode);
	void ParseSkeletonJoint(FbxNode* pNode);
	void ParseNull(FbxNode* pNode);
	void Serialize();

	std::vector<std::pair<std::string, std::string> > m_vecSkeletonJoints;
	std::vector<std::pair<int, FbxNode*> > m_vecSkeletonJointNodes;

	int m_numFrameRate;
	int m_numDuration;
	int m_numTotalFrames;
};

