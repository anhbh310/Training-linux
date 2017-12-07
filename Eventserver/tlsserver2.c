#include <netinet/in.h>
#include <sys/socket.h>
#include <fcntl.h>

#include <event2/event.h>
#include <event2/buffer.h>
#include <event2/bufferevent.h>

#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include "openssl/ssl.h"
#include "openssl/err.h"



#define MAX_LINE 16384

void do_read(evutil_socket_t fd, short events, void *arg);
void do_write(evutil_socket_t fd, short events, void *arg);

SSL_CTX* InitServerCTX(void){
    SSL_CTX *ctx;
 
    OpenSSL_add_all_algorithms(); 
    SSL_load_error_strings();   
    ctx = SSL_CTX_new(TLSv1_2_server_method());  
    if ( ctx == NULL )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    return ctx;
}

void LoadCertificates(SSL_CTX* ctx, char* CertFile, char* KeyFile){
    if ( SSL_CTX_use_certificate_file(ctx, CertFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if ( SSL_CTX_use_PrivateKey_file(ctx, KeyFile, SSL_FILETYPE_PEM) <= 0 )
    {
        ERR_print_errors_fp(stderr);
        abort();
    }
    if ( !SSL_CTX_check_private_key(ctx) )
    {
        fprintf(stderr, "Private key does not match the public certificate\n");
        abort();
    }
}

void readcb(struct bufferevent *bev, void *ssl)
{
	char buf[1024] = {0};
    int bytes;
    bytes = SSL_read(ssl, buf, sizeof(buf)); 
	buf[bytes]=0;
	if (bytes>0){
		SSL_write(ssl, buf, sizeof(buf));
	}
}

void errorcb(struct bufferevent *bev, short error, void *ctx) {
    if (error & BEV_EVENT_EOF) {
        /* connection has been closed, do any clean up here */
        /* ... */
    } else if (error & BEV_EVENT_ERROR) {
        /* check errno to see what error occurred */
        /* ... */
    } else if (error & BEV_EVENT_TIMEOUT) {
        /* must be a timeout event handle, handle it */
        /* ... */
    }
    bufferevent_free(bev);
}

void do_accept(evutil_socket_t listener, short event, void *arg)
{
	SSL_CTX *ctx;
	SSL_library_init();
	ctx = InitServerCTX();   
	LoadCertificates(ctx, "mycert.pem", "mycert.pem");
		
    struct event_base *base = arg;
    struct sockaddr_storage ss;
    socklen_t slen = sizeof(ss);
    int fd = accept(listener, (struct sockaddr*)&ss, &slen);
    if (fd < 0) {
        perror("accept");
    } else if (fd > FD_SETSIZE) {
        close(fd);
    } else {
		
		SSL *ssl;
		ssl = SSL_new(ctx);  
		
		SSL_set_fd(ssl, fd); 
		if (SSL_accept(ssl)<0){
			perror("SSL_accept");
		}
		else {
			evutil_make_socket_nonblocking(fd);           
			struct bufferevent *bev;
			bev = bufferevent_socket_new(base, fd, BEV_OPT_CLOSE_ON_FREE);
			bufferevent_setcb(bev, readcb, NULL, errorcb, ssl);
			bufferevent_setwatermark(bev, EV_READ, 0, MAX_LINE);
			bufferevent_enable(bev, EV_READ|EV_WRITE);
		}
    }
}

int main(int c, char **v)
{
    setvbuf(stdout, NULL, _IONBF, 0);
	evutil_socket_t listener;
    struct sockaddr_in sin;
    struct event_base *base;
    struct event *listener_event;

    base = event_base_new();
    if (!base)
        return -1; 

    sin.sin_family = AF_INET;
    sin.sin_addr.s_addr = 0;
    sin.sin_port = htons(9090);

    listener = socket(AF_INET, SOCK_STREAM, 0);
    evutil_make_socket_nonblocking(listener);

    int one = 1;
    setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

    if (bind(listener, (struct sockaddr*)&sin, sizeof(sin)) < 0) {
        perror("bind");
        return -1;
    }

    if (listen(listener, 16)<0) {
        perror("listen");
        return -1;
    }

    listener_event = event_new(base, listener, EV_READ|EV_PERSIST, do_accept, (void*)base);
    event_add(listener_event, NULL);
    event_base_dispatch(base);
    return 0;
}
