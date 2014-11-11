#pragma once

#include "stdafx.h"

#define PAYLOAD_BUFFER_LENGTH 1024
#define STBID_LENGTH 32
#define CHIPID_LENGTH 32
#define MAC_LENGTH 12


//handle httpdata
class HttpData
{
public:
	HttpData(LPSTR comedata);
	virtual ~HttpData(void);

protected:
	//cannot use member signal in MutiThread env
	//MUST use mutex but it has low efficiency
	//Better to transfer it in func parameter
	//so we new this class every time when we want use
	//SendFlag sendflag;
	typedef enum
	{
		kErrorFlag =0,
		kQueryAuthState,
		kQueryAuthSum,
		kQueryActivateState,
		kQueryActivateSum,
		kActivateSucceed,
		kActivateFailed,
		kNullFlag
	}SendFlag;


	//cannot use member signal in MutiThread env
	//so we new this class every time when we want use
	//Errorcode errorcode;
	typedef enum
	{
		kNoField =1,
		kQueryLenIsWrong,
		kQueryNumberUndefine,
		kNoProjectID,
		kProjectIDIsWrong,
		kNoVendorID,
		kVendorIDIsWrong,
		kProjectIDTooLong,
		kVendorIDTooLong,
		kSTBIDTooLong,
		kChipIDTooLong,
		kMACLenIsWrong,
		kNullError
	}Errorcode;

protected:
	//copy of the get buf, eg 
	//"Query=0&ProjectID=1234567&VendorID=9876543&STBID=011010000021E030002100300004C7E7"
	LPSTR recvdatabuf;

	//payloadbuf to build
	LPSTR payloadbuf;

	//the flag to signal how we build the payloadbuf
	UCHAR sendflag;

	//the error signal
	UCHAR errorcode;

protected:
	//here is the five item that is agreed with both side
	LPSTR projectid;
	LPSTR vendorid;
	LPSTR stbid;
	LPSTR chipid;
	LPSTR macaddr;

private:
	//error context corresponding to error code
	static LPSTR ErrorContext[]; 
	
protected:
	//parse value of item, eg STBID
	void ParseItem(LPSTR itemvalue, const LPSTR location);

	//parse the query for auth
	void ParseQueryAuth(void);

	//parse the query for activate
	void ParseQueryActivate(void);

	//parse the whole query context
	void ParseQuery(void);

	//parse the whole activate context
	void ParseActivate(void);

	//parse the whole get context
	void ParseRecvBuf(void);

private:
	//let drived class to cover this four virtual method for active
	//and bulid real respond
	virtual BOOL BuildQueryActivateSumPayLoad(void);
	BOOL BuildQueryActivateStatePayLoad(void);
	virtual BOOL BuildQueryActivatePayLoadByStbId(void);
	virtual BOOL BuildQueryActivatePayLoadByChipId(void);
	virtual BOOL BuildQueryActivatePayLoadByMac(void);

private:
	//let drived class to cover this four virtual method for auth
	//and bulid real respond
	virtual BOOL BuildQueryAuthSumPayLoad(void);
	BOOL BuildQueryAuthStatePayLoad(void);
	virtual BOOL BuildQueryAuthPayLoadByStbId(void);
	virtual BOOL BuildQueryAuthPayLoadByChipId(void);
	virtual BOOL BuildQueryAuthPayLoadByMac(void);

protected:
	//bulid the send data when error occurs
	BOOL BuildErrorPayLoad(void);

	//the method hold all build method
	BOOL BuildPayLoad(void);

public:
	//my public method for others' call
	//it will parse get buf first and then bulid send data
	virtual BOOL HandleHttpData(void);	

	//get the new send data
	LPSTR GetPayLoadBuf(void) const;
};
