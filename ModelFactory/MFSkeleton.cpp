#include "StdAfx.h"
#include "MFSkeleton.h"
#include "MFMesh.h"
#include "GlobalConfig.h"
#include "MFFormat.h"

MFSkeleton::MFSkeleton(MFMesh* pMesh)
{
	m_pMesh = pMesh;
}


MFSkeleton::~MFSkeleton(void)
{
}

void MFSkeleton::Serialize(std::fstream& fs)
{
	if( m_pMesh)
	{
		std::string strPartName = m_pMesh->m_strName;
		std::transform( strPartName.begin(), strPartName.end(), strPartName.begin(), std::tolower);
		int pos1 = strPartName.find("_");
		if( pos1 != std::string::npos )
		{
			int pos2 = strPartName.find("_", pos1 + 1);
			if( pos2 != std::string::npos )
				strPartName = strPartName.substr(0, pos2); 
		}
		std::string strSkeletonPartFile = GlobalConfig::GetSingleton()->m_strRootOutputPath + "/" + strPartName + ".skel";
		std::fstream fsSkeleton(strSkeletonPartFile, std::ios::out | std::ios::binary);
		int index = 0;
		int numBones = m_pMesh->m_vecBonePoseInverse.size();
		fsSkeleton.write( (char*)&numBones, sizeof(int) );
		std::for_each( m_pMesh->m_vecBonePoseInverse.begin(), 
			m_pMesh->m_vecBonePoseInverse.end(),
			[&](std::tuple<std::string, FbxAMatrix, FbxCluster*> bone)
		{
			FbxCluster* pCluster = std::get<2>(bone);
			FbxAMatrix matBonePoseInverse = std::get<1>(bone);

			if( pCluster )
			{
				FbxAMatrix matScale;
				matScale.SetIdentity();
				matScale[2][2] = 1;
				matBonePoseInverse = matScale * matBonePoseInverse;
			}

			std::string strBoneName = std::get<0>(bone);
			int nameSize = strBoneName.length() + 1;				
			float matFloats[16];
			for( int i = 0; i < 16; ++i )
			{
				matFloats[i] = (float)matBonePoseInverse[i / 4][i %4];
			}
			fsSkeleton.write( (char*)&nameSize, sizeof(int));
			fsSkeleton.write( strBoneName.c_str(), nameSize );
			fsSkeleton.write( (char*)matFloats, 64 );
			
			FbxAMatrix matBonePose = matBonePoseInverse.Inverse();
			FbxVector4 T = matBonePose.GetT();
			FbxQuaternion Q = matBonePose.GetQ();
			FbxVector4 S = matBonePose.GetS();

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)T[n];
				fsSkeleton.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 4; ++n )
			{
				float val = (float)Q[n];
				fsSkeleton.write((char*)&val, sizeof(float));
			}

			for( int n = 0; n < 3; ++n )
			{
				float val = (float)S[n];
				fsSkeleton.write((char*)&val, sizeof(float));
			}

			++index;
		}
		);
		fsSkeleton.close();

		unsigned int tag = MF_SKELETON;		
		fs.write( (char*)&tag, sizeof(tag) );
		std::string strRefPath = strPartName + ".skel";

		unsigned int nameSize = strRefPath.size() + 1;
		unsigned int dataSize = nameSize + 4;
		fs.write( (char*)&dataSize, 4 );
		fs.write( (char*)&nameSize, 4);
		fs.write( (char*)strRefPath.data(), nameSize );
	}
}
