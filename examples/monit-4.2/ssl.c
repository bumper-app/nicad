/*
 * Copyright (C), 2000-2004 by the monit project group.
 * All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software Foundation,
 * Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA
 */

/* client socket clean up -> close */
/* close client socket */

#include <config.h>

#ifdef HAVE_STDIO_H
#include <stdio.h>
#endif

#ifdef HAVE_MEMORY_H
#include <memory.h>
#endif

#if TIME_WITH_SYS_TIME
# include <sys/time.h>
# include <time.h>
#else
# if HAVE_SYS_TIME_H
#  include <sys/time.h>
# else
#  include <time.h>
# endif
#endif

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_SOCKET_H
#include <sys/socket.h>
#endif

#ifdef HAVE_NETINET_IN_H
#include <netinet/in.h>
#endif

#ifdef HAVE_ARPA_INET_H
#include <arpa/inet.h>
#endif

#ifdef HAVE_NETDB_H
#include <netdb.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#ifdef HAVE_ERRNO_H
#include <errno.h>
#endif

#ifdef HAVE_STRING_H
#include <string.h>
#endif


#ifdef HAVE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/x509_vfy.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/rand.h>
#include <openssl/bio.h>
#endif

#include "monitor.h"
#include "net.h"
#include "ssl.h"

#ifdef HAVE_OPENSSL

/* Initialization code */
static int ssl_thread_start(void);
static int ssl_thread_stop(void);
static int unsigned long ssl_thread_id(void);
static void ssl_mutex_lock(int mode, int n, const char *file, int line);
static int ssl_entropy_start(void);
static int ssl_entropy_stop(void);

static pthread_mutex_t   ssl_mutex= PTHREAD_MUTEX_INITIALIZER;
static pthread_mutex_t *ssl_mutex_table;
static int ssl_initilized= FALSE;

/* Connection verification code */
static int verify_init(ssl_server_connection *);
static int verify_callback(int, X509_STORE_CTX *);
static int verify_callback_noclientcert(int, X509_STORE_CTX *);
static void verify_info(ssl_server_connection *);
static int check_preverify(X509_STORE_CTX *);

static int allow_self_certfication= FALSE;
static int allow_any_purpose= FALSE;

/* Error handling */
static int handle_connection_error(int, ssl_connection *, char *, int);
static void handle_ssl_error(char *);
#define SSLERROR ERR_error_string(ERR_get_error(),NULL)

#define ALLOW_ANY_PURPOSE

#endif

/**
 * Number of random bytes to obtain
 */
#define RANDOM_BYTES 1024

/**
 * The PRIMARY random device selected for seeding the PRNG. We use a
 * non-blocking pseudo random device, to generate pseudo entropy.
 */
#define URANDOM_DEVICE "/dev/urandom"

/**
 * If a non-blocking device is not found on the system a blocking
 * entropy producer is tried instead.
 */
#define RANDOM_DEVICE "/dev/random"


/**
 *  SSL Socket methods.
 *
 *  @author Christian Hopp <chopp@iei.tu-clausthal.de>
 *  @author Jan-Henrik Haukeland, <hauk@tildeslash.com>
 *
 *  @version \$Id: ssl.c,v 1.36 2004/02/18 22:31:42 chopp Exp $
 *
 *  @file
 */


  /*
   * For some of the code I was enlightened by:
   *
   * An Introduction to OpenSSL Programming, Part I of II
   *
   * by Eric Rescorla
   * Linux Journal 9/2001
   * http://www.linuxjournal.com/article.php?sid=4822
   */

/* ------------------------------------------------------------------ Public */


/**
 * Embeds a socket in a ssl connection.
 * @param socket the socket to be used.
 * @return The ssl connection or NULL if an error occured.
 */
int embed_ssl_socket (ssl_connection *ssl, int socket) {

#ifdef HAVE_OPENSSL

  int ssl_error;
  time_t ssl_time;
  
  if ( ssl == NULL ) {
    
    return FALSE;
    
  }
  
  if (!ssl_initilized) {
    
    start_ssl();

  }

  if ( socket >= 0 ) {
    
    ssl->socket= socket;

  } else {

    log("%s: embed_ssl_socket (): Socket error!\n", prog);
    goto sslerror;
  }

  if ((ssl->handler= SSL_new (ssl->ctx)) == NULL ) {

    handle_ssl_error("embed_ssl_socket()");
    log("%s: embed_ssl_socket (): Cannot initialize the SSL handler!\n", 
	  prog);
    goto sslerror;

  }

  set_noblock(ssl->socket);

  if((ssl->socket_bio= BIO_new_socket(ssl->socket, BIO_NOCLOSE)) == NULL) {

    handle_ssl_error("embed_ssl_socket()");
    log("%s: embed_ssl_socket (): Cannot generate IO buffer!\n", prog);
    goto sslerror;

  }

  SSL_set_bio(ssl->handler, ssl->socket_bio, ssl->socket_bio);

  ssl_time=time(NULL);

  while((ssl_error= SSL_connect (ssl->handler)) < 0) {

    if((time(NULL)-ssl_time) > SSL_TIMEOUT) {

      log("%s: embed_ssl_socket (): SSL service timeout!\n",
	    prog);
      goto sslerror;
    
    }

    if (!handle_connection_error(ssl_error, ssl, "embed_ssl_socket()", SSL_TIMEOUT)) {

      goto sslerror;

    }

    if (!BIO_should_retry(ssl->socket_bio)) {

      goto sslerror;
      
    }


  }

  ssl->cipher= (char *) SSL_get_cipher(ssl->handler);

  if (! update_ssl_cert_data(ssl)) {

    log("%s: embed_ssl_socket (): Cannot get the SSL server certificate!\n", 
	  prog);
    goto sslerror;

  }

  return TRUE;

 sslerror:

  cleanup_ssl_socket(ssl);
  return FALSE;

#else

  return FALSE;

#endif
} 


