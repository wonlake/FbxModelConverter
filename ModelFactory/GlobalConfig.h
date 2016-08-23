#pragma once

class GlobalConfig
{
public:
	GlobalConfig(void);
	~GlobalConfig(void);

	static GlobalConfig* GetSingleton();

	std::string m_strFbxFilename;

	std::string m_strAnimationOutputPath;
	std::string m_strTextureOutputPath;
	std::string m_strRootOutputPath;

	bool m_bExportAnimations;
	bool m_bNeedAttach;
	bool m_bNeedTweakFrames;

	unsigned int m_uiFrameRate;

	bool LoadConfig();
};

