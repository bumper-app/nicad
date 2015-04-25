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

#ifndef SSL_H
#define SSL_H

#include <config.h>

#ifdef HAVE_OPENSSL
#include <openssl/crypto.h>
#include <openssl/x509.h>
#include <openssl/pem.h>
#include <openssl/ssl.h>
#include <openssl/err.h>
#endif

#define SSL_VERSION_AUTO       0
#define SSL_VERSION_SSLV2      1
#define SSL_VERSION_SSLV3      2
#define SSL_VERSION_TLS        3

#ifdef HAVE_OPENSSL
typedef struct my_ssl_connection {

  int           socket;
  int           accepted;

  SSL*          handler;
  SSL_CTX *     ctx;
  X509 *        cert;
  SSL_METHOD *  method;
  BIO *         socket_bio;

  const char *  cipher;

  char *        cert_subject;
  char *        cert_issuer;
  unsigned char * cert_md5;
  unsigned int  cert_md5_len;

  char          * clientpemfile;

  struct my_ssl_connection *prev;
  struct my_ssl_connection *next;
  
} ssl_connection;


typedef struct my_ssl_server_connection {

  int           server_socket;

  SSL_METHOD *  method;
  SSL_CTX *     ctx;

  char          * pemfile;
  char          * clientpemfile;


  ssl_connection *ssl_conn_list;
  
} ssl_server_connection;

#else

typedef  void ssl_connection;
typedef  void ssl_server_connection;

#endif


ssl_connection * new_ssl_connection(char *, int);
ssl_server_connection * new_ssl_server_connection(char *, char *);

ssl_connection * create_ssl_socket(char *, int, int, int);
int embed_ssl_socket (ssl_connection *, int);

ssl_server_connection * init_ssl_server (char *, char *);
ssl_server_connection * create_ssl_server_socket(char *, int, int, char *, char *);
int embed_accepted_ssl_socket(ssl_connection *, int);
ssl_connection *  accept_ssl_socket(ssl_server_connection *);

ssl_connection * insert_accepted_ssl_socket (ssl_server_connection *);

int close_ssl_socket(ssl_connection *);
int close_ssl_server_socket(ssl_server_connection *);
int close_accepted_ssl_socket(ssl_server_connection *, ssl_connection *);

int cleanup_ssl_socket(ssl_connection *);
int cleanup_ssl_server_socket(ssl_server_connection *);

int delete_ssl_socket(ssl_connection *);
int delete_ssl_server_socket(ssl_server_connection *);
int delete_accepted_ssl_socket (ssl_server_connection *, ssl_connection *);

int update_ssl_cert_data(ssl_connection *);
int check_ssl_md5sum(ssl_connection * , char *);

int send_ssl_socket(ssl_connection *, void *, int);
int recv_ssl_socket(ssl_connection *, void *, int, int timeout);

int start_ssl(void);
int stop_ssl(void);
void config_ssl(int);

int have_ssl(void);

#endif
