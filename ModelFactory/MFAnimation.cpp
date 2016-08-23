#include "StdAfx.h"
#include "MFAnimation.h"
#include "GlobalConfig.h"
#include <tinyxml/tinyxml.h>

MFAnimation::MFAnimation(void)
{
	m_numFrameRate = 100;
	m_numDuration = 0;
	m_numTotalFrames = 0;
}


MFAnimation::~MFAnimation(void)
{
}


void MFAnimation::ParseNull(FbxNode* pNode)
{
	m_vecSkeletonJoints.push_back(std::make_pair(pNode->GetName(), ""));
	m_vecSkeletonJointNodes.push_back(std::make_pair(0, pNode));
}

void MFAnimation::ParseSkeletonJoint(FbxNode* pNode)
{
	FbxSkeleton* pSkeleton = (FbxSkeleton*)pNode->GetNodeAttribute();
	switch( pSkeleton->GetSkeletonType() )
	{
	case FbxSkeleton::eRoot:
		m_vecSkeletonJoints.push_back(std::make_pair(pNode->GetName(), ""));
		break;
	case FbxSkeleton::eLimb:
		m_vecSkeletonJoints.push_back(std::make_pair(pNode->GetName(), pNode->GetParent()->GetName()));
		break;
	case FbxSkeleton::eLimbNode:
		m_vecSkeletonJoints.push_back(std::make_pair(pNode->GetName(), pNode->GetParent()->GetName()));
		break;
	case FbxSkeleton::eEffector:
		m_vecSkeletonJoints.push_back(std::make_pair(pNode->GetName(), ""));
		break;
	}
	m_vecSkeletonJointNodes.push_back(std::make_pair(0, pNode));
}

void MFAnimation::ParseScene(FbxNode* pRootNode)
{
	FbxScene* pScene = pRootNode->GetScene();
	size_t numAnimStacks = pScene->GetSrcObjectCount<FbxAnimStack>();
	if( numAnimStacks < 1 )
		return;
	FbxAnimStack* pAnimStack = pScene->GetSrcObject<FbxAnimStack>(0);

	m_numFrameRate = GlobalConfig::GetSingleton()->m_uiFrameRate;
	m_numDuration = (int)pAnimStack->GetLocalTimeSpan().GetDuration().GetMilliSeconds();

	if( GlobalConfig::GetSingleton()->m_bNeedTweakFrames )
	{
		TiXmlDocument doc;
		std::string strTweakConfig = GlobalConfig::GetSingleton()->m_strRootOutputPath +
			"/tweakframes.xml";
		doc.LoadFile( strTweakConfig.c_str() );	
		auto rootElement = doc.RootElement();
		auto pChild = rootElement->FirstChildElement();
		std::string fbxname = GlobalConfig::GetSingleton()->m_strFbxFilename;
		do 
		{
			if( pChild == NULL )
				break;

			auto name = pChild->Attribute("name");
			auto targetName = pChild->GetText();

			std::string strAnimationName = GlobalConfig::GetSingleton()->m_strFbxFilename;
			std::transform( strAnimationName.begin(), strAnimationName.end(), strAnimationName.begin(), std::tolower);

			int pos1 = strAnimationName.rfind("_");
			if( pos1 != std::string::npos )
			{
				strAnimationName = strAnimationName.substr(pos1 + 1); 
			}
			std::string strPartName = GlobalConfig::GetSingleton()->m_strFbxFilename.substr(0, pos1);
			if( strPartName != name )
				continue;
			std::string full_name = targetName;
			full_name += "_" + strAnimationName;
			std::transform( full_name.begin(), full_name.end(), full_name.begin(), std::tolower );
			std::string strAnimationFile = GlobalConfig::GetSingleton()->m_strAnimationOutputPath + "/" + full_name + ".anim";
			std::fstream fsAnimationFile( strAnimationFile, std::ios::in | std::ios::binary);
			if( fsAnimationFile.is_open() )
			{
				int numFrames = 0;
				int numJoints = 0;
				fsAnimationFile.read( (char*)&numFrames, sizeof(int));
				fsAnimationFile.read( (char*)&numJoints, sizeof(int));
				int totalDuration = 0;
				for( int i = 0; i < numFrames; ++i )
				{
					int duration = 0;
					fsAnimationFile.read( (char*)&duration, sizeof(int) );
					totalDuration += duration;
					if( i == 0 )
						m_numFrameRate = duration;

					for( int j = 0; j < numJoints; ++j )
					{
						int nameSize = 0;
						fsAnimationFile.read( (char*)&nameSize, sizeof(int));
						char* pJointName = new char[nameSize];
						fsAnimationFile.read( (char*)pJointName, nameSize);
						float transformation[10];
						fsAnimationFile.read( (char*)transformation, 40 );
						delete[] pJointName;
					}
				}
				fsAnimationFile.close();

				if( m_numDuration < totalDuration )
				{
					break;
				}

				m_numDuration = totalDuration;
			}
		} while ( pChild = pChild->NextSiblingElement());

	}

	if( m_numDuration < m_numFrameRate && m_numDuration > 1 )
	{
		m_numFrameRate = m_numDuration - 1;
	}
	if( m_numDuration % m_numFrameRate == 0 )
		m_numTotalFrames = m_numDuration / m_numFrameRate;
	else
		m_numTotalFrames = m_numDuration / m_numFrameRate + 1;

	int numChildren = pRootNode->GetChildCount();
	for( int i = 0; i < numChildren; ++i )
	{
		FbxNode* pNode = pRootNode->GetChild(i);
		FbxNodeAttribute* pNodeAttrib = pNode->GetNodeAttribute();
		if( pNodeAttrib != NULL )
		{
			switch( pNodeAttrib->GetAttributeType() )
			{
			case FbxNodeAttribute::eSkeleton:
				{
					ParseSkeletonJoint(pNode);
					break;
				}				
			case FbxNodeAttribute::eNull:
				{
					ParseNull(pNode);
					break;
				}
			}
		}
		ParseScene( pNode );
	}
}

