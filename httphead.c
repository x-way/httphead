/*
 * httphead - show http header of a website
 */

/*
 * Copyright (c) 2006 Andreas Jaggi <andreas.jaggi@waterwave.ch>
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. The name of the author may not be used to endorse or promote products
 *    derived from this software without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <unistd.h>
#include <stdlib.h>
#include <netdb.h>

#include <errno.h>
#include <string.h>

const char usagemsg[] =
	"usage: httphead [-r] [-q] [-n] [-u ua] [-a as] [-e ae] [-c ac] [-l al] URL\n"
	"\n"
	"options:\n"
	"    -r               show sent request\n"
	"    -q               show only the recieved status code\n"
	"    -n               don't send User-Agent\n"
	"    -u useragent     send User-Agent: useragent\n"
	"    -a acceptstr     send Accept: acceptstr\n"
	"    -e acceptenc     send Accept-Encoding: acceptenc\n"
	"    -c acceptchs     send Accept-Charset: acceptchs\n"
	"    -l acceptlng     send Accept-Language: acceptlng\n"
	" also: -v    show version\n"
	"       -h    display this help\n"
	"       -b    display (BSD) license\n"
	;

const char licensemsg[] =
	"httphead is copyright (c) 2006 Andreas Jaggi <andreas.jaggi@waterwave.ch>\n"
	"All rights reserved.\n"
	"\n"
	"Redistribution and use in source and binary forms, with or without\n"
	"modification, are permitted provided that the following conditions\n"
	"are met:\n"
	"1. Redistributions of source code must retain the above copyright\n"
	"   notice, this list of conditions and the following disclaimer.\n"
	"2. Redistributions in binary form must reproduce the above copyright\n"
	"   notice, this list of conditions and the following disclaimer in the\n"
	"   documentation and/or other materials provided with the distribution.\n"
	"3. The name of the author may not be used to endorse or promote products\n"
	"   derived from this software without specific prior written permission.\n"
	"\n"
	"THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR\n"
	"IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES\n"
	"OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.\n"
	"IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,\n"
	"INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT\n"
	"NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,\n"
	"DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY\n"
	"THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT\n"
	"(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF\n"
	"THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.\n"
	;

const char versionmsg[] = "httphead 0.6";

const char b64chars[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

#define HH_BUFFSIZE 512

void usage ( );
void license ( );
void version ( );

void showerror ( const char* type );

void crlf ( int s );

char* getuser ( const char* url );
char* getpassword ( const char* url );
char* gethost ( const char* url );
char* getport ( const char* url );
char* getpath ( const char* url );
char* getstatuscode ( const char* response );
char* b64 ( const char* str );
void sendrequest(int stream, const char* path, const char* host,
	const char* port, const char* authstring, const char* useragent,
	const char* accept, const char* acceptencoding,
	const char* acceptcharset, const char* acceptlanguage);

int main ( int argc, char* argv[] ) {

	int tmp;
	int l;
	char* hend = NULL;
	char buff[HH_BUFFSIZE+1];

	char* url = NULL;

	char* useragent = (char*)versionmsg;
	char* accept = NULL;
	char* acceptencoding = NULL;
	char* acceptcharset = NULL;
	char* acceptlanguage = NULL;

	char* host = NULL;
	char* path = NULL;
	char* username = NULL;
	char* password = NULL;
	char* port = NULL;

	int showrequest = 0;
	int nouseragent = 0;
	int statuscodeonly = 0;
	char* statuscode = NULL;

	char* authstring = NULL;
	struct hostent *remote_hostent = NULL;
	struct sockaddr_in *remote_host_addr = NULL;

	int sock;

	while ( (tmp = getopt(argc, argv, "l:c:e:a:nu:qrhbv")) != -1 ) {
		switch ( tmp ) {
			case 'h':
				usage();
				exit(0);
			case 'b':
				license();
				exit(0);
			case 'v':
				version();
				exit(0);
			case 'q':
				statuscodeonly = 1;
				break;
			case 'r':
				showrequest = 1;
				break;
			case 'u':
				useragent = optarg;
				break;
			case 'n':
				nouseragent = 1;
				break;
			case 'a':
				accept = optarg;
				break;
			case 'e':
				acceptencoding = optarg;
				break;
			case 'c':
				acceptcharset = optarg;
				break;
			case 'l':
				acceptlanguage = optarg;
				break;
		}
	}

	if ( optind >= argc ) {
		usage();
		exit(0);
	}

	url = argv[optind];

	path = getpath(url);

	host = gethost(url);

	port = getport(url);

	username = getuser(url);

	password = getpassword(url);

	if ((username != NULL) || (password != NULL)) {
		if ( username != NULL ) {
			l = strlen(username);
			tmp = l;
		} else {
			tmp = 0;
			l = 0;
		}

		tmp++;

		if ( password != NULL ) {
			tmp += strlen(password);
		}

		authstring = (char*)alloca(sizeof(char)*(tmp+1));

		if ( username != NULL ) {
			memcpy(authstring, username, l);
		}

		authstring[l] = ':';

		if ( password != NULL ) {
			memcpy(authstring+l+1, password, strlen(password));
		}

		authstring[tmp] = '\0';

		authstring = b64(authstring);
	}

	if ( nouseragent ) {
		useragent = NULL;
	}

	remote_hostent = gethostbyname(host);

	if ( remote_hostent == NULL ) {
		herror(NULL);
		exit(-1);
	}

	remote_host_addr = alloca(sizeof(struct sockaddr_in));
	remote_host_addr->sin_family = AF_INET;
	remote_host_addr->sin_addr = *((struct in_addr*) remote_hostent->h_addr);
	if ( port == NULL ) {
		remote_host_addr->sin_port = htons(80);
	} else {
		remote_host_addr->sin_port = htons(atoi(port));
	}

	if ( (sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == -1 ) {
		showerror("socket");
		exit(-1);
	}

	if ( connect(sock, (struct sockaddr*)remote_host_addr, sizeof(struct sockaddr_in)) == -1 ) {
		showerror("connect");
		exit(-1);
	}


	/* main */

	if ( showrequest ) {
		sendrequest(1, path, host, port, authstring, useragent, accept, acceptencoding, acceptcharset, acceptlanguage);
		write(1, "Response:\n\n", 12);
	}

	sendrequest(sock, path, host, port, authstring, useragent, accept, acceptencoding, acceptcharset, acceptlanguage);


	while ( (tmp = recv(sock, buff, HH_BUFFSIZE, 0)) > 0 ) {
		buff[tmp] = '\0';

		if ( (hend = strstr(buff, "\x0D\x0A\x0D\x0A")) != NULL) {
			tmp = hend-buff;
			*hend = '\n';
			*(hend+1) = '\0';
		}

		if ( statuscodeonly ) {
			if ( (statuscode = getstatuscode(buff)) != NULL ) {
				write(1, statuscode, 3);
				write(1, "\n", 1);
				break;
			}
		} else {
			write(1, buff, tmp);
		}

		if ( hend != NULL ) {
			break;
		}
	}

	if ( shutdown(sock, 2) == -1 ) {
		showerror("shutdown");
		exit(-1);
	}

	if ( close(sock) == -1 ) {
		showerror("close");
		exit(-1);
	}

	exit(0);
}

