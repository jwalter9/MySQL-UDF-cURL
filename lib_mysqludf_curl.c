/*
        lib_mysqludf_curl - a library for sending and receiving via cURL
	Copyright (C) 2014 Jeff Walter 
	web: http://www.mysqludf.org/
	email: lowadobe@gmail.com
	
	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Lesser General Public
	License as published by the Free Software Foundation; either
	version 2.1 of the License, or (at your option) any later version.
	
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Lesser General Public License for more details.
	
	You should have received a copy of the GNU Lesser General Public
	License along with this library; if not, write to the Free Software
	Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/
#if defined(_WIN32) || defined(_WIN64) || defined(__WIN32__) || defined(WIN32)
#define DLLEXP __declspec(dllexport) 
#else
#define DLLEXP
#endif

#ifdef STANDARD
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#ifdef __WIN__
typedef unsigned __int64 ulonglong;
typedef __int64 longlong;
#else
typedef unsigned long long ulonglong;
typedef long long longlong;
#endif /*__WIN__*/
#else
#include <my_global.h>
#include <my_sys.h>
#endif
#include <mysql.h>
#include <m_ctype.h>
#include <m_string.h>
#include <stdlib.h>

#include <ctype.h>

#include "curl/curl.h"
#define TEMP_DIR "/tmp"

#ifdef HAVE_DLOPEN
#ifdef	__cplusplus
extern "C" {
#endif

#define LIBVERSION "lib_mysqludf_curl version 0.0.1"

#ifdef __WIN__
#define SETENV(name,value)		SetEnvironmentVariable(name,value);
#else
#define SETENV(name,value)		setenv(name,value,1);		
#endif

DLLEXP 
my_bool curl_init(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char *message
);

DLLEXP 
void curl_deinit(
	UDF_INIT *initid
);

DLLEXP 
char *curl(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char* result
,	unsigned long* length
,	char *is_null
,	char *error
);


DLLEXP 
my_bool url_encode_init(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char *message
);

DLLEXP 
void url_encode_deinit(
	UDF_INIT *initid
);

DLLEXP 
char *url_encode(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char* result
,	unsigned long* length
,	char *is_null
,	char *error
);

#ifdef	__cplusplus
}
#endif


my_bool curl_init(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char *message
){
	if(args->arg_count == 1
	&& args->arg_type[0]==STRING_RESULT){
		return 0;
	} else {
		strcpy(
			message,"Expected 1 parameter: list of CURLOPT options"
		);		
		return 1;
	}
}
void curl_deinit(
	UDF_INIT *initid
){
}

struct curl_slist *make_slist(char *strval, struct curl_slist **slists){
    int ind;
    for(ind = 0; ind < 9 && slists[ind]; ind++);
    struct curl_slist *item = NULL;
    char *val = strval;
    char cpy[strlen(strval) + 1];
    size_t len = 0;

    while(strlen(val)){
        len = strcspn(val, ";");
        strncpy(cpy, val, len);
        cpy[len] = '\0';
        if(!item){
            item = curl_slist_append(item, cpy);
            slists[ind] = item;
        }else{
            item = curl_slist_append(item, cpy);
        };
        val += len;
        if(val[0] == ';') val++;
    };
    
    return slists[ind];
}