void MFAnimation::Serialize()
{
	if( m_numDuration < 2 )
		return;

	std::fstream fs( GlobalConfig::GetSingleton()->m_strAnimationOutputPath + "/" + 
		GlobalConfig::GetSingleton()->m_strFbxFilename + ".anim",
		std::ios::out | std::ios::binary);

	int numSkeletonJoints = m_vecSkeletonJoints.size();
	fs.write( (char*)&m_numTotalFrames, sizeof(int));
	fs.write( (char*)&numSkeletonJoints, sizeof(int));

	FbxAMatrix matTransform;
	FbxAMatrix matScale;
	matScale.SetIdentity();
	matScale[2][2] = 1;

	for( int j = 0; j < m_numTotalFrames - 1; ++j )
	{
		FbxTime tm;
		tm.SetMilliSeconds( j * m_numFrameRate );
		fs.write( (char*)&m_numFrameRate, sizeof(int));
		for( int m = 0; m < numSkeletonJoints; ++m )
		{			
			matTransform = m_vecSkeletonJointNodes[m].second->EvaluateGlobalTransform( tm );
			matTransform = matScale * matTransform * matScale;

			FbxVector4 T = matTransform.GetT();
			FbxQuaternion Q = matTransform.GetQ();
			FbxVector4 S = matTransform.GetS();

			int nameSize = m_vecSkeletonJoints[m].first.length() + 1;
			fs.write( (char*)&nameSize, sizeof(int));
			fs.write( m_vecSkeletonJoints[m].first.c_str(), nameSize );

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)T[n];
				fs.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 4; ++n )
			{
				float val = (float)Q[n];
				fs.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)S[n];
				fs.write((char*)&val, sizeof(float));
			}
		}
	}

	int numFrameDiff = m_numDuration % m_numFrameRate;
	if( numFrameDiff == 0 )
		numFrameDiff = m_numFrameRate;
	if(  numFrameDiff > 0)
	{
		FbxTime tm;
		tm.SetMilliSeconds( m_numDuration );
		fs.write( (char*)&numFrameDiff, sizeof(int));
		for( int m = 0; m < numSkeletonJoints; ++m )
		{			
			matTransform = m_vecSkeletonJointNodes[m].second->EvaluateGlobalTransform( tm );
			matTransform = matScale * matTransform * matScale;

			FbxVector4 T = matTransform.GetT();
			FbxQuaternion Q = matTransform.GetQ();
			FbxVector4 S = matTransform.GetS();

			int nameSize = m_vecSkeletonJoints[m].first.length() + 1;
			fs.write( (char*)&nameSize, sizeof(int));
			fs.write( m_vecSkeletonJoints[m].first.c_str(), nameSize );

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)T[n];
				fs.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 4; ++n )
			{
				float val = (float)Q[n];
				fs.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)S[n];
				fs.write((char*)&val, sizeof(float));
			}
		}
	}
	fs.close();
}