char* _gethoststart ( const char* url )
{
	char* s = NULL;

	s = strstr(url, "http://");
	if ( s == NULL ) {
		s = strstr(url, "https://");
		if ( s == NULL ) {
			s = (char*)url; // assume that there is no http[s]:// prefix
		} else {
			s += 8;
		}
	} else {
		s += 7;
	}

	if ( s >= url + strlen(url) ) {
		return (char*)url;
	}

	return s;
}

int _gethostlen ( const char* s )
{
	int e = 0;

	while ( *(s+e) != '/' && *(s+e) != '?' && *(s+e) != '\0' ) {
		e++;
	}

	return e;
}

char* getstatuscode ( const char* s )
{
	char* htp = NULL;

	if ( (htp = strstr(s, "HTTP/")) != NULL ) {
		while ( *htp != '\0' && *htp != ' ' ) {
			htp++;
		}

		while ( *htp == ' ' ) {
			htp++;
		}

		if ( *htp == '\0' ) {
			htp = NULL;
		}
	}

	return htp;
}

char* getpath ( const char* url )
{
	char* s = NULL;
	int e;

	s = _gethoststart(url);

	e = _gethostlen(s);

	s += e;

	if ( s >= url+strlen(url) ) {
		s = (char*)malloc(2*sizeof(char));
		*s = '/';
		*(s+1) = '\0';
	}

	return s;
}

