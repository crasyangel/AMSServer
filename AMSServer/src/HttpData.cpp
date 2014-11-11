#include "stdafx.h"

#include "HttpData.h"

#define RECV_BUFFER_LENGTH 1024

#define PROJECTID_LENGTH 8
#define VENDORID_LENGTH 8


LPSTR HttpData::ErrorContext[] =
{
	NULL,
	"NoField",
	"QueryLenIsWrong",
	"QueryNumberUndefine",
	"NoProjectID",
	"ProjectIDIsWrong",
	"NoVendorID",
	"VendorIDIsWrong",
	"ProjectIDTooLong",
	"VendorIDTooLong",
	"STBIDTooLong",
	"ChipIDTooLong",
	"MACLenIsWrong",
	NULL
};

HttpData::HttpData(LPSTR comedata):
	sendflag(kNullFlag),
	errorcode(kNullError)
{
	recvdatabuf = new CHAR[RECV_BUFFER_LENGTH];
	memset(recvdatabuf, 0, RECV_BUFFER_LENGTH);

	payloadbuf = new CHAR[PAYLOAD_BUFFER_LENGTH];
	memset(payloadbuf, 0, PAYLOAD_BUFFER_LENGTH);

	projectid = new CHAR[PROJECTID_LENGTH+1];
	memset(projectid, 0, PROJECTID_LENGTH+1);

	vendorid = new CHAR[VENDORID_LENGTH+1];
	memset(vendorid, 0, VENDORID_LENGTH+1);

	stbid = new CHAR[STBID_LENGTH+1];
	memset(stbid, 0, STBID_LENGTH+1);

	chipid = new CHAR[CHIPID_LENGTH+1];
	memset(chipid, 0, CHIPID_LENGTH+1);

	macaddr	= new CHAR[MAC_LENGTH+1];
	memset(macaddr, 0, MAC_LENGTH+1);

	strncpy_s(recvdatabuf, RECV_BUFFER_LENGTH, comedata, strlen(comedata));
	*(recvdatabuf +RECV_BUFFER_LENGTH -1) = 0;
}

HttpData::~HttpData(void)
{
	if(NULL != recvdatabuf)
	{
		delete [] recvdatabuf;
	}

	if(NULL != payloadbuf)
	{
		delete [] payloadbuf;
	}

	if(NULL != projectid)
	{
		delete [] projectid;
	}

	if(NULL != vendorid)
	{
		delete [] vendorid;
	}

	if(NULL != stbid)
	{
		delete [] stbid;
	}

	if(NULL != chipid)
	{
		delete [] chipid;
	}

	if(NULL != macaddr)
	{
		delete [] macaddr;
	}
}

BOOL HttpData::HandleHttpData()
{
	ParseRecvBuf();
	if(FALSE == BuildPayLoad())
	{
		AMS_DBUG("failed in BuildPayLoad\n");
		return FALSE;
	}

	return TRUE;
}

//see my head file
LPSTR HttpData::GetPayLoadBuf() const
{
	return payloadbuf;
}

//the protocol beyond http MUST be agreed with both side
void HttpData::ParseRecvBuf()
{
	if(0 == _strnicmp(recvdatabuf, "Query=", strlen("Query=")))
	{
		AMS_DBUG("get \"Query=\" in \"GET\" context\n");
		ParseQuery();
	}
	else if(0 == _strnicmp(recvdatabuf, "Activate=", strlen("Activate=")))
	{
		AMS_DBUG("get \"Activate=\" in \"GET\" context\n");
		ParseActivate();
	}
	else
	{
		AMS_DBUG("the client did not send the \"Query=\" or \"Activate=\" string"
			", I don't know what the client want\n");
		sendflag = kErrorFlag;
		errorcode = kNoField;
		return;
	}
}

void HttpData::ParseActivate()
{

}

//see my head file
void HttpData::ParseQuery()
{
	char querytype[RECV_BUFFER_LENGTH]={0};
	sscanf_s(recvdatabuf, "%*[^=]=%[^&]" ,querytype);
	AMS_DBUG("Query=%s\n", querytype);

	if(1 != strlen(querytype))
	{
		sendflag = kErrorFlag;
		errorcode = kQueryLenIsWrong;
		AMS_DBUG("sendflag=0x%x\n", sendflag);
		return;
	}
	
	if(0 == strcmp(querytype, "0"))
	{
		AMS_DBUG("querytype=0\n");
		ParseQueryAuth();
		return;
	}
	
	if(0 == strcmp(querytype, "1"))
	{
		AMS_DBUG("querytype=1\n");
		ParseQueryActivate();
		return;
	}
	
	AMS_DBUG("querytype is undefined\n");
	sendflag = kErrorFlag;
	errorcode = kQueryNumberUndefine;
	return;
}

