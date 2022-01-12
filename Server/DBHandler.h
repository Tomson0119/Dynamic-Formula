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
	bool ConnectWithID(const std::string& player_id);
	bool DisconnectAndUpdate();

private:
	bool PrintIfError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode);

private:
	SQLHENV m_hEnv;
	SQLHDBC m_hDbc;
	SQLHSTMT m_hStmt;
};