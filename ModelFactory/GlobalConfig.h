#pragma once

class GlobalConfig
{
public:
	GlobalConfig(void);
	~GlobalConfig(void);

	static GlobalConfig* GetSingleton();

	std::string m_strFbxFilename;
	std::string m_strOutputDirectory;
	std::string m_strOutputDirectoryShort;
	std::string m_strAnimationsDirectory;
	std::string m_strTexturesDirectory;

	bool m_bExportAnimations;
	bool m_bNeedAttach;
	bool m_bNeedTweakFrames;

	unsigned int m_uiFrameRate;

	bool LoadConfig();
};