void HttpData::ParseQueryActivate()
{

}

//see my head file
void HttpData::ParseQueryAuth()
{
	//here the reason to use tmp buf 
	//is it will call memory crash when the len is too long
	//maybe client has missed some "&" at the end of one item
	CHAR tmp[RECV_BUFFER_LENGTH] = {0};

	ParseItem(tmp, "ProjectID=");
	*(tmp + RECV_BUFFER_LENGTH -1) = 0;
	//the ProjectID should have length less than 8
	if(8 < strlen(tmp))
	{
		AMS_DBUG("ProjectID len is longer than 8\n");
		sendflag = kErrorFlag;
		errorcode = kProjectIDTooLong;
		return;
	}
	else
	{
		strncpy_s(projectid, PROJECTID_LENGTH+1, tmp, strlen(tmp));
		AMS_DBUG("ProjectID=%s\n", projectid);

		//no value for ProjectID
		if(0 == strlen(projectid))
		{
			AMS_DBUG("No ProjectID context\n");
			sendflag = kErrorFlag;
			errorcode = kNoProjectID;
			return;
		}
	}

	memset(tmp, 0, RECV_BUFFER_LENGTH);
	ParseItem(tmp, "VendorID=");
	*(tmp + RECV_BUFFER_LENGTH -1) = 0;
	//the VendorID should have length less than 8
	if(8 < strlen(tmp))
	{
		AMS_DBUG("VendorID len is longer than 8\n");
		sendflag = kErrorFlag;
		errorcode = kVendorIDTooLong;
		return;
	}
	else
	{
		strncpy_s(vendorid, VENDORID_LENGTH+1, tmp, strlen(tmp));
		AMS_DBUG("VendorID=%s\n", vendorid);

		//no value for VendorID
		if(0 == strlen(vendorid))
		{
			AMS_DBUG("No VendorID context\n");
			sendflag = kErrorFlag;
			errorcode = kNoVendorID;
			return;
		}
	}

	memset(tmp, 0, RECV_BUFFER_LENGTH);
	ParseItem(tmp, "STBID=");
	*(tmp + RECV_BUFFER_LENGTH -1) = 0;
	//the STBID should have length less than 32
	if(32 < strlen(tmp))
	{
		AMS_DBUG("STBID len is longer than 32\n");
		sendflag = kErrorFlag;
		errorcode = kSTBIDTooLong;
		return;
	}
	else
	{
		strncpy_s(stbid, STBID_LENGTH+1, tmp, strlen(tmp));
		AMS_DBUG("STBID=%s\n", stbid);
	}

	memset(tmp, 0, RECV_BUFFER_LENGTH);
	ParseItem(tmp, "ChipID=");
	*(tmp + RECV_BUFFER_LENGTH -1) = 0;
	//the ChipID should have length less than 32
	if(32 < strlen(tmp))
	{
		AMS_DBUG("ChipID len is longer than 32\n");
		sendflag = kErrorFlag;
		errorcode = kChipIDTooLong;
		return;
	}
	else
	{
		strncpy_s(chipid, CHIPID_LENGTH+1, tmp, strlen(tmp));
		AMS_DBUG("ChipID=%s\n", chipid);
	}

	memset(tmp, 0, RECV_BUFFER_LENGTH);
	ParseItem(tmp, "MAC=");
	*(tmp + RECV_BUFFER_LENGTH -1) = 0;
	//the MAC should have length less than 12
	if(12 != strlen(tmp) && 0 != strlen(tmp))
	{
		AMS_DBUG("MAC len is not 12\n");
		sendflag = kErrorFlag;
		errorcode = kMACLenIsWrong;
		return;
	}
	else
	{
		strncpy_s(macaddr, MAC_LENGTH+1, tmp, strlen(tmp));
		AMS_DBUG("MAC=%s\n", macaddr);
	}

	//no value for all three item, we think you want to know the total sum
	if(0 == strlen(stbid) && 0 == strlen(chipid) && 0 == strlen(macaddr))
	{
		AMS_DBUG("client want the Auth sum\n");
		sendflag = kQueryAuthSum;
	}
	else 
	{
		sendflag = kQueryAuthState;
	}
}

//see my head file
void HttpData::ParseItem(LPSTR itemvalue, const LPSTR location)
{
	LPSTR bufpointer = NULL;

	bufpointer = strstr(recvdatabuf, location);
	if(NULL == bufpointer)
	{
		*itemvalue = 0;
	}
	else
	{
		sscanf_s(bufpointer, "%*[^=]=%[^&]" ,itemvalue);
		AMS_DBUG("%s%s\n", location, itemvalue);
	}
}