/**
 * Updates some data in the ssl connection
 * @param ssl reference to ssl connection 
 * @return TRUE, if not successful FALSE 
 */
int update_ssl_cert_data(ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  unsigned char md5[EVP_MAX_MD_SIZE];

  ASSERT(ssl);
  
  if ((ssl->cert = SSL_get_peer_certificate (ssl->handler)) == NULL) {

    return FALSE;

  } 

  /* RATS: ignore */ /* buffer is dynamically allocated */
  ssl->cert_issuer=X509_NAME_oneline (X509_get_issuer_name (ssl->cert),0,0);
  /* RATS: ignore */ /* buffer is dynamically allocated */
  ssl->cert_subject=X509_NAME_oneline (X509_get_subject_name (ssl->cert),0,0);
  X509_digest(ssl->cert,EVP_md5(), md5, &ssl->cert_md5_len);
  ssl->cert_md5= (unsigned char *) xstrdup((char *) md5);

  return TRUE;

#else

  return FALSE;

#endif

}


/**
 * Compare certificate with given md5 sum
 * @param ssl reference to ssl connection 
 * @param md5sum string of the md5sum to test against
 * @return TRUE, if sums do not match FALSE 
 */
int check_ssl_md5sum(ssl_connection *ssl, char *md5sum) {

#ifdef HAVE_OPENSSL
  unsigned int i= 0;

  ASSERT(md5sum);

  while (( i < ssl->cert_md5_len ) && 
	 ( md5sum[2*i] != '\0' ) && 
	 ( md5sum[2*i+1] != '\0' )) {

    unsigned char c= 
      (md5sum[2*i] > 57 ? md5sum[2*i] - 87 : md5sum[2*i] - 48) * 0x10+
      (md5sum[2*i+1] > 57 ? md5sum[2*i+1] - 87 : md5sum[2*i+1] - 48);

    if ( c!=ssl->cert_md5[i] ) {

      return FALSE;
      
    }

    i ++;

  }

#endif

  return TRUE;

}


/**
 * Open a socket against hostname:port with the given protocol.
 * This socket is sent through a ssl connection.
 * The protocol is normaly either SOCK_STREAM or SOCK_DGRAM.
 * @param hostname The host to open a socket at
 * @param port The port number to connect to
 * @param protocol Socket protocol to use (SOCK_STREAM|SOCK_DGRAM)
 * @param sslversion Version of the ssl layer auto, SSLv3, SSLv2 or TLS
 * @return The ssl connection or NULL if an error occured.
 */
ssl_connection *create_ssl_socket(char *hostname, int port, int protocol, int sslversion) { 

#ifdef HAVE_OPENSSL

  int socket;
  ssl_connection *ssl = new_ssl_connection(NULL, sslversion);

  ASSERT(hostname);

  if (!ssl_initilized) {

    start_ssl();

  }

  if((socket= create_socket(hostname, port, protocol, NET_TIMEOUT)) == -1) {

    log("%s: create_ssl_socket(): Cannot connect!\n", prog);
    goto sslerror;

  }

  if (! embed_ssl_socket(ssl, socket)) {

    goto sslerror;

  }

  return ssl;

 sslerror:

  return NULL;

#else

  return NULL;

#endif

}


/**
 * Closes a ssl connection (ssl socket + net socket)
 * @param ssl ssl connection
 * @return TRUE, or FALSE if an error has occured.
 */
int close_ssl_socket(ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  int error;

  if(ssl == NULL) {

    return FALSE;

  }

  if (! (error= SSL_shutdown (ssl->handler))) {

    shutdown(ssl->socket,1 );
    error= SSL_shutdown (ssl->handler);

  }

  close_socket(ssl->socket);
  cleanup_ssl_socket(ssl);

  if ( error<=0 ) {
   
    return FALSE;

  } else {

    return TRUE;

  }

#else

  return FALSE;

#endif

}


/**
 * Garbage collection for non reusabe parts of the ssl connection
 * @param ssl ssl connection
 * @return TRUE, or FALSE if an error has occured.
 */
