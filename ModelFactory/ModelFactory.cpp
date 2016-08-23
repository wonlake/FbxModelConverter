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
#pragma comment(lib, "arg_parser_d.lib")
#else
#pragma comment(lib, "libfbxsdk-md.lib")
#pragma comment(lib, "arg_parser.lib")
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

	Arg_parser::Option opt[] = {
		{'i', "input_file", Arg_parser::yes },
		{252, "frame_rate", Arg_parser::yes },
		{253, "export_animation", Arg_parser::no },
		{254, "attach", Arg_parser::no},
		{255, "tweak_frame", Arg_parser::no}
	};
	Arg_parser parser(argc, argv, opt);
	if (parser.error().size() > 0)
	{
		std::cout << parser.error() << std::endl;
		return -1;
	}

	std::string fbx_filename;
	
	for (int i = 0; i < parser.arguments(); ++i)
	{
		int code = parser.code(i);
		if (code == 'i')
		{
			fbx_filename = parser.argument(i);
		}
		else if (code == 252)
		{
			int frameRate = atoi(parser.argument(i).c_str());
			if (frameRate > 0)
			{
				std::cout << "FrameRate : " << frameRate << "\n";
				GlobalConfig::GetSingleton()->m_uiFrameRate = frameRate;
			}
		}
		else if (code == 253)
		{
			GlobalConfig::GetSingleton()->m_bExportAnimations = true;
		}
		else if (code == 254)
		{
			GlobalConfig::GetSingleton()->m_bNeedAttach = true;
		}
		else if (code == 255)
		{
			GlobalConfig::GetSingleton()->m_bNeedTweakFrames = true;
		}
	}
	if (fbx_filename == "")
	{
		std::cout << "please input the right fbx file!" << std::endl;
		return -1;
	}

	FbxManager* pFbxManager = FbxManager::Create();
	FbxIOSettings* pIOSettings = FbxIOSettings::Create(pFbxManager, IOSROOT);
	FbxScene* pScene = FbxScene::Create(pFbxManager, "TestScene");

	FbxImporter* pImporter = FbxImporter::Create(pFbxManager, "");
	pImporter->Initialize(fbx_filename.c_str(), -1, pIOSettings);
	pImporter->Import(pScene);

	//FbxAxisSystem::DirectX.ConvertScene(pScene);

	FbxNode* pRoot = pScene->GetRootNode();

	FSUtility f;
	f.Initialize(fbx_filename.c_str());

	GlobalConfig::GetSingleton()->m_strFbxFilename = f.GetFileName();
	if (GlobalConfig::GetSingleton()->LoadConfig() == false)
	{
		std::cout << "please check the config xml!" << std::endl;
		return -1;
	}
	
	if (GlobalConfig::GetSingleton()->m_bExportAnimations)
	{
		MFAnimation ea;
		ea.ParseScene(pRoot);
		ea.Serialize();
	}
	else
	{
		std::string msFileName = GlobalConfig::GetSingleton()->m_strRootOutputPath + "/" +
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


