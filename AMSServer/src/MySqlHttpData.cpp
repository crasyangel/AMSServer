#include "stdafx.h"
#include "MySqlHttpData.h"

#define ITEM_VALUE_LEN 1
#define QUERY_STRING_LEN 256
#define NAME_LENGTH 32
#define SUM_LENGTH 8

#define DEFAULT_DATABASE_NAME "information_schema"
#define DATABASE_NAME_HEADER "ProjectID_"
#define TABLE_NAME_HEADER "VendorID_"

//suppress the query details if this value is 0
#define ShowQueryLevel 1

MySqlHttpData::MySqlHttpData(LPSTR comedata):
	HttpData(comedata)
{
	mysql_init(&mysql);

	databasename = new CHAR[NAME_LENGTH];
	memset(databasename, 0, NAME_LENGTH);

	tablename = new CHAR[NAME_LENGTH];
	memset(tablename, 0, NAME_LENGTH);
}

MySqlHttpData::~MySqlHttpData()
{
	if(NULL != databasename)
	{
		delete [] databasename;
	}

	if(NULL != tablename)
	{
		delete [] tablename;
	}

	mysql_close(&mysql);
}

BOOL MySqlHttpData::HandleHttpData()
{
	ParseRecvBuf();

	if(FALSE == ConnectDatabase())
	{
		AMS_DBUG("failed in ConnectDatabase\n");
		return FALSE;
	}

	if(FALSE == BuildPayLoad())
	{
		AMS_DBUG("failed in BuildPayLoad\n");
		return FALSE;
	}

	return TRUE;
}

void MySqlHttpData::ShowQueryDetails(UCHAR level, MYSQL_RES *result)
{
	if(level < 1)
	{
		AMS_DBUG("suppress the query details!!\n");
		return;
	}

	UCHAR columnlen[20] = {0};
	MYSQL_ROW sqlrow = NULL;
	ULONG *lengths = NULL;
	MYSQL_FIELD *fd[20] = {0};
	UCHAR filednum = 0;
	
	printf("\n");
	if(sqlrow = mysql_fetch_row(result))
	{
		lengths = mysql_fetch_lengths(result);
		filednum = mysql_num_fields(result);
		for(UINT i=0; i<filednum; i++)
		{
			fd[i] = mysql_fetch_field(result);
			columnlen[i] = fd[i]->name_length > (UCHAR)lengths[i] ? fd[i]->name_length : (UCHAR)lengths[i];
		}

		for(UINT i=0; i<filednum; i++)
		{
			printf("+-");
			for(UINT j=0; j < columnlen[i]; ++j)
			{
				printf("-");
			}
			printf("-");
		}
		printf("+\n");

		for(UINT i=0; i<filednum; i++)
		{
			printf("| %s ", fd[i]->name);
			while(fd[i]->name_length < columnlen[i])
			{
				printf(" ");
				++fd[i]->name_length;
			}
		}
		printf("|\n");

		for(UINT i=0; i<filednum; i++)
		{
			printf("+-");
			for(UINT j=0; j < columnlen[i]; ++j)
			{
				printf("-");
			}
			printf("-");
		}
		printf("+\n");

		for(UINT i=0; i<filednum; i++)
		{
			printf("| %s ", sqlrow[i]);
			while(lengths[i] < columnlen[i])
			{
				printf(" ");
				++lengths[i];
			}
		}
		printf("|\n");
	}
	else
	{
		AMS_DBUG("no context in result\n");
		return;
	}

	while(sqlrow = mysql_fetch_row(result))
	{
		lengths = mysql_fetch_lengths(result);

		for(UINT i=0; i<filednum; i++)
		{
			printf("+-");
			for(UINT j=0; j < columnlen[i]; ++j)
			{
				printf("-");
			}
			printf("-");
		}
		printf("+\n");

		for(UINT i=0; i<filednum; i++)
		{
			printf("| %s ", sqlrow[i]);
			while(lengths[i] < columnlen[i])
			{
				printf(" ");
				++lengths[i];
			}
		}
		printf("|\n");
	}

	for(UINT i=0; i<filednum; i++)
	{
		printf("+-");
		for(UINT j=0; j < columnlen[i]; ++j)
		{
			printf("-");
		}
		printf("-");
	}
	printf("+\n");

	printf("\n");
}