int cleanup_ssl_socket(ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  if(ssl==NULL) {

    return FALSE;

  }

  if (ssl->cert != NULL) {

    X509_free(ssl->cert);
    ssl->cert= NULL;

  }

  if (ssl->handler != NULL) {
    
    SSL_free(ssl->handler);
    ssl->handler= NULL;

  }

  if (ssl->socket_bio != NULL) {
    
    /* 
     * no BIO_free(ssl->socket_bio); necessary, because BIO is freed
     * by ssl->handler
     */

    ssl->socket_bio= NULL;

  }

  if (ssl->cert_issuer != NULL) {

    FREE(ssl->cert_issuer);
    ssl->cert_issuer= NULL;

  }

  if (ssl->cert_subject != NULL) {

    FREE(ssl->cert_subject);
    ssl->cert_subject= NULL;

  }

  if (ssl->cert_md5 != NULL) {

    FREE(ssl->cert_md5);
    ssl->cert_md5= NULL;

  }

  if(ssl->clientpemfile!=NULL) {

    FREE(ssl->clientpemfile);
    ssl->clientpemfile= NULL;

  }

  return TRUE;

#else

  return FALSE;

#endif

}


/**
 * Garbage collection for non-reusable parts a ssl connection
 * @param ssl ssl connection
 * @return TRUE, or FALSE if an error has occured.
 */
int delete_ssl_socket(ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  if(ssl==NULL) {

    return FALSE;

  }

  cleanup_ssl_socket(ssl);

  if((ssl->ctx != NULL) && (! ssl->accepted)) {

    SSL_CTX_free(ssl->ctx);
    ssl->ctx=NULL;
    
  } else {

    ssl->ctx=NULL;
    
  }

  FREE(ssl);

  ssl=NULL;

  return TRUE;

#else

  return TRUE;

#endif

}


/**
 * Initializes a ssl connection for server use.
 * @param pemfilename Filename for the key/cert file
 * @return An ssl connection, or NULL if an error occured.
 */
ssl_server_connection *init_ssl_server (char *pemfile, char *clientpemfile) {

#ifdef HAVE_OPENSSL

  ssl_server_connection *ssl_server = new_ssl_server_connection(pemfile, 
								clientpemfile);

  ASSERT(pemfile);

  if (!ssl_initilized) {

    start_ssl();

  }
  
  if ((ssl_server->method= SSLv23_server_method()) == NULL ) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server (): Cannot initialize the SSL method!\n", prog);
    goto sslerror;

  }

  if ((ssl_server->ctx= SSL_CTX_new(ssl_server->method)) == NULL ) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server (): Cannot initialize SSL server"
	  " certificate handler!\n"
	  , prog);
    goto sslerror;

  }

  if (SSL_CTX_use_certificate_file(ssl_server->ctx, pemfile, 
				   SSL_FILETYPE_PEM) <= 0) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server(): Cannot initialize SSL server"
	  " certificate!\n", prog);
    goto sslerror;

  }

  if (SSL_CTX_use_PrivateKey_file(ssl_server->ctx, pemfile, 
				  SSL_FILETYPE_PEM) <= 0) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server(): Cannot initialize SSL server" 
	  " private key!\n", prog);
    goto sslerror;

  }

  if (!SSL_CTX_check_private_key(ssl_server->ctx)) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server(): The private key does not match the"
	  " certificate public key!\n", prog);
    goto sslerror;

  }

  /*
   * We need this to force transmission of client certs
   */
  if (!verify_init(ssl_server)) {

    handle_ssl_error("init_ssl_server()");
    log("%s: init_ssl_server(): Verification engine was not"
	  " properly initilized!\n", prog);
    goto sslerror;

  }

  if (ssl_server->clientpemfile != NULL) {

    verify_info(ssl_server);

  }

  return ssl_server;

 sslerror:

  cleanup_ssl_server_socket(ssl_server);
  return NULL;

#else

  return NULL;

#endif

}


/**
 * Creates a server socket (SOCK_STREAM type) and binds it to the
 * specified local port number.  The socket get a ssl layer for
 * data transmission.
 * @param pemfilename Filename for the key/cert file
 * @param port The localhost port number to open
 * @param backlog The maximum queue length for incomming connections
 * @param bindAddr the local address the server will bind to
 * @return An ssl connection ready for accept, or NULL if an error occured.
 */
ssl_server_connection *create_ssl_server_socket(char *pemfile, int port, 
						int backlog, 
						char *bindAddr,
						char *clientpemfile) {

#ifdef HAVE_OPENSSL
  
  int socket;
  ssl_server_connection *ssl_server;

  ASSERT(pemfile);
  ASSERT(bindAddr);

  if (!ssl_initilized) {

    start_ssl();

  }

  if ((socket= create_server_socket(port, backlog, bindAddr)) == -1) {

    log("%s: create_ssl_server_socket(): Cannot connect!\n", prog);
    goto sslerror;

  }

  if (( ssl_server= init_ssl_server(pemfile, clientpemfile)) == NULL) {

    goto sslerror;

  }
  
  ssl_server->server_socket=socket;

  return ssl_server;

 sslerror:

  return NULL;

#else

  return FALSE;

#endif 
 
}


/**
 * Closes a ssl server connection (ssl socket + net socket)
 * @param ssl ssl connection
 * @return TRUE, or FALSE if an error has occured.
 */
int close_ssl_server_socket(ssl_server_connection *ssl_server) {

#ifdef HAVE_OPENSSL

  if (ssl_server==NULL) {

    return FALSE;

  }

  close(ssl_server->server_socket);
  cleanup_ssl_server_socket(ssl_server);

  return TRUE;

#else
  
  return FALSE;

#endif
}