long proto_bitmask(char *strval){
    char *val = strval;
    char cpy[strlen(strval) + 1];
    size_t len = 0;
    long mask = 0;

    while(strlen(val)){
        len = strspn(val, " \t|");
        val += len;
        len = strcspn(val, " \t|");
        strncpy(cpy, val, len);
        cpy[len] = '\0';
        val += len;
        
        if(strcmp(cpy, "CURLPROTO_HTTP") == 0)
            mask |= CURLPROTO_HTTP;
        else if(strcmp(cpy, "CURLPROTO_HTTPS") == 0)
            mask |= CURLPROTO_HTTPS;
        else if(strcmp(cpy, "CURLPROTO_FTP") == 0)
            mask |= CURLPROTO_FTP;
        else if(strcmp(cpy, "CURLPROTO_FTPS") == 0)
            mask |= CURLPROTO_FTPS;
        else if(strcmp(cpy, "CURLPROTO_SCP") == 0)
            mask |= CURLPROTO_SCP;
        else if(strcmp(cpy, "CURLPROTO_SFTP") == 0)
            mask |= CURLPROTO_SFTP;
        else if(strcmp(cpy, "CURLPROTO_TELNET") == 0)
            mask |= CURLPROTO_TELNET;
        else if(strcmp(cpy, "CURLPROTO_LDAP") == 0)
            mask |= CURLPROTO_LDAP;
        else if(strcmp(cpy, "CURLPROTO_LDAPS") == 0)
            mask |= CURLPROTO_LDAPS;
        else if(strcmp(cpy, "CURLPROTO_DICT") == 0)
            mask |= CURLPROTO_DICT;
        else if(strcmp(cpy, "CURLPROTO_FILE") == 0)
            mask |= CURLPROTO_FILE;
        else if(strcmp(cpy, "CURLPROTO_TFTP") == 0)
            mask |= CURLPROTO_TFTP;
        else if(strcmp(cpy, "CURLPROTO_IMAP") == 0)
            mask |= CURLPROTO_IMAP;
        else if(strcmp(cpy, "CURLPROTO_IMAPS") == 0)
            mask |= CURLPROTO_IMAPS;
        else if(strcmp(cpy, "CURLPROTO_POP3") == 0)
            mask |= CURLPROTO_POP3;
        else if(strcmp(cpy, "CURLPROTO_POP3S") == 0)
            mask |= CURLPROTO_POP3S;
        else if(strcmp(cpy, "CURLPROTO_SMTP") == 0)
            mask |= CURLPROTO_SMTP;
        else if(strcmp(cpy, "CURLPROTO_SMTPS") == 0)
            mask |= CURLPROTO_SMTPS;
        else if(strcmp(cpy, "CURLPROTO_RTSP") == 0)
            mask |= CURLPROTO_RTSP;
        else if(strcmp(cpy, "CURLPROTO_RTMP") == 0)
            mask |= CURLPROTO_RTMP;
        else if(strcmp(cpy, "CURLPROTO_RTMPT") == 0)
            mask |= CURLPROTO_RTMPT;
        else if(strcmp(cpy, "CURLPROTO_RTMPE") == 0)
            mask |= CURLPROTO_RTMPE;
        else if(strcmp(cpy, "CURLPROTO_RTMPTE") == 0)
            mask |= CURLPROTO_RTMPTE;
        else if(strcmp(cpy, "CURLPROTO_RTMPS") == 0)
            mask |= CURLPROTO_RTMPS;
        else if(strcmp(cpy, "CURLPROTO_RTMPTS") == 0)
            mask |= CURLPROTO_RTMPTS;
        else if(strcmp(cpy, "CURLPROTO_GOPHER") == 0)
            mask |= CURLPROTO_GOPHER;
    };
    return mask;
}

long auth_bitmask(char *strval){
    char *val = strval;
    char cpy[strlen(strval) + 1];
    size_t len = 0;
    long mask = 0;

    while(strlen(val)){
        len = strspn(val, " \t|");
        val += len;
        len = strcspn(val, " \t|");
        strncpy(cpy, val, len);
        cpy[len] = '\0';
        val += len;
        
        if(strcmp(cpy, "CURLAUTH_ONLY") == 0)
            mask |= CURLAUTH_ONLY;
        else if(strcmp(cpy, "CURLAUTH_ANYSAFE") == 0)
            mask |= CURLAUTH_ANYSAFE;
        else if(strcmp(cpy, "CURLAUTH_ANY") == 0)
            mask |= CURLAUTH_ANY;
        else if(strcmp(cpy, "CURLAUTH_NTLM_WB") == 0)
            mask |= CURLAUTH_NTLM_WB;
        else if(strcmp(cpy, "CURLAUTH_NTLM") == 0)
            mask |= CURLAUTH_NTLM;
        else if(strcmp(cpy, "CURLAUTH_GSSNEGOTIATE") == 0)
            mask |= CURLAUTH_GSSNEGOTIATE;
        else if(strcmp(cpy, "CURLAUTH_DIGEST_IE") == 0)
            mask |= CURLAUTH_DIGEST_IE;
        else if(strcmp(cpy, "CURLAUTH_DIGEST") == 0)
            mask |= CURLAUTH_DIGEST;
        else if(strcmp(cpy, "CURLAUTH_BASIC") == 0)
            mask |= CURLAUTH_BASIC;
    };
    return mask;
}

long redir_bitmask(char *strval){
    char *val = strval;
    char cpy[strlen(strval) + 1];
    size_t len = 0;
    long mask = 0;

    while(strlen(val)){
        len = strspn(val, " \t|");
        val += len;
        len = strcspn(val, " \t|");
        strncpy(cpy, val, len);
        cpy[len] = '\0';
        val += len;
        
        if(strcmp(cpy, "CURL_REDIR_POST_301") == 0)
            mask |= CURL_REDIR_POST_301;
        else if(strcmp(cpy, "CURL_REDIR_POST_302") == 0)
            mask |= CURL_REDIR_POST_302;
        else if(strcmp(cpy, "CURL_REDIR_POST_303") == 0)
            mask |= CURL_REDIR_POST_303;
    };
    return mask;
}

long sshauth_bitmask(char *strval){
    char *val = strval;
    char cpy[strlen(strval) + 1];
    size_t len = 0;
    long mask = 0;

    while(strlen(val)){
        len = strspn(val, " \t|");
        val += len;
        len = strcspn(val, " \t|");
        strncpy(cpy, val, len);
        cpy[len] = '\0';
        val += len;
        
        if(strcmp(cpy, "CURLSSH_AUTH_PUBLICKEY") == 0)
            mask |= CURLSSH_AUTH_PUBLICKEY;
        else if(strcmp(cpy, "CURLSSH_AUTH_PASSWORD") == 0)
            mask |= CURLSSH_AUTH_PASSWORD;
        else if(strcmp(cpy, "CURLSSH_AUTH_HOST") == 0)
            mask |= CURLSSH_AUTH_HOST;
        else if(strcmp(cpy, "CURLSSH_AUTH_AGENT") == 0)
            mask |= CURLSSH_AUTH_AGENT;
    };
    return mask;
}

