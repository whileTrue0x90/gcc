/**
 * Windows API header module
 *
 * Translated from MinGW Windows headers
 *
 * License: $(LINK2 http://www.boost.org/LICENSE_1_0.txt, Boost License 1.0)
 * Source: $(DRUNTIMESRC src/core/sys/windows/_sqlucode.d)
 */
module core.sys.windows.sqlucode;
version (Windows):

version (ANSI) {} else version = Unicode;

private import core.sys.windows.sqlext;

enum SQL_WCHAR        = -8;
enum SQL_WVARCHAR     = -9;
enum SQL_WLONGVARCHAR = -10;
enum SQL_C_WCHAR      = SQL_WCHAR;

enum SQL_SQLSTATE_SIZEW = 10;
version (Unicode) {
enum SQL_C_TCHAR = SQL_C_WCHAR;
} else {
enum SQL_C_TCHAR = SQL_C_CHAR;
}

// Moved from sqlext
static if (ODBCVER <= 0x0300) {
enum SQL_UNICODE             = -95;
enum SQL_UNICODE_VARCHAR     = -96;
enum SQL_UNICODE_LONGVARCHAR = -97;
enum SQL_UNICODE_CHAR        = SQL_UNICODE;
} else {
enum SQL_UNICODE             = SQL_WCHAR;
enum SQL_UNICODE_VARCHAR     = SQL_WVARCHAR;
enum SQL_UNICODE_LONGVARCHAR = SQL_WLONGVARCHAR;
enum SQL_UNICODE_CHAR        = SQL_WCHAR;
}