/**
 * Garbage collection for a SSL server connection. 
 * @param ssl_server data for ssl server connection
 * @return TRUE, or FALSE if an error has occured.
 */
int cleanup_ssl_server_socket(ssl_server_connection *ssl_server) {

#ifdef HAVE_OPENSSL

  ssl_connection *ssl;

  if(ssl_server==NULL) {

    return FALSE;

  }

  if(ssl_server->pemfile!=NULL) {

    FREE(ssl_server->pemfile);
    ssl_server->pemfile= NULL;

  }

  if(ssl_server->clientpemfile!=NULL) {

    FREE(ssl_server->clientpemfile);
    ssl_server->clientpemfile= NULL;

  }

  while (ssl_server->ssl_conn_list!=NULL) {
    
    ssl = ssl_server->ssl_conn_list;

    ssl_server->ssl_conn_list=ssl_server->ssl_conn_list->next;
    
    close_accepted_ssl_socket(ssl_server, ssl);
    
  }

  return TRUE;

#else

  return TRUE;

#endif

}


/**
 * Deletes a SSL server connection. 
 * @param ssl_server data for ssl server connection
 * @return TRUE, or FALSE if an error has occured.
 */
int delete_ssl_server_socket(ssl_server_connection *ssl_server) {

#ifdef HAVE_OPENSSL
  
  if(ssl_server==NULL) {

    return FALSE;

  }

  cleanup_ssl_server_socket(ssl_server);

  if (ssl_server->ctx != NULL) {

    SSL_CTX_free(ssl_server->ctx);

  }

  FREE(ssl_server);
  ssl_server=NULL;

  return TRUE;

#else

  return TRUE;

#endif

}


/**
 * Inserts an SSL connection in the connection list of a server.
 * @param ssl_server data for ssl server connection
 * @return new SSL connection for the connection, or NULL if failed
 */
ssl_connection *insert_accepted_ssl_socket(ssl_server_connection *ssl_server) {

#ifdef HAVE_OPENSSL

  ssl_connection *ssl = (ssl_connection *) NEW(ssl);

  ASSERT(ssl_server);

  if (!ssl_initilized) {

    start_ssl();

  }

  ssl->method= NULL;
  ssl->handler= NULL;
  ssl->cert= NULL;
  ssl->cipher= NULL;
  ssl->socket= 0;
  ssl->next= NULL;
  ssl->accepted= FALSE;
  ssl->cert_md5= NULL;
  ssl->cert_md5_len= 0;
  ssl->clientpemfile= NULL;

  if(ssl_server->clientpemfile!=NULL) {

    ssl->clientpemfile= xstrdup(ssl_server->clientpemfile);

  }

  if(( ssl_server == NULL ) || (ssl == NULL)) {

    return NULL;

  }

  LOCK(ssl_mutex);

  ssl->prev=NULL;
  ssl->next=ssl_server->ssl_conn_list;

  if( ssl->next != NULL ) {

    ssl->next->prev=ssl;

  }

  END_LOCK;

  ssl_server->ssl_conn_list=ssl;
  ssl->ctx=ssl_server->ctx;
  ssl->accepted=TRUE;

  return ssl;

#else

  return NULL;

#endif

}


/**
 * Closes an accepted SSL server connection and deletes it form the 
 * connection list. 
 * @param ssl_server data for ssl server connection
 * @param ssl data the connection to be deleted
 * @return TRUE, or FALSE if an error has occured.
 */
int close_accepted_ssl_socket(ssl_server_connection *ssl_server, 
			      ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  int return_value= TRUE;

  if ((ssl == NULL) ||  (ssl_server == NULL)) {

    return FALSE;

  }

  close_socket(ssl->socket);
  
  if (! delete_accepted_ssl_socket(ssl_server,  ssl)) {

    return_value= FALSE;

  } 

  return return_value;

#else

  return FALSE;

#endif

}


/**
 * Deletes an accepted SSL server connection from the connection
 * list. 
 * @param ssl_server data for ssl server connection
 * @param ssl data the connection to be deleted
 * @return TRUE, or FALSE if an error has occured.
 */
int delete_accepted_ssl_socket (ssl_server_connection *ssl_server, 
				ssl_connection *ssl) {

#ifdef HAVE_OPENSSL

  int return_value= TRUE;

  if ((ssl == NULL) ||  (ssl_server == NULL)) {
    return FALSE;
  }

  LOCK(ssl_mutex);

  if ( ssl->prev == NULL ) {

    ssl_server->ssl_conn_list=ssl->next;

  } else {

    ssl->prev->next=ssl->next;

  }

  END_LOCK;  

  if(! cleanup_ssl_socket(ssl)) {

    return_value= FALSE;

  }
     
  if (! delete_ssl_socket(ssl)) {

    return_value= FALSE;

  }

  return return_value;

#else

  return FALSE;

#endif

}


/**
 * Embeds an accepted server socket in an existing ssl connection.
 * @param ssl ssl connection
 * @param socket the socket to be used.
 * @return TRUE, or FALSE if an error has occured.
 */