char* getpassword ( const char* url )
{
	char* s = NULL;
	char* r = NULL;
	int e = 0;
	char* a = NULL;
	char* p = NULL;

	s = _gethoststart(url);

	e = _gethostlen(s);

	a = strchr(s, '@');

	if ((a == NULL) || (a >= s+e)) {
		return NULL;
	}

	p = strchr(s, ':');

	if ((p == NULL) || (p >= a-1)) {
		return NULL;
	}

	r = (char*)malloc(sizeof(char)*(a - p));

	memcpy(r, p+1, (a - p - 1));

	r[a-p] = '\0';

	return r;
}

char* getuser ( const char* url )
{
	char* s = NULL;
	char* r = NULL;
	int e = 0;
	char* a = NULL;
	char* p = NULL;

	s = _gethoststart(url);

	e = _gethostlen(s);

	a = strchr(s, '@');

	if ((a == NULL) || (a >= s+e)) {
		return NULL;
	}

	p = strchr(s, ':');

	if ((p == NULL) || (p >= s+e)) {
		return NULL;
	}

	if (p > a) {
		p = a;
	}

	r = (char*)malloc(sizeof(char)*(1 + (int)p - (int)s));

	memcpy(r, s, (int)p-(int)s);

	r[1+(int)p-(int)s] = '\0';

	return r;
}

char* gethost ( const char* url )
{
	char* s = NULL;
	char* r = NULL;
	int e = 0;
	char* a = NULL;
	char* p = NULL;

	s = _gethoststart(url);

	e = _gethostlen(s);

	if ( e == 0 ) {
		return NULL;
	}

	a = strchr(s, '@');

	if ((a != NULL) && (a < s+e)) {
		e = ((int)(s+e) - (int)a - 1);
		s = a+1;
	}

	p = strchr(s, ':');

	if ((p != NULL) && ((a == NULL) || (p>a)) && (p < s+e)) {
		e = (int)p-(int)s;
	}

	r = (char*)malloc(sizeof(char)*(e+1));

	memcpy(r, s, e);

	r[e+1] = '\0';

	return r;
}

char* getport ( const char* url )
{
	char* s = NULL;
	char* r = NULL;
	int e = 0;
	char* a = NULL;
	char* p = NULL;

	s = _gethoststart(url);

	e = _gethostlen(s);

	if ( e == 0 ) {
		return NULL;
	}

	a = strchr(s, '@');

	if ((a != NULL) && (a < s+e)) {
		e = ((int)(s+e) - (int)a - 1);
		s = a+1;
	}

	p = strchr(s, ':');

	if ((p != NULL) && ((a == NULL) || (p>a)) && (p < s+e-1)) {
		p++;
		e = e - ((int)p-(int)s);

		r = (char*)malloc(sizeof(char)*(e+1));

		memcpy(r, p, e);

		r[e+1] = '\0';

		return r;
	} else {
		return NULL;
	}

}