BOOL MySqlHttpData::ConnectDatabase()
{
	if(NULL == mysql_real_connect(&mysql, "localhost", "root", "123", DEFAULT_DATABASE_NAME, MYSQL_PORT, NULL, 0)) 
	{
		AMS_DBUG("connect to database server failed!\n"); 
		return FALSE;
	}

	CHAR query[QUERY_STRING_LEN] = {0};
	strcat_s(query, QUERY_STRING_LEN, "SELECT schema_name FROM SCHEMATA WHERE schema_name=");

	strcat_s(databasename, QUERY_STRING_LEN, DATABASE_NAME_HEADER);
	strcat_s(databasename, QUERY_STRING_LEN, projectid);
	AMS_DBUG("databasename is %s!!\n", databasename);

	CHAR tempdatabasename[NAME_LENGTH+3] = {0};
	sprintf_s(tempdatabasename, NAME_LENGTH+3, "\"%s\"", databasename);
	strcat_s(query, QUERY_STRING_LEN, tempdatabasename);
	AMS_DBUG("query string: %s\n", query);

	int sqlerror = 0;
	sqlerror = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	if(0 != sqlerror)
	{
		AMS_DBUG("mysql_real_query fails,error: %d\n",sqlerror);
		return FALSE;
	}

	MYSQL_RES *result = NULL;
	result = mysql_store_result(&mysql);
	if(NULL == result)
	{
		AMS_DBUG("mysql_store_result fails\n");
		return FALSE;
	}

	if(0 == mysql_num_rows(result))
	{
		AMS_DBUG("projectid:%s is wrong!!\n", projectid);
		mysql_free_result(result);

		sendflag = kErrorFlag;
		errorcode = kProjectIDIsWrong;

		return TRUE;
	}

	ShowQueryDetails(ShowQueryLevel, result);
	mysql_free_result(result);

	memset(query, 0, QUERY_STRING_LEN);
	strcat_s(query, QUERY_STRING_LEN, "USE ");
	strcat_s(query, QUERY_STRING_LEN, tempdatabasename);
	//AMS_DBUG("query string: %s\n", query);

	sqlerror = mysql_select_db(&mysql, databasename);
	if(0 != sqlerror)
	{
		AMS_DBUG("mysql_select_db fails,error: %d\n", sqlerror);
		return FALSE;
	}

	memset(query, 0, QUERY_STRING_LEN);
	strcat_s(query, QUERY_STRING_LEN, "SELECT table_name FROM information_schema.TABLES WHERE table_name=");

	strcat_s(tablename, QUERY_STRING_LEN, TABLE_NAME_HEADER);
	strcat_s(tablename, QUERY_STRING_LEN, vendorid);
	AMS_DBUG("tablename is %s!!\n", tablename);

	CHAR temptablename[NAME_LENGTH+3] = {0};
	sprintf_s(temptablename, NAME_LENGTH+3, "\"%s\"", tablename);
	strcat_s(query, QUERY_STRING_LEN, temptablename);
	AMS_DBUG("query string: %s\n", query);

	sqlerror = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	if(0 != sqlerror)
	{
		AMS_DBUG("mysql_real_query fails,error: %d\n",sqlerror);
		return FALSE;
	}

	result = mysql_store_result(&mysql);
	if(NULL == result)
	{
		AMS_DBUG("mysql_store_result fails\n");
		return FALSE;
	}

	if(0 == mysql_num_rows(result))
	{
		AMS_DBUG("projectid:%s is wrong!!\n", vendorid);
		mysql_free_result(result);

		sendflag = kErrorFlag;
		errorcode = kVendorIDIsWrong;

		return TRUE;
	}

	ShowQueryDetails(ShowQueryLevel, result);
	mysql_free_result(result);

	return TRUE;
}


BOOL MySqlHttpData::BuildQueryAuthSumPayLoad()
{
	CHAR query[QUERY_STRING_LEN] = {0};
	strcat_s(query, QUERY_STRING_LEN, "SELECT * FROM ");	
	strcat_s(query, QUERY_STRING_LEN, tablename);
	AMS_DBUG("query string: %s\n", query);

	int sqlerror = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	if(0 != sqlerror)
	{
		AMS_DBUG("mysql_real_query fails,error: %d\n",sqlerror);
		return FALSE;
	}

	MYSQL_RES *result = mysql_store_result(&mysql);
	if(NULL == result)
	{
		AMS_DBUG("mysql_store_result fails\n");
		return FALSE;
	}

	ShowQueryDetails(ShowQueryLevel, result);
	int authsum = (int)mysql_num_rows(result);
	mysql_free_result(result);

	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthSum=");
	char tempauthsum[SUM_LENGTH] = {0};

	if(0 != _itoa_s(authsum, tempauthsum, 10))
	{
		AMS_DBUG("_itoa_s fails\n");
		return FALSE;
	}
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, tempauthsum);

	return TRUE;
}

BOOL MySqlHttpData::BuildQueryAuthPayLoadByStbId()
{
	CHAR query[QUERY_STRING_LEN] = {0};
	strcat_s(query, QUERY_STRING_LEN, "SELECT * FROM ");	
	strcat_s(query, QUERY_STRING_LEN, tablename);
	strcat_s(query, QUERY_STRING_LEN, " WHERE stbid=");

	CHAR tempstbid[STBID_LENGTH+3] = {0};
	sprintf_s(tempstbid, STBID_LENGTH+3, "\"%s\"", stbid);
	strcat_s(query, QUERY_STRING_LEN, tempstbid);
	AMS_DBUG("query string: %s\n", query);

	int sqlerror = mysql_real_query(&mysql, query, (unsigned int)strlen(query));
	if(0 != sqlerror)
	{
		AMS_DBUG("mysql_real_query fails,error: %d\n",sqlerror);
		return FALSE;
	}

	MYSQL_RES *result = mysql_store_result(&mysql);
	if(NULL == result)
	{
		AMS_DBUG("mysql_store_result fails\n");
		return FALSE;
	}

	ShowQueryDetails(ShowQueryLevel, result);
	int authstate = (int)mysql_num_rows(result);
	mysql_free_result(result);

	if(1 > authstate)
	{
		AMS_DBUG("the item with this special stbid is not unique!!\n");
		return FALSE;
	}

	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthState=");
	char tempauthstate[SUM_LENGTH] = {0};

	if(0 != _itoa_s(authstate, tempauthstate, 10))
	{
		AMS_DBUG("_itoa_s fails\n");
		return FALSE;
	}
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, tempauthstate);

	return TRUE;
}