int embed_accepted_ssl_socket(ssl_connection *ssl, int socket) {

#ifdef HAVE_OPENSSL

  int ssl_error;
  time_t ssl_time;

  ASSERT(ssl);
  
  ssl->socket=socket;

  if(!ssl_initilized) {

    start_ssl();

  }

  if((ssl->handler= SSL_new(ssl->ctx)) == NULL) { 
 
    handle_ssl_error("embed_accepted_ssl_socket()");
    log("%s: embed_accepted_ssl_socket(): Cannot initialize the"
	  " SSL handler!\n", prog); 
    goto sslerror;

  } 

  if(socket < 0) {

    log("Socket error!\n");
    goto sslerror;

  }

  set_noblock(ssl->socket);

  if((ssl->socket_bio= BIO_new_socket(ssl->socket, BIO_NOCLOSE)) == NULL) {

    handle_ssl_error("embed_accepted_ssl_socket()");
    log("%s: embed_accepted_ssl_socket(): Cannot generate IO buffer!\n", 
	  prog);
    goto sslerror;

  }

  SSL_set_bio(ssl->handler, ssl->socket_bio, ssl->socket_bio);

  ssl_time= time(NULL);
  
  while((ssl_error= SSL_accept(ssl->handler)) < 0) {

    if((time(NULL)-ssl_time) > SSL_TIMEOUT) {

      log("%s: embed_accepted_ssl_socket(): SSL service timeout!\n", 
	    prog);
      goto sslerror;
      
    }

    if (!handle_connection_error(ssl_error, ssl, 
				 "embed_accepted_ssl_socket()", SSL_TIMEOUT)) {

      goto sslerror;

    }

    if (!BIO_should_retry(ssl->socket_bio)) {

      goto sslerror;
      
    }

  }

  ssl->cipher= (char *) SSL_get_cipher(ssl->handler);

  if(!update_ssl_cert_data(ssl) && (ssl->clientpemfile != NULL)) {

    log("%s: The client did not supply a required client certificate!\n", 
	  prog);
    goto sslerror;

  }

  if (SSL_get_verify_result(ssl->handler)>0) {

    log("%s: Verification of the certificate has failed!\n", 
	  prog);
    goto sslerror;

  }

  return TRUE;

 sslerror:

  return FALSE;

#else
  
  return FALSE;

#endif

}


/**
 * Do "accept" for a ssl server socket 
 * @param ssl ssl connection
 * @return the ssl_connection of the socket, NULL in case of an error
 */
ssl_connection *accept_ssl_socket(ssl_server_connection *ssl_server) {

#ifdef HAVE_OPENSSL

  int no_crypt_socket;
  int len= sizeof(struct sockaddr_in);
  struct sockaddr_in in;

  ASSERT(ssl_server);
  
  no_crypt_socket= accept(ssl_server->server_socket, (struct sockaddr*)&in, 
			  &len);

  if(no_crypt_socket >= 0) {
    
    ssl_connection *ssl = insert_accepted_ssl_socket(ssl_server);
    
    if(ssl == NULL) {

      return NULL;

    }

    if(embed_accepted_ssl_socket(ssl, no_crypt_socket)) {

      return ssl;

    } else {

      close_accepted_ssl_socket(ssl_server, ssl);
      return NULL;

    }

  } else {

    return NULL;

  }

#else
  
  return NULL;

#endif

}


/**
 * Send data package though the ssl connection 
 * @param ssl ssl connection
 * @param buffer array containg the data
 * @param len size of the data container
 * @return number of bytes transmitted, -1 in case of an error
 */
int send_ssl_socket(ssl_connection *ssl, void *buffer, int len) {

#ifdef HAVE_OPENSSL
  int n= 0;

  ASSERT(ssl);

  do {
    n= SSL_write(ssl->handler, buffer, len);
  } while(n <= 0 &&
	  BIO_should_retry(ssl->socket_bio) &&
	  can_write(ssl->socket, 5));
  
  if(n <= 0) {
    return -1;
  }

  return n;

#else

  return -1;
  
#endif

}


/**
 * Receive data package though the ssl connection 
 * @param ssl ssl connection
 * @param buffer array to hold the data
 * @param len size of the data container
 * @param timeout Seconds to wait for data to be available
 * @return number of bytes transmitted, -1 in case of an error
 */
int recv_ssl_socket(ssl_connection *ssl, void *buffer, int len, int timeout) {

#ifdef HAVE_OPENSSL
  int n= 0;

  ASSERT(ssl);

  do {
    n= SSL_read(ssl->handler, buffer, len);
  } while(n <= 0 &&
	  BIO_should_retry(ssl->socket_bio) &&
	  can_read(ssl->socket, timeout));
  
  if(n <= 0) {
    return -1;
  }

  return n;
  
#else

  return -1;
  
#endif

}


/**
 * Start SSL support library. It has to be run before the SSL support
 * can be used.
 * @return TRUE, or FALSE if an error has occured.
 */
int start_ssl() {

#ifdef HAVE_OPENSSL

  if (! ssl_initilized ) {

    ssl_initilized=TRUE;
    ERR_load_crypto_strings();
    return (ssl_thread_start() && SSL_library_init() && ssl_entropy_start());

  } else {

    return TRUE;

  }

#else

  return FALSE;

#endif

}


