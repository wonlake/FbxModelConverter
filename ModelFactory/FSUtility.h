#pragma once
class FSUtility
{
public:
	FSUtility(void);
	~FSUtility(void);

public:
	bool Initialize(const char* pPath);

	const char* const GetFileName() const;

	const char* const GetFilePath() const;

	const char* const GetFileFullPath() const;

	const char* const GetFileExtension() const;

	static void MakeFolderExist(const std::string& strFolder );

private:
	std::string m_strFileName;
	std::string m_strFilePath;
	std::string m_strFileExtension;
	std::string m_strFileFullPath;
};