//see my head file
BOOL HttpData::BuildErrorPayLoad()
{
	if(kNullError == errorcode)
	{
		AMS_DBUG("Wrong Error value after parse recv buf\n");
		return FALSE;
	}

	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "ErrorCode=");
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, ErrorContext[errorcode]);

	return TRUE;
}

//see my head file
BOOL HttpData::BuildPayLoad()
{
	switch(sendflag)
	{
		case kErrorFlag:
		{
			if(FALSE == BuildErrorPayLoad())
			{
				AMS_DBUG("failed in BuildErrorPayLoad\n");
				return FALSE;
			}
			break;
		}
		
		case kQueryAuthState:
		{
			if(FALSE == BuildQueryAuthStatePayLoad())
			{
				AMS_DBUG("failed in BuildQueryAuthStatePayLoad\n");
				return FALSE;
			}
			break;
		}
		
		case kQueryAuthSum:
		{

			if(FALSE == BuildQueryAuthSumPayLoad())
			{
				AMS_DBUG("failed in BuildQueryAuthSumPayLoad\n");
				return FALSE;
			}
			break;
		}

		case kQueryActivateState:
		{
			if(FALSE == BuildQueryActivateStatePayLoad())
			{
				AMS_DBUG("failed in BuildQueryActivatePayLoad\n");
				return FALSE;
			}
			break;
		}

		case kQueryActivateSum:
		{
			if(FALSE == BuildQueryActivateSumPayLoad())
			{
				AMS_DBUG("failed in BuildQueryActivateSumPayLoad\n");
				return FALSE;
			}
			break;
		}

		case kActivateSucceed:
		{

			break;
		}

		case kActivateFailed:
		{

			break;
		}

		case kNullFlag:
		{
			AMS_DBUG("Wrong SendFlag value after parse recv buf\n");
			return FALSE;
			break;
		}
	}

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryActivateSumPayLoad()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "ActivateSum=0");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryActivateStatePayLoad()
{
	if(0 != strlen(stbid))
	{
		AMS_DBUG("client has sent the STBID\n");
		if(FALSE == BuildQueryActivatePayLoadByStbId())
		{
			AMS_DBUG("failed in BuildQueryActivatePayLoadByStbId\n");
			return FALSE;
		}
	}

	if(0 != strlen(chipid))
	{
		AMS_DBUG("client has sent the ChipID\n");
		if(FALSE == BuildQueryActivatePayLoadByChipId())
		{
			AMS_DBUG("failed in BuildQueryActivatePayLoadByChipId\n");
			return FALSE;
		}
	}

	if(0 != strlen(macaddr))
	{
		AMS_DBUG("client has sent the MAC\n");
		if(FALSE == BuildQueryActivatePayLoadByMac())
		{
			AMS_DBUG("failed in BuildQueryActivatePayLoadByMac\n");
			return FALSE;
		}
	}

	AMS_DBUG("BuildQueryActivateStatePayLoad complete\n");
	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryActivatePayLoadByStbId()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "ActivateState=StbId");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryActivatePayLoadByChipId()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "ActivateState=ChipId");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryActivatePayLoadByMac()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "ActivateState=Mac");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryAuthSumPayLoad()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthSum=0");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryAuthStatePayLoad()
{
	if(0 != strlen(stbid))
	{
		AMS_DBUG("client has sent the STBID\n");
		if(FALSE == BuildQueryAuthPayLoadByStbId())
		{
			AMS_DBUG("failed in BuildQueryAuthPayLoadByStbId\n");
			return FALSE;
		}
		return TRUE;
	}

	if(0 != strlen(chipid))
	{
		AMS_DBUG("client has sent the ChipID\n");
		if(FALSE == BuildQueryAuthPayLoadByChipId())
		{
			AMS_DBUG("failed in BuildQueryAuthPayLoadByChipId\n");
			return FALSE;
		}
		return TRUE;
	}

	if(0 != strlen(macaddr))
	{
		AMS_DBUG("client has sent the MAC\n");
		if(FALSE == BuildQueryAuthPayLoadByMac())
		{
			AMS_DBUG("failed in BuildQueryAuthPayLoadByMac\n");
			return FALSE;
		}
		return TRUE;
	}

	AMS_DBUG("client send no STBID, ChipID and MAC, this line shouldn't show up\n");
	return FALSE;
}

//see my head file
BOOL HttpData::BuildQueryAuthPayLoadByStbId()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthState=StbId");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryAuthPayLoadByChipId()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthState=ChipId");

	return TRUE;
}

//see my head file
BOOL HttpData::BuildQueryAuthPayLoadByMac()
{
	strcat_s(payloadbuf, PAYLOAD_BUFFER_LENGTH, "AuthState=Mac");

	return TRUE;
}