/**
 * Stop SSL support library
 * @return TRUE, or FALSE if an error has occured.
 */
int stop_ssl() {

#ifdef HAVE_OPENSSL

  if ( ssl_initilized ) {

    ssl_initilized=FALSE;
    ERR_free_strings();
    return (ssl_thread_stop() && ssl_entropy_stop());

  } else {

    return TRUE;

  }

#else

  return FALSE;

#endif

}

/**
 * Configures the ssl engine
 */

void config_ssl(int conf_allow_self_cert) {

#ifdef HAVE_OPENSSL

  allow_self_certfication= conf_allow_self_cert;

#endif

  return;

}

/**
 * Generate a new ssl connection
 * @return ssl connection container
 */
ssl_connection *new_ssl_connection(char *clientpemfile, int sslversion) {

#ifdef HAVE_OPENSSL

  ssl_connection *ssl = (ssl_connection *) NEW(ssl);

  if (!ssl_initilized) {

    start_ssl();

  }

  ssl->socket_bio= NULL; 
  ssl->handler= NULL;
  ssl->cert= NULL;
  ssl->cipher= NULL;
  ssl->socket= 0;
  ssl->next = NULL;
  ssl->accepted = FALSE;
  ssl->cert_md5 = NULL;
  ssl->cert_md5_len = 0;
  
  if(clientpemfile!=NULL) {

    ssl->clientpemfile= xstrdup(clientpemfile);

  } else {

    ssl->clientpemfile= NULL;

  }

  switch (sslversion) {

  case SSL_VERSION_AUTO:

    ssl->method = SSLv23_client_method();
    break;

  case SSL_VERSION_SSLV2:

    ssl->method = SSLv2_client_method();
    break;

  case SSL_VERSION_SSLV3:

    ssl->method = SSLv3_client_method();
    break;

  case SSL_VERSION_TLS:

    ssl->method = TLSv1_client_method();
    break;

  default:

    log("%s: new_ssl_connection(): Unknown SSL version!\n", prog);
    goto sslerror;

  }

  if (ssl->method == NULL ) {

    handle_ssl_error("new_ssl_connection()");
    log("%s: new_ssl_connection(): Cannot initilize SSL method!\n", prog);
    goto sslerror;

  } 

  if ((ssl->ctx= SSL_CTX_new (ssl->method)) == NULL ) {

    handle_ssl_error("new_ssl_connection()");
    log("%s: new_ssl_connection(): Cannot initilize SSL server certificate"
	  " handler!\n", prog);
    goto sslerror;

  }

  if ( ssl->clientpemfile!=NULL ) {

    if (SSL_CTX_use_certificate_file(ssl->ctx, ssl->clientpemfile, 
				     SSL_FILETYPE_PEM) <= 0) {

      handle_ssl_error("new_ssl_connection()");
      log("%s: new_ssl_connection(): Cannot initilize SSL server"
	    " certificate!\n", prog);
      goto sslerror;
      
    }

    if (SSL_CTX_use_PrivateKey_file(ssl->ctx, ssl->clientpemfile, 
				    SSL_FILETYPE_PEM) <= 0) {

      handle_ssl_error("new_ssl_connection()");
      log("%s: new_ssl_connection(): Cannot initilize SSL server"
	    " private key!\n", prog);
      goto sslerror;

    }

    if (!SSL_CTX_check_private_key(ssl->ctx)) {

      handle_ssl_error("new_ssl_connection()");
      log("%s: new_ssl_connection(): Private key does not match the"
	    " certificate public key!\n", 
	    prog);
      goto sslerror;

    }

  }


  return ssl;

 sslerror:

  delete_ssl_socket(ssl);

  return NULL;

#else

  return NULL;

#endif

}


/**
 * Generate a new ssl server connection
 * @return ssl server connection container
 */
ssl_server_connection *new_ssl_server_connection(char * pemfile, 
						 char * clientpemfile) {

#ifdef HAVE_OPENSSL

  ssl_server_connection *ssl_server = 
    (ssl_server_connection *) NEW(ssl_server);

  ASSERT(pemfile);

  if (!ssl_initilized) {

    start_ssl();

  }

  ssl_server->ctx= NULL;
  ssl_server->method= NULL;
  ssl_server->server_socket= 0;
  ssl_server->ssl_conn_list = NULL;
  ssl_server->pemfile=xstrdup(pemfile);

  if(clientpemfile != NULL) {
    
    ssl_server->clientpemfile= xstrdup(clientpemfile);

  } else {

    ssl_server->clientpemfile= NULL;

  }
  
  return ssl_server;

#else

  return NULL;

#endif

}


/**
 * Checks if openssl is compiled in
 * @return TRUE if installed
 */
int have_ssl(void) {

#ifdef HAVE_OPENSSL

  return HAVE_OPENSSL;

#else

  return FALSE;

#endif

}

/* ----------------------------------------------------------------- Private */

#ifdef HAVE_OPENSSL

/**
 * Init verification of transmitted client certs
 */
