#include "StdAfx.h"
#include "FSUtility.h"


FSUtility::FSUtility(void)
{
}


FSUtility::~FSUtility(void)
{
}

bool FSUtility::Initialize(const char* pPath)
{
	if( pPath == nullptr )
		return false;
	std::string strPath = pPath;
	if( strPath.length() < 1 )
		return false;

	int pos1 = strPath.find_last_of('.');
	int pos2 = strPath.find_last_of('\\');
	int pos3 = strPath.find_last_of('/');
	if( pos2 != std::string::npos || pos3 != std::string::npos )
	{
		if( pos2 > pos3 )
			pos3 = pos2;
		else
			pos2 = pos3;
	}

	if( pos2 != std::string::npos )
	{
		if( pos2 == strPath.length() - 1) // folder
		{
			m_strFileName = "";
			m_strFileExtension = "";
			m_strFilePath = strPath.substr(0, pos2);
			m_strFileFullPath = m_strFilePath;
		}
		else
		{
			if( pos1 == std::string::npos ) // folder
			{
				m_strFileName = "";
				m_strFileExtension = "";
				m_strFilePath = strPath.substr(0, pos2);
				m_strFileFullPath = m_strFilePath;
			}
			else
			{
				if( pos1 < pos2 ) // folder
				{
					m_strFileName = "";
					m_strFileExtension = "";
					m_strFilePath = strPath.substr(0, pos2);
					m_strFileFullPath = m_strFilePath;
				}
				else	// file
				{
					m_strFileName = strPath.substr(pos2 + 1, pos1 - pos2 - 1);
					m_strFileExtension = strPath.substr(pos1 + 1, strPath.length() - pos1 - 1);
					m_strFilePath = strPath.substr(0, pos2);
					m_strFileFullPath = m_strFilePath + "/" + m_strFileName + "." + m_strFileExtension;
				}
			}
		}
	}
	else // file
	{
		if( pos1 == std::string::npos )
		{
			m_strFileName = strPath;
			m_strFileExtension = "";
			m_strFilePath = ".";
			m_strFileFullPath = m_strFilePath + "/" + m_strFileName;
		}
		else
		{
			m_strFileName = strPath.substr(0, pos1);
			m_strFileExtension = strPath.substr(pos1 + 1, strPath.length() - pos1 - 1);
			m_strFilePath = ".";
			m_strFileFullPath = m_strFilePath + "/" + m_strFileName + "." + m_strFileExtension;
		}
	}
	return true;
}

const char* const FSUtility::GetFileName() const
{
	return m_strFileName.c_str();
}

const char* const FSUtility::GetFilePath() const
{
	return m_strFilePath.c_str();
}

const char* const FSUtility::GetFileFullPath() const
{
	return m_strFileFullPath.c_str();
}

const char* const FSUtility::GetFileExtension() const
{
	return m_strFileExtension.c_str();
}

void FSUtility::MakeFolderExist(const std::string& strFolder )
{
	std::string strTestFile = strFolder + "/testForDirectory.bin";
	std::fstream fsTest(strTestFile, std::ios::out);
	if( !fsTest.is_open()  )
	{
		std::string strCmd = "mkdir \"";
		strCmd += strFolder;
		strCmd += "\"";
		system( strCmd.c_str() );		
	}
	else
		fsTest.close();
}