extern (Windows) {
    SQLRETURN SQLBrowseConnectA(SQLHDBC, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLBrowseConnectW(SQLHDBC, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLColAttributeA(SQLHSTMT, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*, SQLPOINTER);
    SQLRETURN SQLColAttributeW(SQLHSTMT, SQLUSMALLINT, SQLUSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*, SQLPOINTER);
    SQLRETURN SQLColAttributesA(SQLHSTMT, SQLUSMALLINT, SQLUSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*, SQLLEN*);
    SQLRETURN SQLColAttributesW(SQLHSTMT, SQLUSMALLINT, SQLUSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*, SQLLEN*);
    SQLRETURN SQLColumnPrivilegesA( SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT , SQLCHAR*, SQLSMALLINT );
    SQLRETURN SQLColumnPrivilegesW( SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT );
    SQLRETURN SQLColumnsA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT , SQLCHAR*, SQLSMALLINT );
    SQLRETURN SQLColumnsW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT , SQLWCHAR*, SQLSMALLINT );
    SQLRETURN SQLConnectA(SQLHDBC, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLConnectW(SQLHDBC, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLDataSourcesA(SQLHENV, SQLUSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLDataSourcesW(SQLHENV, SQLUSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLDescribeColA(SQLHSTMT, SQLUSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLSMALLINT*, SQLULEN*, SQLSMALLINT*, SQLSMALLINT*);
    SQLRETURN SQLDescribeColW(SQLHSTMT, SQLUSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLSMALLINT*, SQLULEN*, SQLSMALLINT*, SQLSMALLINT*);
    SQLRETURN SQLDriverConnectA(SQLHDBC, SQLHWND, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLUSMALLINT);
    SQLRETURN SQLDriverConnectW(SQLHDBC, SQLHWND, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLUSMALLINT);
    SQLRETURN SQLDriversA(SQLHENV, SQLUSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLDriversW(SQLHENV, SQLUSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLErrorA(SQLHENV, SQLHDBC, SQLHSTMT, SQLCHAR*, SQLINTEGER*, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLErrorW(SQLHENV, SQLHDBC, SQLHSTMT, SQLWCHAR*, SQLINTEGER*, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLExecDirectA(SQLHSTMT, SQLCHAR*, SQLINTEGER);
    SQLRETURN SQLExecDirectW(SQLHSTMT, SQLWCHAR*, SQLINTEGER);
    SQLRETURN SQLForeignKeysA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLForeignKeysW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLGetConnectAttrA(SQLHDBC, SQLINTEGER, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
    SQLRETURN SQLGetConnectAttrW(SQLHDBC, SQLINTEGER, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
    SQLRETURN SQLGetConnectOptionA(SQLHDBC, SQLUSMALLINT, SQLPOINTER);
    SQLRETURN SQLGetConnectOptionW(SQLHDBC, SQLUSMALLINT, SQLPOINTER);
    SQLRETURN SQLGetCursorNameA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLGetCursorNameW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLGetInfoA(SQLHDBC, SQLUSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLGetInfoW(SQLHDBC, SQLUSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*);
    SQLRETURN SQLGetTypeInfoA(SQLHSTMT, SQLSMALLINT);
    SQLRETURN SQLGetTypeInfoW(SQLHSTMT, SQLSMALLINT);
    SQLRETURN SQLNativeSqlA(SQLHDBC, SQLCHAR*, SQLINTEGER, SQLCHAR*, SQLINTEGER, SQLINTEGER*);
    SQLRETURN SQLNativeSqlW(SQLHDBC, SQLWCHAR*, SQLINTEGER, SQLWCHAR*, SQLINTEGER, SQLINTEGER*);
    SQLRETURN SQLPrepareA(SQLHSTMT, SQLCHAR*, SQLINTEGER);
    SQLRETURN SQLPrepareW(SQLHSTMT, SQLWCHAR*, SQLINTEGER);
    SQLRETURN SQLPrimaryKeysA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT );
    SQLRETURN SQLPrimaryKeysW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLProcedureColumnsA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLProcedureColumnsW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLProceduresA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLProceduresW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLSetConnectAttrA(SQLHDBC, SQLINTEGER, SQLPOINTER, SQLINTEGER);
    SQLRETURN SQLSetConnectAttrW(SQLHDBC, SQLINTEGER, SQLPOINTER, SQLINTEGER);
    SQLRETURN SQLSetConnectOptionA(SQLHDBC, SQLUSMALLINT, SQLULEN);
    SQLRETURN SQLSetConnectOptionW(SQLHDBC, SQLUSMALLINT, SQLULEN);
    SQLRETURN SQLSetCursorNameA(SQLHSTMT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLSetCursorNameW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT);
    SQLRETURN SQLSpecialColumnsA(SQLHSTMT, SQLUSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT , SQLUSMALLINT, SQLUSMALLINT);
    SQLRETURN SQLSpecialColumnsW(SQLHSTMT, SQLUSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT , SQLUSMALLINT, SQLUSMALLINT);
    SQLRETURN SQLStatisticsA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT , SQLUSMALLINT, SQLUSMALLINT);
    SQLRETURN SQLStatisticsW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT , SQLUSMALLINT, SQLUSMALLINT);
    SQLRETURN SQLTablePrivilegesA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLTablePrivilegesW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT );
    SQLRETURN SQLTablesA(SQLHSTMT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLCHAR*, SQLSMALLINT);
    SQLRETURN SQLTablesW(SQLHSTMT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT);
    static if (ODBCVER >= 0x0300) {
        SQLRETURN SQLGetDescFieldA(SQLHDESC, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
        SQLRETURN SQLGetDescFieldW(SQLHDESC, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
        SQLRETURN SQLSetDescFieldA(SQLHDESC, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLINTEGER);
        SQLRETURN SQLSetDescFieldW(SQLHDESC, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLINTEGER);
        SQLRETURN SQLGetDescRecA(SQLHDESC, SQLSMALLINT, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLSMALLINT*, SQLSMALLINT*, SQLLEN*, SQLSMALLINT*, SQLSMALLINT*, SQLSMALLINT*);
        SQLRETURN SQLGetDescRecW(SQLHDESC, SQLSMALLINT, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*, SQLSMALLINT*, SQLSMALLINT*, SQLLEN*, SQLSMALLINT*, SQLSMALLINT*, SQLSMALLINT*);
        SQLRETURN SQLGetDiagFieldA(SQLSMALLINT, SQLHANDLE, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*);
        SQLRETURN SQLGetDiagFieldW(SQLSMALLINT, SQLHANDLE, SQLSMALLINT, SQLSMALLINT, SQLPOINTER, SQLSMALLINT, SQLSMALLINT*);
        SQLRETURN SQLGetDiagRecA(SQLSMALLINT, SQLHANDLE, SQLSMALLINT, SQLCHAR*, SQLINTEGER*, SQLCHAR*, SQLSMALLINT, SQLSMALLINT*);
        SQLRETURN SQLGetDiagRecW(SQLSMALLINT, SQLHANDLE, SQLSMALLINT, SQLWCHAR*, SQLINTEGER*, SQLWCHAR*, SQLSMALLINT, SQLSMALLINT*);
        SQLRETURN SQLGetStmtAttrA(SQLHSTMT, SQLINTEGER, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
        SQLRETURN SQLGetStmtAttrW(SQLHSTMT, SQLINTEGER, SQLPOINTER, SQLINTEGER, SQLINTEGER*);
        SQLRETURN SQLSetStmtAttrA(SQLHSTMT, SQLINTEGER, SQLPOINTER, SQLINTEGER);
        SQLRETURN SQLSetStmtAttrW(SQLHSTMT, SQLINTEGER, SQLPOINTER, SQLINTEGER);
    } // #endif /* (ODBCVER >= 0x0300) */
}

version (Unicode) {
    alias SQLBrowseConnectW SQLBrowseConnect;
    alias SQLColAttributeW SQLColAttribute;
    alias SQLColAttributesW SQLColAttributes;
    alias SQLColumnPrivilegesW SQLColumnPrivileges;
    alias SQLColumnsW SQLColumns;
    alias SQLConnectW SQLConnect;
    alias SQLDataSourcesW SQLDataSources;
    alias SQLDescribeColW SQLDescribeCol;
    alias SQLDriverConnectW SQLDriverConnect;
    alias SQLDriversW SQLDrivers;
    alias SQLErrorW SQLError;
    alias SQLExecDirectW SQLExecDirect;
    alias SQLForeignKeysW SQLForeignKeys;
    alias SQLGetConnectAttrW SQLGetConnectAttr;
    alias SQLGetConnectOptionW SQLGetConnectOption;
    alias SQLGetCursorNameW SQLGetCursorName;
    alias SQLGetDescFieldW SQLGetDescField;
    alias SQLGetDescRecW SQLGetDescRec;
    alias SQLGetDiagFieldW SQLGetDiagField;
    alias SQLGetDiagRecW SQLGetDiagRec;
    alias SQLGetInfoW SQLGetInfo;
    alias SQLGetStmtAttrW SQLGetStmtAttr;
    alias SQLGetTypeInfoW SQLGetTypeInfo;
    alias SQLNativeSqlW SQLNativeSql;
    alias SQLPrepareW SQLPrepare;
    alias SQLPrimaryKeysW SQLPrimaryKeys;
    alias SQLProcedureColumnsW SQLProcedureColumns;
    alias SQLProceduresW SQLProcedures;
    alias SQLSetConnectAttrW SQLSetConnectAttr;
    alias SQLSetConnectOptionW SQLSetConnectOption;
    alias SQLSetCursorNameW SQLSetCursorName;
    alias SQLSetDescFieldW SQLSetDescField;
    alias SQLSetStmtAttrW SQLSetStmtAttr;
    alias SQLSpecialColumnsW SQLSpecialColumns;
    alias SQLStatisticsW SQLStatistics;
    alias SQLTablePrivilegesW SQLTablePrivileges;
    alias SQLTablesW SQLTables;
}
