#pragma once

#include "stdafx.h"

#include <Windows.h>
#include <sqlext.h>
#include <string>

class DBHandler
{
public:
	DBHandler();
	~DBHandler();

	bool ConnectToDB(const std::wstring& sourcename);	
	void ResetAllHost();

	bool SaveUserInfo(int host_id);
	bool RegisterIdAndPwd(char* id, char* pwd);
	int SearchIdAndPwd(const char* id, const char* pwd, int host_id);

private:
	bool PrintIfError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode);

private:
	SQLHENV m_hEnv;
	SQLHDBC m_hDbc;
	SQLHSTMT m_hStmt;
};