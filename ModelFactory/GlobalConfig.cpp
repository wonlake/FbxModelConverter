#include "StdAfx.h"
#include "GlobalConfig.h"

#include <tinyxml/tinyxml.h>
#include <windows.h>

GlobalConfig::GlobalConfig(void)
{
	m_strAnimationsDirectory = "Animations";
	m_strTexturesDirectory = "Textures";

	m_bExportAnimations = false;
	m_uiFrameRate = 100;

	m_bNeedAttach = false;
	m_bNeedTweakFrames = false;
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
	if( pos != std::string::npos )
		strConfigPath = strExePath.substr(0, pos + 1) + strConfigPath;

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
		m_strOutputDirectory = pChild->GetText();
		break;

	} while ( pChild = pChild->NextSiblingElement());

	return true;
}