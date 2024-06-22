#include "StdAfx.h"
#include "GlobalConfig.h"

#include <windows.h>
#include <nlohmann/json.hpp>

GlobalConfig::GlobalConfig(void)
{
	m_bExportAnimations = false;
	m_uiFrameRate = 100;

	m_bNeedAttach = false;
	m_bNeedTweakFrames = false;
	m_bMergeMesh = false;
}


GlobalConfig::~GlobalConfig(void)
{
}

GlobalConfig* GlobalConfig::GetSingleton()
{
	static GlobalConfig gs;
	return &gs;
}

bool GlobalConfig::LoadConfig()
{
	char szPath[MAX_PATH + 1] = {0};
	GetModuleFileNameA( NULL, szPath, MAX_PATH );
	std::string strExePath = szPath;
	int pos = strExePath.rfind("\\");
	
	std::string strConfigPath = "ModelFactoryConfig.json";

	std::fstream fsConfig(strConfigPath, std::ios::in);
	if( !fsConfig.is_open() )
	{
		return false;
	} 
	auto doc = nlohmann::json::parse(fsConfig);

	if (doc.contains("root"))
		m_strRootOutputPath = doc["root"];
	if (doc.contains("animations"))
		m_strAnimationOutputPath = doc["animations"];
	if (doc.contains("textures"))
		m_strTextureOutputPath = doc["textures"];

	if (m_strRootOutputPath == "" || m_strAnimationOutputPath == "" ||
		m_strTextureOutputPath == "")
	{
		std::cout << "please set root, animations, textures node in the config json!!!" << std::endl;
		return false;
	}

	std::string strTweakConfig = GlobalConfig::GetSingleton()->m_strRootOutputPath +
		"/tweakframes.json";

	return true;
}