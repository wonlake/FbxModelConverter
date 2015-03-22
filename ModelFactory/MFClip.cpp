#include "StdAfx.h"
#include "MFClip.h"
#include <tinyxml/tinyxml.h>
#include "GlobalConfig.h"
#include "MFMesh.h"
#include "MFFormat.h"

#ifdef _DEBUG
#pragma comment(lib, "tinyxml_d.lib")
#else
#pragma comment(lib, "tinyxml.lib")
#endif

MFClip::MFClip(MFMesh* pMesh)
{
	m_pMesh = pMesh;
}


MFClip::~MFClip(void)
{
}

void MFClip::Serialize(std::fstream& fs)
{
	if( m_pMesh )
	{
		std::map<std::string, std::string> mapAnimations;

		{
			std::string strClipsConfig = GlobalConfig::GetSingleton()->m_strOutputDirectory +
				"/clipnames.xml";
			std::fstream fsClipsConfig(strClipsConfig, std::ios::in | std::ios::binary);
			if( !fsClipsConfig.is_open() )
			{
				return;
			}
			else
			{
				fsClipsConfig.close();

				TiXmlDocument doc;
				doc.LoadFile( strClipsConfig.c_str() );	
				auto rootElement = doc.RootElement();
				auto pChild = rootElement->FirstChildElement();
				std::string fbxname = GlobalConfig::GetSingleton()->m_strFbxFilename;
				do 
				{
					if( pChild == NULL )
						break;

					auto name = pChild->Attribute("name");
					std::string strPartName = GlobalConfig::GetSingleton()->m_strFbxFilename;
					std::transform( strPartName.begin(), strPartName.end(), strPartName.begin(), std::tolower);
					int pos1 = strPartName.rfind("_");
					if( pos1 != std::string::npos )
					{
						strPartName = strPartName.substr(0, pos1); 
					}
					std::string full_name = strPartName + "_" + name;
					std::transform( full_name.begin(), full_name.end(), full_name.begin(), std::tolower );
					std::string strAnimationFile = GlobalConfig::GetSingleton()->m_strAnimationsDirectory + "/" + full_name + ".anim";
					std::fstream fsAnimationFile( strAnimationFile, std::ios::in | std::ios::binary );
					if( fsAnimationFile.is_open() )
					{
						mapAnimations[full_name] = name;
						fsAnimationFile.close();
					}
				} while ( pChild = pChild->NextSiblingElement());
			}
		}		

		auto iter = mapAnimations.begin();
		auto iterEnd = mapAnimations.end();
		while( iter != iterEnd )
		{
			std::string strAnimationFileName = GlobalConfig::GetSingleton()->m_strAnimationsDirectory + "/" + iter->first + ".anim";
			std::fstream fsAnimationFile(strAnimationFileName, std::ios::in | std::ios::binary);

			std::vector<int> vecDurations;

			if( fsAnimationFile.is_open() )
			{
				int numFrames = 0;
				int numBones = 0;
				fsAnimationFile.read( (char*)&numFrames, sizeof(int));
				fsAnimationFile.read( (char*)&numBones, sizeof(int));

				std::map<std::string, std::vector<BonePose> > mapBoneAnimations;		
				vecDurations.resize(numFrames);

				for( int i = 0; i < numFrames; ++i )
				{
					fsAnimationFile.read( (char*)&vecDurations[i], sizeof(int) );
					for( int j = 0; j < numBones; ++j )
					{
						int nameSize = 0;
						fsAnimationFile.read( (char*)&nameSize, sizeof(int));
						char* szBoneName = new char[nameSize];
						fsAnimationFile.read( (char*)szBoneName, nameSize);
						BonePose bonePose;
						bonePose.m_strName = szBoneName;
						fsAnimationFile.read( (char*)bonePose.transform, 40 );

						if( mapBoneAnimations.find(bonePose.m_strName) == mapBoneAnimations.end() )
							mapBoneAnimations[bonePose.m_strName] = std::vector<BonePose>();
						mapBoneAnimations[bonePose.m_strName].push_back( bonePose );
						delete[] szBoneName;
					}
				}
				fsAnimationFile.close();

				numBones = m_pMesh->m_vecBonePoseInverse.size();
				std::vector<BonePose> vecBonePoses;
				vecBonePoses.resize( numBones );

				std::string strPartName = m_pMesh->m_strName;
				std::transform( strPartName.begin(), strPartName.end(), strPartName.begin(), std::tolower);
				int pos1 = strPartName.find("_");
				if( pos1 != std::string::npos )
				{
					int pos2 = strPartName.find("_", pos1 + 1);
					if( pos2 != std::string::npos )
						strPartName = strPartName.substr(0, pos2); 
				}

				std::string strClipNodeFileName = GlobalConfig::GetSingleton()->m_strOutputDirectory + "/" + strPartName + "_" + iter->second + ".clip";
				std::fstream fsClipNodeFile( strClipNodeFileName, std::ios::out | std::ios::binary );

				int nameLen = iter->second.size() + 1;
				fsClipNodeFile.write( (char*)&nameLen, sizeof(int) );
				fsClipNodeFile.write( (char*)iter->second.data(), nameLen );
				fsClipNodeFile.write( (char*)&numFrames, sizeof(int) );		

				for( int i = 0; i < numFrames; ++i )
				{			
					fsClipNodeFile.write( (char*)&numBones, sizeof(int) );
					fsClipNodeFile.write( (char*)&vecDurations[i], sizeof(int) );
					for( int j = 0; j < numBones; ++j )
					{
						std::string& boneName = std::get<0>(m_pMesh->m_vecBonePoseInverse[j]);
						auto iter = mapBoneAnimations.find( boneName );
						if( iter != mapBoneAnimations.end())
						{
							vecBonePoses[j] = iter->second[i];
							vecBonePoses[j].m_strName = boneName;
						}
						else
						{
							vecBonePoses[j].m_strName = boneName;
							vecBonePoses[j].transform[0] = 0.0;
							vecBonePoses[j].transform[1] = 0.0;
							vecBonePoses[j].transform[2] = 0.0;

							vecBonePoses[j].transform[3] = 0.0;
							vecBonePoses[j].transform[4] = 0.0;
							vecBonePoses[j].transform[5] = 0.0;
							vecBonePoses[j].transform[6] = 1.0;

							vecBonePoses[j].transform[7] = 1.0;
							vecBonePoses[j].transform[8] = 1.0;
							vecBonePoses[j].transform[9] = 1.0;
						}	
						nameLen = boneName.size() + 1;
						fsClipNodeFile.write( (char*)&nameLen, sizeof(int) );
						fsClipNodeFile.write( boneName.data(), nameLen );
						fsClipNodeFile.write( (char*)vecBonePoses[j].transform, 40 );
					}
				}

				fsClipNodeFile.close();
				unsigned int tag = MF_CLIP;		
				fs.write( (char*)&tag, sizeof(tag) );
				std::string strRefPath = strPartName + "_" + iter->second + ".clip";

				unsigned int nameSize = strRefPath.size() + 1;
				unsigned int dataSize = nameSize + 4;
				fs.write( (char*)&dataSize, 4 );
				fs.write( (char*)&nameSize, 4);
				fs.write( (char*)strRefPath.data(), nameSize );

				std::cout << iter->first << "\n";
			}
			++iter;
		}
	}
}