char *udf_setopt(CURL *hnd, char *op, struct curl_slist **slists){
    char *opstr = op;
    opstr += 8; /* skip "CURLOPT_" - will always match */
    size_t len, vlen;
    char *val;
    
    /* speed up the comparisons by first determining the option length */
    len = strcspn(opstr, " \t");
    vlen = strspn(&opstr[len], " \t") + len;
    val = opstr + vlen;
    vlen = strcspn(val, "\n");
    long onoff = atol(val) ? 1 : 0;
    char strval[vlen + 1];
    strncpy(strval, val, vlen);
    strval[vlen] = '\0';
    
    switch(len){
    case 3:
        if(strncmp(opstr, "URL", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_URL, strval);
        }else if(strncmp(opstr, "PUT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PUT, onoff);
        };
        break;
    case 4:
        if(strncmp(opstr, "PORT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PORT, atol(strval));
        }else if(strncmp(opstr, "POST", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POST, onoff);
        }else if(strncmp(opstr, "CRLF", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CRLF, onoff);
        };
        break;
    case 5:
        if(strncmp(opstr, "PROXY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXY, strval);
        }else if(strncmp(opstr, "NETRC", len) == 0){
            if(strncmp(strval, "CURL_NETRC_OPTIONAL", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_NETRC, CURL_NETRC_OPTIONAL);
            else if(strncmp(strval, "CURL_NETRC_IGNORED", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_NETRC, CURL_NETRC_IGNORED);
            else if(strncmp(strval, "CURL_NETRC_REQUIRED", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_NETRC, CURL_NETRC_REQUIRED);
        }else if(strncmp(opstr, "QUOTE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_QUOTE, make_slist(strval, slists));
        }else if(strncmp(opstr, "RANGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RANGE, strval);
        };
        break;
    case 6:
        if(strncmp(opstr, "COOKIE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COOKIE, strval);
        }else if(strncmp(opstr, "APPEND", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_APPEND, onoff);
        }else if(strncmp(opstr, "NOBODY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_NOBODY, onoff);
        }else if(strncmp(opstr, "UPLOAD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_UPLOAD, onoff);
        }else if(strncmp(opstr, "SSLKEY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLKEY, strval);
        }else if(strncmp(opstr, "CAINFO", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CAINFO, strval);
        }else if(strncmp(opstr, "CAPATH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CAPATH, strval);
        };
        break;
    case 7:
        if(strncmp(opstr, "NOPROXY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_NOPROXY, strval);
        }else if(strncmp(opstr, "USERPWD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_USERPWD, strval);
        }else if(strncmp(opstr, "SASL_IR", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SASL_IR, onoff);
        }else if(strncmp(opstr, "REFERER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_REFERER, strval);
        }else if(strncmp(opstr, "HTTPGET", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTPGET, onoff);
        }else if(strncmp(opstr, "FTPPORT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTPPORT, strval);
        }else if(strncmp(opstr, "TIMEOUT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TIMEOUT, atol(strval));
        }else if(strncmp(opstr, "USE_SSL", len) == 0){
            if(strncmp(strval, "CURLUSESSL_NONE", 15) == 0)
                curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_NONE);
            else if(strncmp(strval, "CURLUSESSL_TRY", 14) == 0)
                curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_TRY);
            else if(strncmp(strval, "CURLUSESSL_CONTROL", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_CONTROL);
            else if(strncmp(strval, "CURLUSESSL_ALL", 14) == 0)
                curl_easy_setopt(hnd, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        }else if(strncmp(opstr, "RESOLVE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RESOLVE, make_slist(strval, slists));
        }else if(strncmp(opstr, "SSLCERT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLCERT, strval);
        }else if(strncmp(opstr, "CRLFILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CRLFILE, strval);
        };
        break;
    case 8:
        if(strncmp(opstr, "USERNAME", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_USERNAME, strval);
        }else if(strncmp(opstr, "PASSWORD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PASSWORD, strval);
        }else if(strncmp(opstr, "HTTPAUTH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTPAUTH, auth_bitmask(strval));
        }else if(strncmp(opstr, "PREQUOTE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PREQUOTE, make_slist(strval, slists));
        }else if(strncmp(opstr, "FILETIME", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FILETIME, onoff);
        }else if(strncmp(opstr, "CERTINFO", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CERTINFO, onoff);
        }else if(strncmp(opstr, "KRBLEVEL", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_KRBLEVEL, onoff);
        };
        break;
    case 9:
        if(strncmp(opstr, "PROTOCOLS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROTOCOLS, proto_bitmask(strval));
        }else if(strncmp(opstr, "PROXYPORT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYPORT, atol(strval));
        }else if(strncmp(opstr, "PROXYTYPE", len) == 0){
            if(strncmp(strval, "CURLPROXY_HTTP_1_0", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_HTTP_1_0);
            else if(strncmp(strval, "CURLPROXY_HTTP", 14) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_HTTP);
            else if(strncmp(strval, "CURLPROXY_SOCKS5_HOSTNAME", 25) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5_HOSTNAME);
            else if(strncmp(strval, "CURLPROXY_SOCKS4A", 17) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4A);
            else if(strncmp(strval, "CURLPROXY_SOCKS5", 16) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS5);
            else if(strncmp(strval, "CURLPROXY_SOCKS4", 16) == 0)
                curl_easy_setopt(hnd, CURLOPT_PROXYTYPE, CURLPROXY_SOCKS4);
        }else if(strncmp(opstr, "INTERFACE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_INTERFACE, strval);
        }else if(strncmp(opstr, "LOCALPORT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_LOCALPORT, atol(strval));
        }else if(strncmp(opstr, "PROXYAUTH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYAUTH, auth_bitmask(strval));
        }else if(strncmp(opstr, "MAXREDIRS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAXREDIRS, atol(strval));
        }else if(strncmp(opstr, "POSTREDIR", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POSTREDIR, redir_bitmask(strval));
        }else if(strncmp(opstr, "USERAGENT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_USERAGENT, strval);
        }else if(strncmp(opstr, "EGDSOCKET", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_EGDSOCKET, strval);
        }else if(strncmp(opstr, "COOKIEJAR", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COOKIEJAR, strval);
        }else if(strncmp(opstr, "MAIL_FROM", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAIL_FROM, strval);
        }else if(strncmp(opstr, "MAIL_RCPT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAIL_RCPT, make_slist(strval, slists));
        }else if(strncmp(opstr, "MAIL_AUTH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAIL_AUTH, strval);
        }else if(strncmp(opstr, "POSTQUOTE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POSTQUOTE, make_slist(strval, slists));
        }else if(strncmp(opstr, "TIMEVALUE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TIMEVALUE, atol(strval));
        }else if(strncmp(opstr, "IPRESOLVE", len) == 0){
            if(strncmp(strval, "CURL_IPRESOLVE_WHATEVER", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_WHATEVER);
            else if(strncmp(strval, "CURL_IPRESOLVE_V4", 17) == 0)
                curl_easy_setopt(hnd, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
            else if(strncmp(strval, "CURL_IPRESOLVE_V6", 17) == 0)
                curl_easy_setopt(hnd, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V6);
        }else if(strncmp(opstr, "KEYPASSWD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_KEYPASSWD, strval);
        }else if(strncmp(opstr, "SSLENGINE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLENGINE, strval);
/*  added to curl in v7.37 - uncomment if needed

        }else if(strncmp(opstr, "HEADEROPT", len) == 0){
            if(strncmp(strval, "CURLHEADER_UNIFIED", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_HEADEROPT, CURLHEADER_UNIFIED);
            else if(strncmp(strval, "CURLHEADER_SEPARATE", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_HEADEROPT, CURLHEADER_SEPARATE); */
        };
        break;
    case 10:
        if(strncmp(opstr, "BUFFERSIZE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_BUFFERSIZE, atol(strval));
        }else if(strncmp(opstr, "NETRC_FILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_NETRC_FILE, strval);
        }else if(strncmp(opstr, "POSTFIELDS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDS, strval);
        }else if(strncmp(opstr, "HTTPHEADER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTPHEADER, make_slist(strval, slists));
        }else if(strncmp(opstr, "COOKIEFILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COOKIEFILE, strval);
        }else if(strncmp(opstr, "COOKIELIST", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COOKIELIST, strval);
        }else if(strncmp(opstr, "FTPSSLAUTH", len) == 0){
            if(strncmp(strval, "CURLFTPAUTH_DEFAULT", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_DEFAULT);
            else if(strncmp(strval, "CURLFTPAUTH_SSL", 15) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_SSL);
            else if(strncmp(strval, "CURLFTPAUTH_TLS", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTPSSLAUTH, CURLFTPAUTH_TLS);
        }else if(strncmp(opstr, "INFILESIZE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_INFILESIZE, atol(strval));
        }else if(strncmp(opstr, "TIMEOUT_MS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TIMEOUT_MS, atol(strval));
        }else if(strncmp(opstr, "SSLKEYTYPE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLKEYTYPE, strval);
        }else if(strncmp(opstr, "SSLVERSION", len) == 0){
            if(strncmp(strval, "CURL_SSLVERSION_DEFAULT", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_DEFAULT);
            else if(strncmp(strval, "CURL_SSLVERSION_TLSv1", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1);
            else if(strncmp(strval, "CURL_SSLVERSION_SSLv2", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv2);
            else if(strncmp(strval, "CURL_SSLVERSION_SSLv3", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_SSLv3);
            else if(strncmp(strval, "CURL_SSLVERSION_TLSv1_0", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_0);
            else if(strncmp(strval, "CURL_SSLVERSION_TLSv1_1", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_1);
            else if(strncmp(strval, "CURL_SSLVERSION_TLSv1_2", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSLVERSION, CURL_SSLVERSION_TLSv1_2);
        }else if(strncmp(opstr, "ISSUERCERT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_ISSUERCERT, strval);
        };
        break;
    case 11:
        if(strncmp(opstr, "TCP_NODELAY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TCP_NODELAY, onoff);
        }else if(strncmp(opstr, "AUTOREFERER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_AUTOREFERER, onoff);
/*  added to curl v7.37 - uncomment if needed
        }else if(strncmp(opstr, "PROXYHEADER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYHEADER, make_slist(strval, slists)); */
        }else if(strncmp(opstr, "DIRLISTONLY", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DIRLISTONLY, onoff);
        }else if(strncmp(opstr, "FTP_SSL_CCC", len) == 0){
            if(strncmp(strval, "CURLFTPSSL_CCC_NONE", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_SSL_CCC, CURLFTPSSL_CCC_NONE);
            else if(strncmp(strval, "CURLFTPSSL_CCC_PASSIVE", 22) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_SSL_CCC, CURLFTPSSL_CCC_PASSIVE);
            else if(strncmp(strval, "CURLFTPSSL_CCC_ACTIVE", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_SSL_CCC, CURLFTPSSL_CCC_ACTIVE);
        }else if(strncmp(opstr, "FTP_ACCOUNT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_ACCOUNT, strval);
        }else if(strncmp(opstr, "RESUME_FROM", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RESUME_FROM, atol(strval));
        }else if(strncmp(opstr, "MAXFILESIZE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAXFILESIZE, atol(strval));
        }else if(strncmp(opstr, "MAXCONNECTS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAXCONNECTS, atol(strval));
        }else if(strncmp(opstr, "SSLCERTTYPE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLCERTTYPE, strval);
        }else if(strncmp(opstr, "RANDOM_FILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RANDOM_FILE, strval);
        }else if(strncmp(opstr, "SSL_OPTIONS", len) == 0){
            if(strncmp(strval, "CURLSSLOPT_ALLOW_BEAST", 22) == 0)
                curl_easy_setopt(hnd, CURLOPT_SSL_OPTIONS, CURLSSLOPT_ALLOW_BEAST);
        };
        break;
    case 12:
        if(strncmp(opstr, "TCP_KEEPIDLE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TCP_KEEPIDLE, atol(strval));
        }else if(strncmp(opstr, "PROXYUSERPWD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYUSERPWD, strval);
        }else if(strncmp(opstr, "HTTP_VERSION", len) == 0){
            if(strncmp(strval, "CURL_HTTP_VERSION_NONE", 22) == 0)
                curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_NONE);
            else if(strncmp(strval, "CURL_HTTP_VERSION_1_0", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_0);
            else if(strncmp(strval, "CURL_HTTP_VERSION_1_1", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_1_1);
            else if(strncmp(strval, "CURL_HTTP_VERSION_2_0", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_HTTP_VERSION, CURL_HTTP_VERSION_2_0);
        }else if(strncmp(opstr, "TFTP_BLKSIZE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TFTP_BLKSIZE, atol(strval));
        }else if(strncmp(opstr, "FTP_USE_EPRT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_USE_EPRT, onoff);
        }else if(strncmp(opstr, "FTP_USE_EPSV", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_USE_EPSV, onoff);
        }else if(strncmp(opstr, "FTP_USE_PRET", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_USE_PRET, onoff);
        }else if(strncmp(opstr, "RTSP_REQUEST", len) == 0){
            if(strncmp(strval, "CURL_RTSPREQ_OPTIONS", 20) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_OPTIONS);
            else if(strncmp(strval, "CURL_RTSPREQ_DESCRIBE", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_DESCRIBE);
            else if(strncmp(strval, "CURL_RTSPREQ_ANNOUNCE", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_ANNOUNCE);
            else if(strncmp(strval, "CURL_RTSPREQ_SETUP", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_SETUP);
            else if(strncmp(strval, "CURL_RTSPREQ_PLAY", 17) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_PLAY);
            else if(strncmp(strval, "CURL_RTSPREQ_PAUSE", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_PAUSE);
            else if(strncmp(strval, "CURL_RTSPREQ_TEARDOWN", 21) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_TEARDOWN);
            else if(strncmp(strval, "CURL_RTSPREQ_GET_PARAMETER", 26) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_GET_PARAMETER);
            else if(strncmp(strval, "CURL_RTSPREQ_SET_PARAMETER", 26) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_SET_PARAMETER);
            else if(strncmp(strval, "CURL_RTSPREQ_RECORD", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_RECORD);
            else if(strncmp(strval, "CURL_RTSPREQ_RECEIVE", 20) == 0)
                curl_easy_setopt(hnd, CURLOPT_RTSP_REQUEST, CURL_RTSPREQ_RECEIVE);
        }else if(strncmp(opstr, "TRANSFERTEXT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TRANSFERTEXT, onoff);
        }else if(strncmp(opstr, "FORBID_REUSE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FORBID_REUSE, onoff);
        };
        break;
    case 13:
        if(strncmp(opstr, "ADDRESS_SCOPE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_ADDRESS_SCOPE, atol(strval));
        }else if(strncmp(opstr, "TCP_KEEPALIVE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TCP_KEEPALIVE, onoff);
        }else if(strncmp(opstr, "TCP_KEEPINTVL", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TCP_KEEPINTVL, atol(strval));
        }else if(strncmp(opstr, "LOGIN_OPTIONS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_LOGIN_OPTIONS, strval);
        }else if(strncmp(opstr, "PROXYUSERNAME", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYUSERNAME, strval);
        }else if(strncmp(opstr, "PROXYPASSWORD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXYPASSWORD, strval);
        }else if(strncmp(opstr, "POSTFIELDSIZE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE, atol(strval));
        }else if(strncmp(opstr, "COOKIESESSION", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COOKIESESSION, onoff);
        }else if(strncmp(opstr, "CUSTOMREQUEST", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CUSTOMREQUEST, strval);
        }else if(strncmp(opstr, "TIMECONDITION", len) == 0){
            if(strncmp(strval, "CURL_TIMECOND_IFMODSINCE", 24) == 0)
                curl_easy_setopt(hnd, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFMODSINCE);
            else if(strncmp(strval, "CURL_TIMECOND_IFUNMODSINCE", 26) == 0)
                curl_easy_setopt(hnd, CURLOPT_TIMECONDITION, CURL_TIMECOND_IFUNMODSINCE);
        }else if(strncmp(opstr, "FRESH_CONNECT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FRESH_CONNECT, onoff);
        }else if(strncmp(opstr, "DNS_INTERFACE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DNS_INTERFACE, strval);
        }else if(strncmp(opstr, "DNS_LOCAL_IP4", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DNS_LOCAL_IP4, strval);
        }else if(strncmp(opstr, "DNS_LOCAL_IP6", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DNS_LOCAL_IP6, strval);
        }else if(strncmp(opstr, "TELNETOPTIONS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TELNETOPTIONS, make_slist(strval, slists));
        }else if(strncmp(opstr, "WILDCARDMATCH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_WILDCARDMATCH, onoff);
        };
        break;
    case 14:
        if(strncmp(opstr, "LOCALPORTRANGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_LOCALPORTRANGE, atol(strval));
        }else if(strncmp(opstr, "XOAUTH2_BEARER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_XOAUTH2_BEARER, strval);
        }else if(strncmp(opstr, "FOLLOWLOCATION", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FOLLOWLOCATION, onoff);
        }else if(strncmp(opstr, "COPYPOSTFIELDS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_COPYPOSTFIELDS, strval);
        }else if(strncmp(opstr, "HTTP200ALIASES", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTP200ALIASES, make_slist(strval, slists));
        }else if(strncmp(opstr, "FTP_FILEMETHOD", len) == 0){
            if(strncmp(strval, "CURLFTPMETHOD_MULTICWD", 22) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_MULTICWD);
            else if(strncmp(strval, "CURLFTPMETHOD_NOCWD", 19) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_NOCWD);
            else if(strncmp(strval, "CURLFTPMETHOD_SINGLECWD", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_FILEMETHOD, CURLFTPMETHOD_SINGLECWD);
        }else if(strncmp(opstr, "RTSP_TRANSPORT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RTSP_TRANSPORT, strval);
        }else if(strncmp(opstr, "LOW_SPEED_TIME", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_TIME, atol(strval));
        }else if(strncmp(opstr, "CONNECTTIMEOUT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT, atol(strval));
/*  added to curl v7.37 - uncomment if needed
        }else if(strncmp(opstr, "SSL_ENABLE_NPN", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_ENABLE_NPN, onoff); */
        }else if(strncmp(opstr, "SSL_VERIFYPEER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYPEER, onoff);
        }else if(strncmp(opstr, "SSL_VERIFYHOST", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_VERIFYHOST, onoff);
        }else if(strncmp(opstr, "SSH_AUTH_TYPES", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSH_AUTH_TYPES, sshauth_bitmask(strval));
        }else if(strncmp(opstr, "SSH_KNOWNHOSTS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSH_KNOWNHOSTS, strval);
        }else if(strncmp(opstr, "NEW_FILE_PERMS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_NEW_FILE_PERMS, atol(strval));
        };
        break;
    case 15:
        if(strncmp(opstr, "REDIR_PROTOCOLS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_REDIR_PROTOCOLS, proto_bitmask(strval));
        }else if(strncmp(opstr, "HTTPPROXYTUNNEL", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTPPROXYTUNNEL, onoff);
        }else if(strncmp(opstr, "ACCEPT_ENCODING", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_ACCEPT_ENCODING, strval);
        }else if(strncmp(opstr, "RTSP_SESSION_ID", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RTSP_SESSION_ID, strval);
        }else if(strncmp(opstr, "RTSP_STREAM_URI", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RTSP_STREAM_URI, strval);
        }else if(strncmp(opstr, "LOW_SPEED_LIMIT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_LOW_SPEED_LIMIT, atol(strval));
/*  added to curl v7.37 - uncomment if needed
        }else if(strncmp(opstr, "SSL_ENABLE_ALPN", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_ENABLE_ALPN, onoff); */
        }else if(strncmp(opstr, "SSL_CIPHER_LIST", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_CIPHER_LIST, strval);
        };
        break;
    case 16:
        if(strncmp(opstr, "TLSAUTH_USERNAME", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TLSAUTH_USERNAME, strval);
        }else if(strncmp(opstr, "TLSAUTH_PASSWORD", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TLSAUTH_PASSWORD, strval);
        }else if(strncmp(opstr, "FTP_SKIP_PASV_IP", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_SKIP_PASV_IP, onoff);
        }else if(strncmp(opstr, "RTSP_CLIENT_CSEQ", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RTSP_CLIENT_CSEQ, atol(strval));
        }else if(strncmp(opstr, "RTSP_SERVER_CSEQ", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RTSP_SERVER_CSEQ, atol(strval));
        }else if(strncmp(opstr, "INFILESIZE_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_INFILESIZE_LARGE, (curl_off_t)atoll(strval));
        }else if(strncmp(opstr, "ACCEPTTIMEOUT_MS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_ACCEPTTIMEOUT_MS, atol(strval));
        };
        break;
    case 17:
        if(strncmp(opstr, "SOCKS5_GSSAPI_NEC", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SOCKS5_GSSAPI_NEC, onoff);
        }else if(strncmp(opstr, "DNS_CACHE_TIMEOUT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DNS_CACHE_TIMEOUT, atol(strval));
        }else if(strncmp(opstr, "TRANSFER_ENCODING", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_TRANSFER_ENCODING, onoff);
        }else if(strncmp(opstr, "UNRESTRICTED_AUTH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_UNRESTRICTED_AUTH, onoff);
        }else if(strncmp(opstr, "RESUME_FROM_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_RESUME_FROM_LARGE, (curl_off_t)atoll(strval));
        }else if(strncmp(opstr, "MAXFILESIZE_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAXFILESIZE_LARGE, (curl_off_t)atoll(strval));
        }else if(strncmp(opstr, "CONNECTTIMEOUT_MS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_CONNECTTIMEOUT_MS, atol(strval));
        }else if(strncmp(opstr, "SSLENGINE_DEFAULT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSLENGINE_DEFAULT, onoff);
        }else if(strncmp(opstr, "GSSAPI_DELEGATION", len) == 0){
            if(strncmp(strval, "CURLGSSAPI_DELEGATION_POLICY_FLAG", 33) == 0)
                curl_easy_setopt(hnd, CURLOPT_GSSAPI_DELEGATION, CURLGSSAPI_DELEGATION_POLICY_FLAG);
/*  added to curl v7.37 - uncomment if needed
            else if(strncmp(strval, "GSS_C_DELEG_POLICY_FLAG", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_GSSAPI_DELEGATION, GSS_C_DELEG_POLICY_FLAG); */
        };
        break;
    case 18:
        if(strncmp(opstr, "SSH_PUBLIC_KEYFILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSH_PUBLIC_KEYFILE, strval);
        };
        break;
    case 19:
        if(strncmp(opstr, "POSTFIELDSIZE_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)atoll(strval));
        }else if(strncmp(opstr, "PROXY_TRANSFER_MODE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_PROXY_TRANSFER_MODE, onoff);
        }else if(strncmp(opstr, "SSL_SESSIONID_CACHE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSL_SESSIONID_CACHE, onoff);
        }else if(strncmp(opstr, "SSH_PRIVATE_KEYFILE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSH_PRIVATE_KEYFILE, strval);
        }else if(strncmp(opstr, "NEW_DIRECTORY_PERMS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_NEW_DIRECTORY_PERMS, atol(strval));
        };
        break;
    case 20:
        if(strncmp(opstr, "DNS_USE_GLOBAL_CACHE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_DNS_USE_GLOBAL_CACHE, onoff);
        }else if(strncmp(opstr, "FTP_RESPONSE_TIMEOUT", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_RESPONSE_TIMEOUT, atol(strval));
        }else if(strncmp(opstr, "MAX_SEND_SPEED_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAX_SEND_SPEED_LARGE, (curl_off_t)atoll(strval));
        }else if(strncmp(opstr, "MAX_RECV_SPEED_LARGE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_MAX_RECV_SPEED_LARGE, (curl_off_t)atoll(strval));
        };
        break;
    default:
        if(strncmp(opstr, "SOCKS5_GSSAPI_SERVICE", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SOCKS5_GSSAPI_SERVICE, strval);
        }else if(strncmp(opstr, "IGNORE_CONTENT_LENGTH", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_IGNORE_CONTENT_LENGTH, onoff);
        }else if(strncmp(opstr, "HTTP_CONTENT_DECODING", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTP_CONTENT_DECODING, onoff);
/*  added to curl v7.37 - uncomment if needed
        }else if(strncmp(opstr, "EXPECT_100_TIMEOUT_MS", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_EXPECT_100_TIMEOUT_MS, atol(strval));  */
        }else if(strncmp(opstr, "HTTP_TRANSFER_DECODING", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_HTTP_TRANSFER_DECODING, onoff);
        }else if(strncmp(opstr, "FTP_CREATE_MISSING_DIRS", len) == 0){
            if(strncmp(strval, "CURLFTP_CREATE_DIR_NONE", 23) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_NONE);
            else if(strncmp(strval, "CURLFTP_CREATE_DIR", 18) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR);
            else if(strncmp(strval, "CURLFTP_CREATE_DIR_RETRY", 24) == 0)
                curl_easy_setopt(hnd, CURLOPT_FTP_CREATE_MISSING_DIRS, CURLFTP_CREATE_DIR_RETRY);
        }else if(strncmp(opstr, "FTP_ALTERNATIVE_TO_USER", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_FTP_ALTERNATIVE_TO_USER, strval);
        }else if(strncmp(opstr, "SSH_HOST_PUBLIC_KEY_MD5", len) == 0){
            curl_easy_setopt(hnd, CURLOPT_SSH_HOST_PUBLIC_KEY_MD5, strval);
        };
    };
    
    val += vlen;
    return val;
}

char *curl(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char* result
,	unsigned long* length
,	char *is_null
,	char *error
){
    CURLcode ret;
    CURL *hnd;
    char *opts = args->args[0];
    char *op;
    FILE *datfile;
    char *dat_filename = malloc(strlen(TEMP_DIR) + 18);
    strcpy(dat_filename, TEMP_DIR);
    strcat(dat_filename, "/curl_resp_XXXXXX");
    int dat_d, i;

    struct curl_slist *slists[9];
    for(i = 0; i < 9; i++)
        slists[i] = NULL;

    hnd = curl_easy_init();

    dat_d = mkstemp(dat_filename);
    if(dat_d == -1){
        strcpy(error, strerror(errno));
        return NULL;
    };
    datfile = fdopen(dat_d, "w");
    if(!datfile){
        strcpy(error, strerror(errno));
        return NULL;
    };
    curl_easy_setopt(hnd, CURLOPT_WRITEDATA, (void *)datfile);
    curl_easy_setopt(hnd, CURLOPT_NOPROGRESS, 1);
    curl_easy_setopt(hnd, CURLOPT_ERRORBUFFER, error);
    
    while(strlen(opts)){
        op = strstr(opts, "CURLOPT_");
        if(!op) break;
        if(strncmp(op, "CURLOPT_HEADER", 14) == 0){
            op += 14;
            op += strspn(op, " \t");
            if(op[0] == '1')
                curl_easy_setopt(hnd, CURLOPT_HEADERDATA, (void *)datfile);
            opts = op;
        }else{
            opts = udf_setopt(hnd, op, slists);
        };
    };

	ret = curl_easy_perform(hnd);
	for(i = 0; i < 9 && slists[i]; i++)
    	curl_slist_free_all(slists[i]);
	curl_easy_cleanup(hnd);
	hnd = NULL;
	
	fclose(datfile);

	if(ret) return NULL;

	memcpy(result, dat_filename, strlen(dat_filename));
	*length = strlen(dat_filename);
	return result;
}



my_bool url_encode_init(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char *message
){
	if(args->arg_count == 1
	&& args->arg_type[0]==STRING_RESULT){
	    initid->ptr = malloc(args->lengths[0] * 3 + 1);
	    if(!initid->ptr){
	        strcpy(message,"Failed to malloc memory");
	        return 1;
	    };
	} else {
		strcpy(message,"Expected 1 parameter: string to be encoded");
		return 1;
	};
	return 0;
}

void url_encode_deinit(
	UDF_INIT *initid
){
    free(initid->ptr);
}

char *url_encode(
	UDF_INIT *initid
,	UDF_ARGS *args
,	char* result
,	unsigned long* length
,	char *is_null
,	char *error
){
    CURL *hnd;
    hnd = curl_easy_init();
    char *escaped = curl_easy_escape(hnd, args->args[0], args->lengths[0]);
    strcpy(initid->ptr, escaped);
    *length = strlen(escaped);
    free(escaped);
	curl_easy_cleanup(hnd);
	hnd = NULL;
    
    return initid->ptr;
}


#endif /* HAVE_DLOPEN */