void _b64_43 ( char* dst, const char* src )
{
	dst[0] = b64chars[src[0] >> 2];
	dst[1] = b64chars[(((src[0] & 0x03) << 4) & 0xF0) | (src[1] >> 4)];
	dst[2] = b64chars[((src[1] << 2) & 0x3C) | ((src[2] >> 6) & 0x03)];
	dst[3] = b64chars[src[2] & 0x3F];
}

char* b64 ( const char* str )
{
	int l = strlen(str);
	char* out = NULL;
	int pos = 0;
	int nl;

	if ( l%3 == 0 ) {
		nl = (l/3)*4;
	} else {
		nl = ((l+(3-(l%3)))/3)*4;
	}

	out = (char*)malloc(sizeof(char)*(nl+1));
	out[nl] = '\0';

	for ( pos = 0; pos < (nl/4)-1; pos++ ) {
		_b64_43(out + pos*4, str + pos*3);
	}

	if ( l%3 == 0 ) {
		_b64_43(out + (nl/4 - 1)*4, str + (nl/4 - 1)*3);
	}

	if ((l%3) == 2 ) {
		out[nl-4] = b64chars[str[l-2] >> 2];
		out[nl-3] = b64chars[(((str[l-2] & 0x03) << 4) & 0xF0) | (str[l-1] >> 4)];
		out[nl-2] = b64chars[((str[l-1] << 2) & 0x3C)];
		out[nl-1] = '=';
	}

	if ((l%3) == 1 ) {
		out[nl-4] = b64chars[str[l-2] >> 2];
		out[nl-3] = b64chars[(((str[l-2] & 0x03) << 4) & 0xF0)];
		out[nl-2] = '=';
		out[nl-1] = '=';
	}

	out[nl] = '\0';

	return out;
}

void crlf ( int s )
{
	write(s, "\x0D\x0A", 2);
}

void usage ( )
{
	write(1, usagemsg, strlen(usagemsg));
}

void license ( )
{
	write(1, licensemsg, strlen(licensemsg));
}

void version ( )
{
	write(1, versionmsg, strlen(versionmsg));
	write(1, "\n", 1);
}

void showerror ( const char* type )
{
	char* errstr = strerror(errno);
	write(2, type, strlen(type));
	write(2, ": ", 2);
	write(2, errstr, strlen(errstr));
	write(2, "\n", 1);
}

void sendrequest ( int stream, const char* path, const char* host,
	const char* port, const char* authstring, const char* useragent,
	const char* accept, const char* acceptencoding,
	const char* acceptcharset, const char* acceptlanguage )
{

	write(stream, "GET ", 4);
	write(stream, path, strlen(path));
	write(stream, " HTTP/1.0", 9);
	crlf(stream);
	write(stream, "Host: ", 6);
	write(stream, host, strlen(host));
	if ( port != NULL ) {
		write(stream, ":", 1);
		write(stream, port, strlen(port));
	}
	crlf(stream);

	if ( authstring != NULL ) {
		write(stream, "Authorization: Basic ", 21);
		write(stream, authstring, strlen(authstring));
		crlf(stream);
	}

	if ( useragent != NULL ) {
		write(stream, "User-Agent: ", 12);
		write(stream, useragent, strlen(useragent));
		crlf(stream);
	}

	if ( accept != NULL ) {
		write(stream, "Accept: ", 8);
		write(stream, accept, strlen(accept));
		crlf(stream);
	}

	if ( acceptencoding != NULL ) {
		write(stream, "Accept-Encoding: ", 17);
		write(stream, acceptencoding, strlen(acceptencoding));
		crlf(stream);
	}

	if ( acceptcharset != NULL ) {
		write(stream, "Accept-Charset: ", 16);
		write(stream, acceptcharset, strlen(acceptcharset));
		crlf(stream);
	}

	if ( acceptlanguage != NULL ) {
		write(stream, "Accept-Language: ", 17);
		write(stream, acceptlanguage, strlen(acceptlanguage));
		crlf(stream);
	}

	crlf(stream);
}