static int verify_init(ssl_server_connection *ssl_server) {

  struct stat stat_buf;

  if (ssl_server->clientpemfile==NULL) {

    allow_any_purpose= TRUE;
    SSL_CTX_set_verify(ssl_server->ctx, SSL_VERIFY_PEER , 
		       verify_callback_noclientcert);
    goto end_success; /* No verification, but we have to call the callback! */

  }

  if ( -1 == stat(ssl_server->clientpemfile, &stat_buf )) {

    log("%s: verify_init(): Cannot stat the SSL pem path '%s' -- %s\n",
	  prog, Run.httpsslclientpem, STRERROR);

    goto end_error;

  }
  
  if (S_ISDIR(stat_buf.st_mode)) {

    if (!SSL_CTX_load_verify_locations(ssl_server->ctx, NULL ,
				       ssl_server->clientpemfile)) {

      handle_ssl_error("verify_init()");
      log("%s: verify_init(): Error setting verify directory to %s\n", 
	    Run.httpsslclientpem);

      goto end_error;

    }

    log("%s: verify_init(): Loaded SSL client pem directory '%s'\n", 
	prog, ssl_server->clientpemfile);

    /* Monits server cert for cli support ! */

    if(!SSL_CTX_load_verify_locations(ssl_server->ctx, ssl_server->pemfile, 
				      NULL)) {

      handle_ssl_error("verify_init()");
      log("%s: verify_init(): Error loading verify certificates from %s\n",
	    prog, ssl_server->pemfile);

      goto end_error;

    }

    log("%s: verify_init(): Loaded monit's SSL pem server file '%s'\n", 
	prog, ssl_server->pemfile);

  } else if (S_ISREG(stat_buf.st_mode)) {

    if(!SSL_CTX_load_verify_locations(ssl_server->ctx, 
				      ssl_server->clientpemfile, 
				      NULL)) {

      handle_ssl_error("verify_init()");
      log("%s: verify_init(): Error loading verify certificates from %s\n",
	    prog, Run.httpsslclientpem);

      goto end_error;

    }

    log("%s: verify_init(): Loaded SSL pem client file '%s'\n", 
	prog, ssl_server->clientpemfile);

    /* Monits server cert for cli support ! */

    if(!SSL_CTX_load_verify_locations(ssl_server->ctx, ssl_server->pemfile, 
				      NULL)) {

      handle_ssl_error("verify_init()");
      log("%s: verify_init(): Error loading verify certificates from %s\n",
	    prog, ssl_server->pemfile);

      goto end_error;

    }

    log("%s: verify_init(): Loaded monit's SSL pem server file '%s'\n", 
	prog, ssl_server->pemfile);

    SSL_CTX_set_client_CA_list(ssl_server->ctx,
			       SSL_load_client_CA_file(ssl_server->clientpemfile));

  } else {

    log("%s: verify_init(): SSL client pem path is no file or directory %s\n",
	  prog, ssl_server->clientpemfile);

    goto end_error;

  }

  allow_any_purpose= FALSE;
  SSL_CTX_set_verify(ssl_server->ctx, SSL_VERIFY_PEER , verify_callback);

 end_success:
  
  return TRUE;

 end_error:

  return FALSE;
}


/**
 * Check the transmitted client certs and a compare with client cert database
 */
static int verify_callback(int preverify_ok, X509_STORE_CTX *ctx) {

  char subject[STRLEN];
  X509_OBJECT found_cert;

  /* RATS: ignore */ /* buffer size is limited by STRLEN */
  X509_NAME_oneline(X509_get_subject_name(ctx->current_cert), subject,
                    STRLEN-1);

  if(!preverify_ok) {

    if (!check_preverify(ctx)) {

      goto reject;

    }

  }

  if(ctx->error_depth==0 &&
     X509_STORE_get_by_subject(ctx, X509_LU_X509,
			       X509_get_subject_name(ctx->current_cert), 
			       &found_cert)!=1) {

    handle_ssl_error("verify_callback()");
    log("%s: verify_callback(): SSL connection rejected. No matching "
	  "certificate found.", prog);

    goto reject; 

  }

  return 1; 

 reject:
  return 0; 

}

/**
 * Check the transmitted client certs and a do NOT compare with client cert 
 * database
 */
static int verify_callback_noclientcert(int preverify_ok, 
					X509_STORE_CTX *ctx) {

  char subject[STRLEN];

  /* RATS: ignore */ /* buffer size is limited by STRLEN-1 */
  X509_NAME_oneline(X509_get_subject_name(ctx->current_cert), subject,
                    STRLEN-1);

  if(!preverify_ok) {

    if (!check_preverify(ctx)) {

      goto reject;

    }

  }

  return 1; 

 reject:
  return 0; 

}

/**
 * Analyse errors found before actual verification
 * @return TRUE if successful
 */
