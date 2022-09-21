#include "StdAfx.h"
#include "GlobalConfig.h"

#include <tinyxml.h>
#include <windows.h>

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
	
	std::string strConfigPath = "ModelFactoryConfig.xml";

	std::fstream fsConfig(strConfigPath, std::ios::in);
	if( !fsConfig.is_open() )
	{
		return false;
	}
	fsConfig.close();

	TiXmlDocument doc;
	doc.LoadFile( strConfigPath.c_str() );	
	auto rootElement = doc.RootElement();
	auto pChild = rootElement->FirstChildElement();
	do 
	{
		if( pChild == NULL )
			return false;
		if (std::string(pChild->Value()) == "root" )
		{
			m_strRootOutputPath = pChild->GetText();
		}
		else if (std::string(pChild->Value()) == "animations")
		{
			m_strAnimationOutputPath = pChild->GetText();
		}
		else if (std::string(pChild->Value()) == "textures")
		{
			m_strTextureOutputPath = pChild->GetText();
		}
	} while ( pChild = pChild->NextSiblingElement());

	if (m_strRootOutputPath == "" || m_strAnimationOutputPath == "" ||
		m_strTextureOutputPath == "")
	{
		std::cout << "please set root, animations, textures node in the config xml!!!" << std::endl;
		return false;
	}
	return true;
}