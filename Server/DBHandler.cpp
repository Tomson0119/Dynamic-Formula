#include "common.h"
#include "DBHandler.h"
#include <iostream>

DBHandler::DBHandler()
	: m_hEnv{}, m_hDbc{}, m_hStmt{}
{
	std::wcout.imbue(std::locale("korean"));	
}

DBHandler::~DBHandler()
{
	if (m_hStmt) SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
	if (m_hDbc) SQLDisconnect(m_hDbc);
	if (m_hDbc) SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
	if (m_hEnv) SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
}

bool DBHandler::ConnectToDB(const std::wstring& sourcename)
{
	RETCODE ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (PrintIfError(L"SQLAllocHandle", m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (PrintIfError(L"SQLSetEnvAttr", m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
	if (PrintIfError(L"SQLAllocHandle", m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLSetConnectAttr(m_hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (PrintIfError(L"SQLSetConnectAttr", m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLConnect(m_hDbc, (SQLWCHAR*)sourcename.c_str(), SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	if (PrintIfError(L"SQLConnect", m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &m_hStmt);
	if (PrintIfError(L"SQLAllocHandle", m_hStmt, SQL_HANDLE_STMT, ret)) return false;
	
	return true;
}

void DBHandler::ResetAllHost()
{
	std::wstring query = L"EXEC reset_all_user";

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(L"SQLExecDirect", m_hStmt, SQL_HANDLE_STMT, ret)) return;

	SQLCloseCursor(m_hStmt);
}

bool DBHandler::SaveUserInfo(int host_id)
{
	std::wstring query = L"EXEC save_user_info " + std::to_wstring(host_id);

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(L"SQLExecDirect", m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	if (m_hStmt) SQLCloseCursor(m_hStmt);

	return true;
}

bool DBHandler::RegisterIdAndPwd(char* id, char* pwd)
{
	std::wstring query = L"EXEC register_new_user " + CharToWString(id) + L", " + CharToWString(pwd);

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(L"SQLExecDirect", m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	SQLSMALLINT retValue{};
	SQLLEN cb{};

	ret = SQLBindCol(m_hStmt, 1, SQL_C_SHORT, &retValue, 2, &cb);
	if (PrintIfError(L"SQLBindCol", m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	ret = SQLFetch(m_hStmt);
	if (PrintIfError(L"SQLFetch", m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	if (m_hStmt) SQLCloseCursor(m_hStmt);

	return (retValue == 1);
}

int DBHandler::SearchIdAndPwd(const char* id, const char* pwd, int host_id)
{
	std::wstring query = L"EXEC search_id_pwd " + CharToWString(id) 
		+ L", " + CharToWString(pwd) + L", " + std::to_wstring(host_id);

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(L"SQLExecDirect", m_hStmt, SQL_HANDLE_STMT, ret)) return -2;

	SQLINTEGER retConnID{};
	SQLLEN cb{};

	ret = SQLBindCol(m_hStmt, 1, SQL_C_LONG, &retConnID, 4, &cb);
	if (PrintIfError(L"SQLBindCol", m_hStmt, SQL_HANDLE_STMT, ret)) return -2;

	ret = SQLFetch(m_hStmt);
	if (PrintIfError(L"SQLFetch", m_hStmt, SQL_HANDLE_STMT, ret)) return -2;

	if (m_hStmt) SQLCloseCursor(m_hStmt);
	
	return (int)(retConnID);
}

bool DBHandler::PrintIfError(const std::wstring& funcName, SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode)
{
	if(retCode != SQL_ERROR) {
		if (retCode == SQL_INVALID_HANDLE) {
			std::cerr << "Invalid Handle\n";
			return true;
		}
		return false;
	}

	SQLSMALLINT record = 0;
	SQLINTEGER error = 0;
	WCHAR errorMsg[1000];
	WCHAR state[SQL_SQLSTATE_SIZE + 1]{};

	while (SQLGetDiagRec(type, handle, ++record, state, &error, errorMsg,
		(SQLSMALLINT)(sizeof(errorMsg) / sizeof(WCHAR)), (SQLSMALLINT*)NULL) == SQL_SUCCESS)
	{
		if (wcsncmp(errorMsg, L"01004", 5))
		{
			std::wcout << "(" << funcName<<" " << state << ") " << errorMsg << " (" << error << ")\n";
		}
	}
	return true;
}