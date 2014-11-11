#pragma once

#include "stdafx.h"

#include "HttpData.h"
#include "mysql/mysql.h"

#pragma comment (lib, "libmysql.lib") 

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO: Should move mysql special method to MySqlHandle
//here only handle httpdata with mysql
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

//handle httpdata
class MySqlHttpData : public HttpData
{
public:
	MySqlHttpData(LPSTR comedata);
	virtual ~MySqlHttpData(void);

private:
	//mysql database handle;
	MYSQL mysql;
	LPSTR databasename;
	LPSTR tablename;

public:
	BOOL HandleHttpData(void);

private:
	void ShowQueryDetails(UCHAR level, MYSQL_RES *result);
	BOOL ConnectDatabase(void);

private:
	//let drived class to cover this four virtual method for active
	//and bulid real respond
	//BOOL BuildQueryActivateSumPayLoad(void);
	//BOOL BuildQueryActivatePayLoadByStbId(void);

private:
	//let drived class to cover this four virtual method for auth
	//and bulid real respond
	BOOL BuildQueryAuthSumPayLoad(void);
	BOOL BuildQueryAuthPayLoadByStbId(void);
};