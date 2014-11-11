#pragma once

#include "stdafx.h"

#include "HttpServer.h"

//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
//TODO: use OpenSSL with CompletionPort
//follow this page:
/*
//http://stackoverflow.com/questions/4403816/io-completion-ports-and-openssl
//
*/
//pay attention for acceptex function
//you should do ssl_accpet in DoAcceptEx
//but it may has some problem I imagine, and I'm not sure
//the worse solution is use accept instead of acceptex
//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

/* SSLeay stuff */
#include <openssl/rsa.h>       
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#pragma comment(lib, "ssleay32.lib")

class SSLserver : public HttpServer
{
public:
	SSLserver(u_short useport);
	virtual ~SSLserver(void);

private:
	SSL_CTX* ctx;
	SSL*     ssl;
	X509*    client_cert;

private:
	BOOL InitializeSSL(void);
	BOOL SSLAccept(PerIOData *listeniodata);
};