#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>

#include <event.h>
#include <event2/listener.h>
#include <event2/bufferevent_ssl.h>

int count =0;

static void ssl_readcb(struct bufferevent * bev, void * arg){
	struct evbuffer *input = bufferevent_get_input(bev);
	struct evbuffer *output = bufferevent_get_output(bev);
	if (count==0){
		evbuffer_add(output, "Username: ", sizeof("Username: "));
		count =1;
	}
    
    if (count==1){
		if(evbuffer_pullup(input, -1)=="admin"){
			count =2;
		}
		else {
			evbuffer_add(output, "WRONG! \n", sizeof("WRONG! \n"));
			count =0;
		}
	}
	
	if (count==2){
		evbuffer_add(output, "Password: ", sizeof("Password: "));
		count =3;
	}
	
	if (count==3){
		if(evbuffer_pullup(input, -1)=="admin"){
			count =4;
		}
		else {
			evbuffer_add(output, "WRONG! \n", sizeof("WRONG! \n"));
			count =0;
		}
	}
	
    if (count==4){
		evbuffer_add_buffer(output, input);
	}
    
    
}

static void ssl_acceptcb(struct evconnlistener *serv, int sock, struct sockaddr *sa, int sa_len, void *arg){
    struct event_base *evbase;
    struct bufferevent *bev;
    SSL_CTX *server_ctx;
    SSL *client_ctx;

    server_ctx = (SSL_CTX *)arg;
    client_ctx = SSL_new(server_ctx);
    evbase = evconnlistener_get_base(serv);

    bev =(struct bufferevent*) bufferevent_openssl_socket_new(evbase, sock, client_ctx,BUFFEREVENT_SSL_ACCEPTING,BEV_OPT_CLOSE_ON_FREE);

    bufferevent_enable(bev, EV_READ|EV_WRITE);
    bufferevent_setcb(bev, ssl_readcb, NULL, NULL, NULL);
}

static SSL_CTX * evssl_init(void){
	SSL_CTX  *server_ctx;

    /* Initialize the OpenSSL library */
    SSL_load_error_strings();
    SSL_library_init();
    /* We MUST have entropy, or else there's no point to crypto. */
    if (!RAND_poll())
        return NULL;

    server_ctx = SSL_CTX_new(SSLv23_server_method());

    if (! SSL_CTX_use_certificate_chain_file(server_ctx, "cert") ||
        ! SSL_CTX_use_PrivateKey_file(server_ctx, "pkey", SSL_FILETYPE_PEM)) {
        puts("Couldn't read 'pkey' or 'cert' file.  To generate a key\n"
           "and self-signed certificate, run:\n"
           "  openssl genrsa -out pkey 2048\n"
           "  openssl req -new -key pkey -out cert.req\n"
           "  openssl x509 -req -days 365 -in cert.req -signkey pkey -out cert");
        return NULL;
    }
    SSL_CTX_set_options(server_ctx, SSL_OP_NO_SSLv2);

    return server_ctx;
}

int main(int argc, char **argv){
    SSL_CTX *ctx;
    struct evconnlistener *listener;
    struct event_base *evbase;
    struct sockaddr_in sin;

    memset(&sin, 0, sizeof(sin));
    sin.sin_family = AF_INET;
    sin.sin_port = htons(9999);
    sin.sin_addr.s_addr = INADDR_ANY; 

    ctx = evssl_init();
    if (ctx == NULL)
        return 1;
    evbase = event_base_new();
    listener = evconnlistener_new_bind(evbase, ssl_acceptcb, (void *)ctx,LEV_OPT_CLOSE_ON_FREE | LEV_OPT_REUSEABLE, 1024,(struct sockaddr *)&sin, sizeof(sin));

    event_base_loop(evbase, 0);

    evconnlistener_free(listener);
    SSL_CTX_free(ctx);

    return 0;
}