static int check_preverify(X509_STORE_CTX *ctx) {

  if ((ctx->error != X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT) &&
      (ctx->error != X509_V_ERR_INVALID_PURPOSE)) {

    /* Remote site specified a certificate, but it's not correct */
    
    log("%s: check_preverify(): SSL connection rejected because"
	  " certificate verification has failed -- Error %i\n", 
	  prog, ctx->error);
    return FALSE; /* Reject connection */

  } 


  if(allow_self_certfication && 
     (ctx->error == X509_V_ERR_DEPTH_ZERO_SELF_SIGNED_CERT)) {
    
    /* Let's accept self signed certs for the moment! */
    
    log("%s: check_preverify(): SSL connection accepted with"
	" self signed certificate!\n", prog);

    ctx->error=0;
    return TRUE;

  } 

  if(allow_any_purpose && 
     (ctx->error == X509_V_ERR_INVALID_PURPOSE)) {
    
    /* Let's accept any purpose certs for the moment! */
    
    log("%s: check_preverify(): SSL connection accepted "
	"with invalid purpose!\n", prog);

    ctx->error=0;
    return TRUE;

  } 
    
  log("%s: check_preverify(): SSL connection rejected because "
	"certificate verification has failed -- Error %i!\n", 
	prog, ctx->error);
  return FALSE; /* Reject connection */


}

/**
 * Print info about the verification engine
 */
static void verify_info(ssl_server_connection *ssl_server) {
  
  STACK_OF(X509_NAME) *stack;
  
  stack=SSL_CTX_get_client_CA_list(ssl_server->ctx);
  log("%s: verify_info(): Found %d client certificates\n", prog, 
      sk_X509_NAME_num(stack));
  
}


/**
 * Helper function for the SSL threadding support
 * @return current thread number
 */
static int unsigned long ssl_thread_id(void) {

  return ((unsigned long) pthread_self());

}


/**
 * Helper function for the SSL threadding support
 */
static void ssl_mutex_lock(int mode, int n, const char *file, int line) {


  if(mode & CRYPTO_LOCK) {
    
    pthread_mutex_lock( & ssl_mutex_table[n]);
    
  } else {

    pthread_mutex_unlock( & ssl_mutex_table[n]);
    
  }
  
}


/**
 * Initialize threadding support for the SSL library
 * @return TRUE if successful, FALSE if failed
 */
static int ssl_thread_start(void) {

  int i;
  ssl_mutex_table= xcalloc(CRYPTO_num_locks(), sizeof(pthread_mutex_t));
  for(i= 0; i < CRYPTO_num_locks(); i++)
      pthread_mutex_init(&ssl_mutex_table[i], NULL);
  CRYPTO_set_id_callback(ssl_thread_id);
  CRYPTO_set_locking_callback(ssl_mutex_lock);

  return TRUE;

}


/**
 * Garbage colletion for the SSL threadding support
 * @return TRUE if successful, FALSE if failed
 */
static int ssl_thread_stop(void) {

  int i;
  CRYPTO_set_id_callback(NULL);
  CRYPTO_set_locking_callback(NULL);
  for(i= 0; i < CRYPTO_num_locks(); i++)
      pthread_mutex_destroy(&ssl_mutex_table[i]);
  free(ssl_mutex_table);
  ssl_mutex_table= NULL;

  return TRUE;

}


/**
 * Start entropy gathering
 * @return TRUE if successful, FALSE if failed
 */
static int ssl_entropy_start(void) {

  if(exist_file(URANDOM_DEVICE)) {

    return(RAND_load_file(URANDOM_DEVICE, RANDOM_BYTES)==RANDOM_BYTES);

  } else if(exist_file(RANDOM_DEVICE)) {

    fprintf(stdout, "Gathering entropy from the random device. Please wait\n");
    fflush(stdout);

    return(RAND_load_file(RANDOM_DEVICE, RANDOM_BYTES)==RANDOM_BYTES);

  }

  return FALSE;

}


/**
 * Stop entropy gathering
 * @return TRUE if successful, FALSE if failed
 */
static int ssl_entropy_stop(void) {

  RAND_cleanup();

  return TRUE;

}


/**
 * Handle errors during read, write, connect and accept
 * @return TRUE if non fatal, FALSE if non fatal and retry
 */
static int handle_connection_error(int code, ssl_connection *ssl, 
				   char *operation, int timeout) {

  int ssl_error= 0;

  switch ((ssl_error= SSL_get_error(ssl->handler, code))) {
    
  case SSL_ERROR_WANT_READ:
      if (can_read(ssl->socket, timeout))
          return TRUE;
      else
          log("%s: %s: Openssl read timeout error!\n", prog, operation);
          return FALSE;
      
  case SSL_ERROR_WANT_WRITE:
      if (can_read(ssl->socket, timeout))
          return TRUE;
      else
          log("%s: %s: Openssl write timeout error!\n", prog, operation);
          return FALSE;
    
  case SSL_ERROR_SYSCALL:
    log("%s: %s: Openssl syscall error: %s!\n", prog, operation, 
	  STRERROR);
    return FALSE;

  case SSL_ERROR_SSL:
    handle_ssl_error(operation);
    return FALSE;
      
  default:
    log("%s: %s: Openssl error!\n", prog, operation);

  }

  return FALSE;
}


/**
 * Handle errors of arbitrary SSL calls
 */
static void handle_ssl_error(char *operation) {

  log("%s: %s: Openssl engine error: %s\n", prog, operation, SSLERROR);

}

#endif
