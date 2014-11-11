#include "stdafx.h"

#include "SSLserver.h"

#define HOME ".\\cert\\server\\"
#define CERTF HOME "server-cert.pem"
#define KEYF  HOME "server-key.pem"

SSLserver::SSLserver(u_short useport):
	HttpServer(useport),
	ctx(NULL),
	ssl(NULL),
	client_cert(NULL)
{
	
}

SSLserver::~SSLserver()
{

}

BOOL SSLserver::InitializeSSL()
{
	/* SSL preliminaries. We keep the certificate and key with the context. */
	SSL_load_error_strings();
	SSLeay_add_ssl_algorithms();

	ctx = SSL_CTX_new (SSLv23_server_method());
	if (!ctx) {
		AMS_DBUG("SSL_CTX_new SSLv23_server_method failed: %d\n", 
			GetLastError());
		return FALSE;
	}

	if (SSL_CTX_use_certificate_file(ctx, CERTF, SSL_FILETYPE_PEM) <= 0) 
	{
		AMS_DBUG("SSL_CTX_use_certificate_file %s failed: %d\n", 
			CERTF, GetLastError());
		return FALSE;
	}

	if (SSL_CTX_use_PrivateKey_file(ctx, KEYF, SSL_FILETYPE_PEM) <= 0) 
	{
		AMS_DBUG("SSL_CTX_use_certificate_file %s failed: %d\n", 
			KEYF, GetLastError());
		return FALSE;
	}

	if (!SSL_CTX_check_private_key(ctx)) 
	{
		AMS_DBUG("Private key does not match the certificate public key, errno: %d\n", 
			GetLastError());
		return FALSE;
	}

	return TRUE;
}

