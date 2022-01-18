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
	if (m_hEnv) SQLFreeHandle(SQL_HANDLE_ENV, m_hEnv);
	if (m_hDbc) SQLFreeHandle(SQL_HANDLE_DBC, m_hDbc);
	if (m_hStmt) SQLFreeHandle(SQL_HANDLE_STMT, m_hStmt);
}

bool DBHandler::ConnectToDB(const std::wstring& sourcename)
{
	RETCODE ret = SQLAllocHandle(SQL_HANDLE_ENV, SQL_NULL_HANDLE, &m_hEnv);
	if (PrintIfError(m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLSetEnvAttr(m_hEnv, SQL_ATTR_ODBC_VERSION, (SQLPOINTER*)SQL_OV_ODBC3, 0);
	if (PrintIfError(m_hEnv, SQL_HANDLE_ENV, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_DBC, m_hEnv, &m_hDbc);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLSetConnectAttr(m_hDbc, SQL_LOGIN_TIMEOUT, (SQLPOINTER)5, 0);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLConnect(m_hDbc, (SQLWCHAR*)sourcename.c_str(), SQL_NTS, (SQLWCHAR*)NULL, 0, NULL, 0);
	if (PrintIfError(m_hDbc, SQL_HANDLE_DBC, ret)) return false;

	ret = SQLAllocHandle(SQL_HANDLE_STMT, m_hDbc, &m_hStmt);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	return true;
}

bool DBHandler::SearchIdAndPwd(const std::string& id, const std::string& pwd)
{
	std::wstring query = L"EXEC search_id_pwd ";
	query.insert(query.end(), id.begin(), id.end());
	query += L", " + std::wstring(pwd.begin(), pwd.end());

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str() , SQL_NTS);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	SQLINTEGER retCount{};
	SQLLEN cb{};

	SQLBindCol(m_hStmt, 1, SQL_C_LONG, &retCount, 4, &cb);
	
	ret = SQLFetch(m_hStmt);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) {
		SQLCloseCursor(m_hStmt);
		return false;
	}

	SQLCancel(m_hStmt);
	SQLCloseCursor(m_hStmt);

	return (retCount == 1);
}

bool DBHandler::DisconnectAndUpdate()
{
	/*std::wstring query = L"EXEC disconnect ";
	query.insert(query.end(), std::begin(info.name), std::begin(info.name) + strlen(info.name));
	query += L", " + std::to_wstring(info.level)
		+ L", " + std::to_wstring(info.x)
		+ L", " + std::to_wstring(info.y)
		+ L", " + std::to_wstring(info.hp)
		+ L", " + std::to_wstring(info.max_hp)
		+ L", " + std::to_wstring(info.exp);

	RETCODE ret = SQLExecDirect(m_hStmt, (SQLWCHAR*)query.c_str(), SQL_NTS);
	if (PrintIfError(m_hStmt, SQL_HANDLE_STMT, ret)) return false;

	if (ret == SQL_SUCCESS || ret == SQL_SUCCESS_WITH_INFO) {
		SQLCancel(m_hStmt);
	}

	SQLCloseCursor(m_hStmt);*/
	return true;
}

bool DBHandler::PrintIfError(SQLHANDLE handle, SQLSMALLINT type, RETCODE retCode)
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
			std::wcout << "[" << state << "] " << errorMsg << " (" << error << ")\n";
		}
	}
	return true;
}
