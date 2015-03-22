// ModelFactory.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"

#include "MFAnimation.h"
#include "FSUtility.h"
#include "MFFormat.h"
#include "MFMesh.h"
#include "GlobalConfig.h"

#ifdef _DEBUG
#pragma comment(lib, "libfbxsdk-mdd.lib")
#else
#pragma comment(lib, "libfbxsdk-md.lib")
#endif


std::vector<MFMesh*> g_vecMeshes;

void ParseScene(FbxNode* pRootNode, std::fstream& fs)
{
	int numChildren = pRootNode->GetChildCount();
	for (int i = 0; i < numChildren; ++i)
	{
		FbxNode* pNode = pRootNode->GetChild(i);
		FbxNodeAttribute* pNodeAttrib = pNode->GetNodeAttribute();
		if (pNodeAttrib != NULL)
		{
			switch (pNodeAttrib->GetAttributeType())
			{
			case FbxNodeAttribute::eMesh:
			{
											MFMesh* pMesh = new MFMesh();
											pMesh->Parse(pNode);
											pMesh->Serialize(fs);
			}
				break;
			default:
				std::cout << "unknown node attribute!\n";
				break;
			}
		}
		ParseScene(pNode, fs);
	}
}

int main(int argc, char** argv)
{
	if (argc < 2)
		return -1;

	std::set<std::string> sCmds;
	for (int i = 0; i < argc; ++i)
	{
		std::string strCmd = argv[i];
		sCmds.insert(strCmd);
		int pos = strCmd.find("FrameRate_");
		if (pos != std::string::npos)
		{
			pos = strCmd.find("_");
			std::string strFrameRate = strCmd.substr(pos + 1, strCmd.length() - pos - 1);
			int frameRate = atoi(strFrameRate.c_str());
			if (frameRate > 0)
			{
				std::cout << "FrameRate : " << frameRate << "\n";
				GlobalConfig::GetSingleton()->m_uiFrameRate = frameRate;
			}
		}
	}

	if (sCmds.find("ExportAnimations") != sCmds.end())
	{
		GlobalConfig::GetSingleton()->m_bExportAnimations = true;
	}

	if (sCmds.find("NeedAttach") != sCmds.end())
	{
		GlobalConfig::GetSingleton()->m_bNeedAttach = true;
	}

	if (sCmds.find("NeedTweakFrames") != sCmds.end())
	{
		GlobalConfig::GetSingleton()->m_bNeedTweakFrames = true;
	}

	FbxManager* pFbxManager = FbxManager::Create();
	FbxIOSettings* pIOSettings = FbxIOSettings::Create(pFbxManager, IOSROOT);
	FbxScene* pScene = FbxScene::Create(pFbxManager, "TestScene");

	FbxImporter* pImporter = FbxImporter::Create(pFbxManager, "");
	pImporter->Initialize(argv[1], -1, pIOSettings);
	pImporter->Import(pScene);

	FbxNode* pRoot = pScene->GetRootNode();

	FSUtility f;
	f.Initialize(argv[1]);

	GlobalConfig::GetSingleton()->LoadConfig();
	GlobalConfig::GetSingleton()->m_strFbxFilename = f.GetFileName();
	if (GlobalConfig::GetSingleton()->m_strOutputDirectory.empty())
		GlobalConfig::GetSingleton()->m_strOutputDirectory =
		std::string("") + f.GetFilePath() + "/" + f.GetFileName();
	GlobalConfig::GetSingleton()->m_strAnimationsDirectory =
		GlobalConfig::GetSingleton()->m_strOutputDirectory + "/Animations";
	GlobalConfig::GetSingleton()->m_strTexturesDirectory =
		GlobalConfig::GetSingleton()->m_strOutputDirectory + "/Textures";

	FSUtility::MakeFolderExist(GlobalConfig::GetSingleton()->m_strOutputDirectory);
	FSUtility::MakeFolderExist(GlobalConfig::GetSingleton()->m_strTexturesDirectory);
	FSUtility::MakeFolderExist(GlobalConfig::GetSingleton()->m_strAnimationsDirectory);

	if (GlobalConfig::GetSingleton()->m_bExportAnimations)
	{
		MFAnimation ea;
		ea.ParseScene(pRoot);
		ea.Serialize();
	}
	else
	{
		std::string msFileName = GlobalConfig::GetSingleton()->m_strOutputDirectory + "/" +
			GlobalConfig::GetSingleton()->m_strFbxFilename + ".ms";
		std::fstream fsMS(msFileName, std::ios::out | std::ios::binary);
		char szMagic[4] = { 'M', 'S', '3', 'D' };
		fsMS.write(szMagic, 4);
		unsigned int tag = MF_HEADER;
		fsMS.write((char*)&tag, 4);
		unsigned int dataSize = 4;
		fsMS.write((char*)&dataSize, 4);
		unsigned int version = 0x10000001;
		fsMS.write((char*)&version, 4);
		ParseScene(pRoot, fsMS);
		fsMS.close();
	}



	pImporter->Destroy();
	pScene->Destroy();
	pFbxManager->Destroy();

	return 0;
}


