#include "StdAfx.h"
#include "MFAnimation.h"
#include "GlobalConfig.h"
#include <nlohmann/json.hpp>

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
		std::string strTweakConfig = GlobalConfig::GetSingleton()->m_strRootOutputPath +
			"/tweakframes.json";
		std::ifstream f(strTweakConfig); 
	    auto doc = nlohmann::json::parse(f);

		for (auto iter : doc.items()) 
		{
			auto m = iter.value();
			auto name = m["src"];
			auto targetName = m["target"];

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
		} 
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

//#include <fbxsdk.h>
//
//// 假设您已经有一个已初始化并加载了FBX文件的 FbxScene 对象
//FbxScene* scene;
//
//// 获取场景中的第一个节点
//FbxNode* node = scene->GetRootNode()->GetChild(0); // 假设这里是要获取第一个子节点的关键帧数据
//
//// 获取节点的动画轨道
//FbxAnimLayer* anim_layer = scene->GetSrcObject<FbxAnimLayer>();
//FbxAnimCurveNode* anim_curve_node = node->LclRotation.GetCurveNode(anim_layer); // 获取旋转动画曲线节点
//
//// 检查动画轨道是否存在
//if (anim_curve_node) {
//	// 获取 X 轴的动画曲线
//	FbxAnimCurve* anim_curve_x = anim_curve_node->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_X);
//	if (anim_curve_x) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_x->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_x->KeyGetTime(i);
//			double key_value = anim_curve_x->KeyGetValue(i);
//			// 在这里使用旋转关键帧数据进行处理
//		}
//	}
//
//	// 获取 Y 轴的动画曲线
//	FbxAnimCurve* anim_curve_y = anim_curve_node->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_Y);
//	if (anim_curve_y) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_y->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_y->KeyGetTime(i);
//			double key_value = anim_curve_y->KeyGetValue(i);
//			// 在这里使用旋转关键帧数据进行处理
//		}
//	}
//
//	// 获取 Z 轴的动画曲线
//	FbxAnimCurve* anim_curve_z = anim_curve_node->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_Z);
//	if (anim_curve_z) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_z->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_z->KeyGetTime(i);
//			double key_value = anim_curve_z->KeyGetValue(i);
//			// 在这里使用旋转关键帧数据进行处理
//		}
//	}
//}
//
//// 获取节点的缩放动画轨道
//FbxAnimCurveNode* anim_curve_node_scale = node->LclScaling.GetCurveNode(anim_layer);
//
//// 检查缩放动画轨道是否存在
//if (anim_curve_node_scale) {
//	// 获取 X 轴的动画曲线
//	FbxAnimCurve* anim_curve_x_scale = anim_curve_node_scale->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_X);
//	if (anim_curve_x_scale) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_x_scale->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_x_scale->KeyGetTime(i);
//			double key_value = anim_curve_x_scale->KeyGetValue(i);
//			// 在这里使用缩放关键帧数据进行处理
//		}
//	}
//
//	// 获取 Y 轴的动画曲线
//	FbxAnimCurve* anim_curve_y_scale = anim_curve_node_scale->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_Y);
//	if (anim_curve_y_scale) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_y_scale->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_y_scale->KeyGetTime(i);
//			double key_value = anim_curve_y_scale->KeyGetValue(i);
//			// 在这里使用缩放关键帧数据进行处理
//		}
//	}
//
//	// 获取 Z 轴的动画曲线
//	FbxAnimCurve* anim_curve_z_scale = anim_curve_node_scale->GetCurve(0, FBXSDK_CURVENODE_COMPONENT_Z);
//	if (anim_curve_z_scale) {
//		// 输出关键帧数据
//		for (int i = 0; i < anim_curve_z_scale->KeyGetCount(); i++) {
//			FbxTime key_time = anim_curve_z_scale->KeyGetTime(i);
//			double key_value = anim_curve_z_scale->KeyGetValue(i);
//			// 在这里使用缩放关键帧数据进行处理
//		}
//	}
//}
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