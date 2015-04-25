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


#ifndef MONITOR_H
#define MONITOR_H

#include <config.h>

#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif

#ifdef HAVE_SIGNAL_H
#include <signal.h>
#endif

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#ifdef HAVE_STDARG_H
#include <stdarg.h>
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#ifdef HAVE_REGEX_H
#include <regex.h>
#endif

#include "ssl.h"
#include "socket.h"


#define VERSION            PACKAGE_VERSION
#define MONITRC            "monitrc"
#define TIMEFORMAT         "%Z %b %e %T"
#define STRERROR            strerror(errno)
#define STRLEN             256
#define ARGMAX             64
/* Set log file mode: "-rw-rw-r--" */
#define LOGMASK            0112
/* Set pid file mode: "-rw-r--r--" */
#define MYPIDMASK          0122
#define MYPIDDIR           PIDDIR
#define MYPIDFILE          "monit.pid"
#define MYSTATEFILE        "monit.state"

#define LOCALHOST          "localhost"
#define SMTP_PORT          25

#define SSL_TIMEOUT        15

#define START_HTTP         1
#define STOP_HTTP          2

#define TRUE               1
#define FALSE              0

#define MODE_ACTIVE        0
#define MODE_PASSIVE       1
#define MODE_MANUAL        2

#define OPERATOR_GREATER   0
#define OPERATOR_LESS      1
#define OPERATOR_EQUAL     2
#define OPERATOR_NOTEQUAL  3

#define TIME_SECOND        1
#define TIME_MINUTE        60
#define TIME_HOUR          3600
#define TIME_DAY           86400

#define ACTION_ALERT       1
#define ACTION_RESTART     2
#define ACTION_STOP        3
#define ACTION_EXEC        4
#define ACTION_UNMONITOR   5

#define TYPE_DEVICE        0
#define TYPE_DIRECTORY     1
#define TYPE_FILE          2
#define TYPE_PROCESS       3
#define TYPE_REMOTE        4

#define RESOURCE_ID_CPU_PERCENT       1
#define RESOURCE_ID_MEM_PERCENT       2
#define RESOURCE_ID_MEM_KBYTE         3
#define RESOURCE_ID_LOAD1             4
#define RESOURCE_ID_LOAD5             5
#define RESOURCE_ID_LOAD15            6
#define RESOURCE_ID_CHILDREN          7
#define RESOURCE_ID_TOTAL_MEM_KBYTE   8
#define RESOURCE_ID_TOTAL_MEM_PERCENT 9
#define RESOURCE_ID_INODE             10
#define RESOURCE_ID_SPACE             11

#define DIGEST_CLEARTEXT   1
#define DIGEST_CRYPT       2
#define DIGEST_MD5         3

#define UNIT_BYTE          1
#define UNIT_KILOBYTE      1024
#define UNIT_MEGABYTE      1048580
#define UNIT_GIGABYTE      1073740000

#define HASH_MD5           1
#define HASH_SHA1          2
#define DEFAULT_HASH       HASH_MD5

/** ------------------------------------------------- Special purpose macros */


/* Mask compiler built-in namespace (C99) and reserved identifiers as
 *  well as external identifiers (in libraries we link with). */
#define log  log_log

/* Replace the standard signal function with a more reliable using
 * sigaction. Taken from Stevens APUE book. */
typedef void Sigfunc(int);
Sigfunc *signal(int signo, Sigfunc * func);
#if defined(SIG_IGN) && !defined(SIG_ERR)
#define SIG_ERR ((Sigfunc *)-1)
#endif


/** ------------------------------------------------- General purpose macros */


#undef MAX
#define MAX(x,y) ((x) > (y) ? (x) : (y))
#undef MIN
#define MIN(x,y) ((x) < (y) ? (x) : (y))
#define IS(a,b)  ((a&&b)?!strcasecmp(a, b):0)
#define DEBUG if(Run.debug) log


/** ------------------------------------------------- Synchronization macros */


#define LOCK(mutex) do { pthread_mutex_t *_yymutex = &(mutex); \
        pthread_mutex_lock(_yymutex);
#define END_LOCK pthread_mutex_unlock(_yymutex); } while (0)


/** ---------------------------------- Object Constructor/"Destructor" macro */


#define NEW(p) ((p)= xcalloc(1, (long)sizeof *(p)))
#define FREE(p) ((void)(free(p), (p)= 0))


/** ------------------------------------------ Simple Assert Exception macro */


#define ASSERT(e) if(!(e)) { log("AssertException: at %s:%d\naborting..\n", \
    __FILE__, __LINE__); abort(); }


/* --------------------------------------------------------- Data structures */

/** Defines a mailinglist object */
typedef struct mymail {
  char *to;                         /**< Mail address for alert notification */
  char *from;                                     /**< The mail from address */
  char *subject;                                       /**< The mail subject */
  char *message;                                       /**< The mail message */
  char *opt_message;                 /**< An optional message used in alerts */
  unsigned int events;  /*< Events for which this mail object should be sent */

  /** For internal use */
  struct mymail *next;                          /**< next recipient in chain */
} *Mail_T;


/** Defines a mail server address */
typedef struct mymailserver {
  char *host;     /**< Server host address, may be a IP or a hostname string */
  /** For internal use */
  struct mymailserver *next;        /**< Next server to try on connect error */
} *MailServer_T;


typedef struct myauthentication {
  char *uname;                  /**< User allowed to connect to monit httpd */
  char *passwd;                                /**< The users password data */
  int   digesttype;                      /**< How did we store the password */
  int   is_readonly;     /**< TRUE if this is a read-only authenticated user*/
  struct myauthentication *next;       /**< Next credential or NULL if last */
} *Auth_T;


/** Defines process tree */
typedef struct myprocesstree {
  int  pid;
  int  ppid;
  int  visited;
  long mem_kbyte;
  int  children_num;
  int  children_sum;
  long mem_kbyte_sum;

  /** For internal use */
  struct myprocesstree  *parent;
  struct myprocesstree **children;
} ProcessTree_T;


/** Defines data for application runtime */
struct myrun {
  int  stopped;         /**< TRUE if monit was stopped. Flag used by threads */
  char *controlfile;                /**< The file to read configuration from */
  char *logfile;                         /**< The file to write logdata into */
  char *localhostname;                      /**< The host name for localhost */
  char *pidfile;                                  /**< This programs pidfile */
  char *statefile;                /**< The file with the saved runtime state */
  char *mygroup;                              /**< Group Name of the Service */
  int  debug;                   /**< Write debug information - TRUE or FALSE */
  int  use_syslog;                          /**< If TRUE write log to syslog */
  int  dolog;       /**< TRUE if program should log actions, otherwise FALSE */
  int  isdaemon;                 /**< TRUE if program should run as a daemon */
  int  polltime;        /**< In deamon mode, the sleeptime (sec) between run */
  int  dohttpd;                    /**< TRUE if monit HTTP server should run */
  int  httpdssl;                     /**< TRUE if monit HTTP server uses ssl */
  char *httpsslpem;                       /**< PEM file for the HTTPS server */
  int  clientssl;   /**< TRUE if monit HTTP server uses ssl with client auth */
  char *httpsslclientpem;      /**< PEM file/dir to check against at connect */
  int  servicessl;                 /**< TRUE if monit has ssl service checks */
  int  allowselfcert;   /**< TRUE if self certified client certs are allowed */
  int  httpdsig;   /**< TRUE if monit HTTP server presents version signature */
  int  httpdport;                    /**< The monit http server's portnumber */
  int  once;                                       /**< TRUE - run only once */
  int  init;                   /**< TRUE - don't background to run from init */
  int  facility;              /** The facility to use when running openlog() */
  int  doprocess;                 /**< TRUE if process status engine is used */
  char *bind_addr;                  /**< The address monit http will bind to */
  int  doreload;             /**< TRUE if a monit daemon should reinitialize */
  mode_t umask;                /**< The initial umask monit was started with */
  int  testing;   /**< Running in configuration testing mode - TRUE or FALSE */

  double loadavg[3];                                /**< Load average triple */

       /** An object holding program relevant "environment" data, see; env.c */
  struct myenvironment {
    char *user;             /**< The the effective user running this program */
    char *home;                                    /**< Users home directory */
    char *cwd;                                /**< Current working directory */
  } Env;

  int mailserver_timeout;    /**< Connect and read timeout for a SMTP server */
  Mail_T maillist;                /**< Global alert notification mailinglist */
  MailServer_T mailservers;    /**< List of MTAs used for alert notification */
  Auth_T credentials;    /** A list holding Basic Authentication information */
                                      /** User selected standard mail format */
  struct myformat {
    char *from;                          /**< The standard mail from address */
    char *subject;                            /**< The standard mail subject */
    char *message;                            /**< The standard mail message */
  } MailFormat;
                                          
  pthread_mutex_t mutex;    /**< Mutex used for service data synchronization */

};


/**
 * Defines a Command with ARGMAX optional arguments. The arguments
 * array must be NULL terminated and the first entry is the program
 * itself. In addition, a user and group may be set for the Command
 * which means that the Command should run as a certain user and with
 * certain group.
 */
typedef struct mycommand {
  char *arg[ARGMAX];                             /**< Program with arguments */
  int length;                         /**< The length of the arguments array */
  int has_uid;            /**< TRUE if a new uid is defined for this Command */
  uid_t uid;         /**< The user id to switch to when running this Command */
  int has_gid;            /**< TRUE if a new gid is defined for this Command */
  gid_t gid;        /**< The group id to switch to when running this Command */
} *Command_T;


/** Defines a protocol object with protocol functions */
typedef struct myprotocol {
  const char *name;                                       /**< Protocol name */
  int(*check)(Socket_T);                 /**< Protocol verification function */
} *Protocol_T;


/** Defines a send/expect object used for generic protocol tests */
typedef struct mygenericproto {
  char *send;                           /* string to send, or NULL if expect */
#ifdef HAVE_REGEX_H
  regex_t *expect;                  /* regex code to expect, or NULL if send */
#else
  char *expect;                         /* string to expect, or NULL if send */
#endif
  /** For internal use */
  struct mygenericproto *next;
} *Generic_T;


/** Defines a port object */
typedef struct myport {
  volatile int socket;                       /**< Socket used for connection */
  int  type;                  /**< Socket type used for connection (UDP/TCP) */
  int  family;            /**< Socket family used for connection (INET/UNIX) */
  char *hostname;                                     /**< Hostname to check */
  int  port;                                                 /**< Portnumber */
  char *request;                              /**< Specific protocol request */
  char *request_checksum;     /**< The optional checksum for a req. document */
  int  request_hashtype;  /**< The optional type of hash for a req. document */
  char *pathname;                   /**< Pathname, in case of an UNIX socket */
  char *address;               /**< Human readable destination of the socket */
  Generic_T generic;                                /**< Generic test handle */
  int  action;                                /**< Action in case of failure */
  int  event_flag;              /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec;      /**< Optional command to be executed upon a port event*/
  int timeout;   /**< The timeout in seconds to wait for connect or read i/o */
  int is_available;                /**< TRUE if the server/port is available */
  
  struct {
    int use_ssl;          /**< TRUE if this port requires SSL for connection */
    int version;                  /**< The SSL version to use for connection */
    char *certmd5;     /**< The expected md5 sum of the server's certificate */
  } SSL;

  /**< Object used for testing a port's service */
  Protocol_T protocol;
  
  /** For internal use */
  struct myport *next;                               /**< next port in chain */
} *Port_T;


/** Defines a ICMP object */
typedef struct myicmp {
  int type;                                              /**< ICMP type used */
  int timeout;              /**< The timeout in seconds to wait for response */
  int action;                                 /**< Action in case of failure */
  int event_flag;               /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec;     /**< Optional command to be executed upon a icmp event */
  int is_available;                     /**< TRUE if the server is available */
  
  /** For internal use */
  struct myicmp *next;                               /**< next icmp in chain */
} *Icmp_T;


typedef struct mydependant {
  char *dependant;                           /**< name of dependant service  */

  /** For internal use */
  struct mydependant *next;           /**< next dependant service in chain   */
} *Dependant_T;


/** Defines resource data */
typedef struct myresource {
  int  resource_id;                              /**< Which value is checked */
  long limit;                                     /**< Limit of the resource */
  int  operator;                                    /**< Comparison operator */
  int  cycle;                                     /**< Cycle overrun counter */
  int  max_cycle;                                   /**< Cycle overrun limit */
  int  action;                                /**< Action in case of failure */
  int  event_flag;              /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec;   /**< Optional command to be executed upon a resurce event*/
  
  /** For internal use */
  struct myresource *next;                       /**< next resource in chain */
} *Resource_T;


/** Defines timestamp object */
typedef struct mytimestamp {
  int  operator;                                    /**< Comparison operator */
  int  time;                                        /**< Timestamp watermark */
  int  action;                                /**< Action in case of failure */
  int  test_changes;            /**< TRUE if we only should test for changes */
  time_t timestamp; /**< The original last modified timestamp for this object*/
  int  event_flag;              /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec; /**< Optional command to be executed upon a timestamp event*/
  
  /** For internal use */
  struct mytimestamp *next;                     /**< next timestamp in chain */
} *Timestamp_T;


/** Defines size object */
typedef struct mysize {
  int  operator;                                    /**< Comparison operator */
  unsigned long size;                                    /**< Size watermark */
  int  action;                                /**< Action in case of failure */
  int  test_changes;            /**< TRUE if we only should test for changes */
  unsigned long runsize;              /**< The original size for this object */
  int  event_flag;              /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec;     /**< Optional command to be executed upon a size event */
  
  /** For internal use */
  struct mysize *next;                          /**< next timestamp in chain */
} *Size_T;


/** Defines checksum object */
typedef struct mychecksum {
  char     *hash;                 /**< A checksum hash computed for the path */
  int       type;                   /**< The type of hash (e.g. md5 or sha1) */
  int       length;                                  /**< Length of the hash */
  int       action;                           /**< Action in case of failure */
  int       event_flag;         /**< TRUE if an event occured on this object */
  Command_T exec; /**< Optional command to be executed upon a checksum event */
} *Checksum_T;


/** Defines permission object */
typedef struct myperm {
  int       perm;                                     /**< Access permission */
  int       action;                           /**< Action in case of failure */
  int       event_flag;         /**< TRUE if an event occured on this object */
  Command_T exec;     /**< Optional command to be executed upon a perm event */
  int       has_error;       /**< TRUE if the service has a permission error */
} *Perm_T;


/** Defines uid object */
typedef struct myuid {
  uid_t     uid;                                            /**< Owner's uid */
  int       action;                           /**< Action in case of failure */
  int       event_flag;         /**< TRUE if an event occured on this object */
  Command_T exec;      /**< Optional command to be executed upon a uid event */
  int       has_error;              /**< TRUE if the service has a UID error */
} *Uid_T;


/** Defines gid object */
typedef struct mygid {
  gid_t     gid;                                            /**< Owner's gid */
  int       action;                           /**< Action in case of failure */
  int       event_flag;         /**< TRUE if an event occured on this object */
  Command_T exec;       /**< Optional command to be executed upon a gid event*/
  int       has_error;              /**< TRUE if the service has a GID error */
} *Gid_T;


/** Defines procfs (or other mechanism) data */
typedef struct myprocinfo {
  int  pid;
  int  ppid;
  int  status_flag;
  long mem_kbyte;    
  int  mem_percent;                                      /**< pecentage * 10 */
  int  cpu_percent;                                      /**< pecentage * 10 */
  int  children;
  long total_mem_kbyte;
  int  total_mem_percent;                                /**< pecentage * 10 */

  /* private for calculating cpu_percent */
  double time_prev;                                        /**< 1/10 seconds */
  long  cputime;                                           /**< 1/10 seconds */
  long  cputime_prev;                                      /**< 1/10 seconds */
} *ProcInfo_T;


/** Defines filesystem device data */
typedef struct mydeviceinfo {
  long f_bsize;                                     /**< Transfer block size */
  long f_blocks;                        /**< Total data blocks in filesystem */
  long f_blocksfree;             /**< Free blocks available to non-superuser */
  long f_blocksfreetotal;                     /**< Free blocks in filesystem */
  long f_files;                          /**< Total file nodes in filesystem */
  long f_filesfree;                       /**< Free file nodes in filesystem */

  int  event_handled;        /**< TRUE if this event has been handled before */
  /** For internal use */
  char mntpath[STRLEN];        /**< Filesystem file, directory or mountpoint */
} *DeviceInfo_T;


/** Defines filesystem device configuration */
typedef struct mydevice {
  int  resource;                        /**< Whether to check inode or space */
  int  operator;                                    /**< Comparison operator */
  long limit_absolute;                               /**< Watermark - blocks */
  int  limit_percent;                               /**< Watermark - percent */
  int  action;                                /**< Action in case of failure */
  int  event_flag;              /**< TRUE if an event occured on this object */
  int  event_handled;        /**< TRUE if this event has been handled before */
  Command_T exec;   /**< Optional command to be executed upon a device event */

  /** For internal use */
  struct mydevice *next;                           /**< next device in chain */
} *Device_T;


/** Defines service data */
typedef struct myservice {

  /** Common parameters */
  char *name;                                  /**< Service descriptive name */
  char *group;                                       /**< Service group name */
  int (*check)(struct myservice *);       /**< Service verification function */
  Command_T start;                    /**< The start command for the service */
  Command_T stop;                      /**< The stop command for the service */
  int  type;                                     /**< Monitored service type */
  int  do_monitor; /**< Monitor flag, if FALSE, the service is not monitored */
  int  mode;                            /**< Monitoring mode for the service */
  int  ncycle;                          /**< The number of the current cycle */
  int  nstart;           /**< The number of current starts with this service */
  int  to_start;                                  /**< Timeout start ceiling */
  int  to_cycle;                                  /**< Timeout cycle ceiling */
  int  every;                        /**< Check this program at given cycles */
  int  nevery;          /**< Counter for every.  When nevery == every, check */
  int  def_every;              /**< TRUE if every is defined for the service */
  int  def_timeout;          /**< TRUE if timeout is defined for the service */
  int  def_procinfo;        /**< TRUE if procinfo is defined for the service */
  int  visited;      /**< Service visited flag, set if dependencies are used */
  int  depend_visited;/**< Depend visited flag, set if dependencies are used */
  int  event_handled; /**< TRUE if an event has been handled on this service */

  Mail_T      maillist;                  /**< Alert notification mailinglist */
  Dependant_T dependantlist;                      /**<Dependant service list */

  /** Tests */
  Checksum_T  checksum;                                  /**< Checksum check */
  Device_T    devicelist;                             /**< Device check list */
  Gid_T       gid;                                            /**< Gid check */
  Icmp_T      icmplist;                                 /**< ICMP check list */
  Perm_T      perm;                                    /**< Permission check */
  Port_T      portlist;              /**< Portnumbers the service listens on */
  Resource_T  resourcelist;                          /**< Resouce check list */
  Size_T      sizelist;                                 /**< Size check list */
  Timestamp_T timestamplist;                       /**< Timestamp check list */
  Uid_T       uid;                                            /**< Uid check */
  
  /** Runtime parameters */
  DeviceInfo_T devinfo;                       /**< Data for the device check */
  ProcInfo_T   procinfo;                      /**< Data for the procfs check */

  /** Context specific parameters */
  char *path;   /**< Path to the device, file, directory or process pid file */
  
  /** For internal use */
  pthread_mutex_t   mutex;        /**< Mutex used for action synchronization */
  struct myservice *next;                         /**< next service in chain */
  struct myservice *next_conf;      /**< next service according to conf file */
  struct myservice *next_depend;           /**< next depend service in chain */
} *Service_T;


/* -------------------------------------------------------- Global variables */


char   *prog;                                  /**< The Name of this Program */
struct myrun Run;                      /**< Struct holding runtime constants */
Service_T servicelist;                /**< The service list (created in p.y) */
Service_T servicelist_conf;   /**< The service list in conf file (c. in p.y) */
ProcessTree_T *ptree;                        /**< Global process information */
int            ptreesize;                    /**< Global process information */


/* ------------------------------------------------------- Public prototypes */


char *trim(char *);
char *ltrim(char *);
char *rtrim(char *);
void  trim_quotes(char *);
char *replace_char(char *, char, char);
char *replace_string(char **, const char *, const char *);
int   count_words(char *, const char *);
int   starts_with(const char *, const char *);
void  handle_string_escapes(char *);
void  chomp(char *,int);
int   exist_service(const char *);
int   get_service_list_length();
Service_T get_service(const char *);
void  printrunlist();
void  printservice(Service_T);
void  printservicelist();
pid_t get_pid(char *);
int   is_process_running(Service_T);
int   is_strdefined(char *);
int   isreg_file(char *);
char *stripfilename(char*);
int   exist_file(char *);
char *get_RFC822date(long *);
char *get_uptime(time_t delta, char *);
char *get_process_uptime(char *, char *);
char *get_checksum(char *, int);
int   check_hash(char *, char *, int);
char *url_encode(char *uri);
char *url_decode(char *url);
char *get_basic_authentication_header();
Auth_T get_user_credentials(char *);
int   compare_user_credentials(char *, char *);
int   parse(char *);
void  control(const char *);
void  control_group(const char *, const char *);
void  control_process(const char *, const char *);
void  d_check_service(const char *, const char *);
void  check_service(const char *, const char *);
void  setup_dependants();
void  reset_depend();
void  spawn(Service_T, Command_T, const char *);
void  status();
int   log_init() ;
void  log_log(const char *format, ...) ;
void  log_close();
void  validate() ;
void  daemonize() ;
void  gc();
void  gc_mail_list(Mail_T*);
void  init_files();
time_t get_timestamp(char *, mode_t);
void  finalize_files();
char *find_rcfile();    
int   create_pidfile(char *);
int   check_rcfile(char *);
int   kill_daemon(int);
int   exist_daemon(); 
void  sendmail(Mail_T);
int   sock_msg(int sock, char *, ...);
void  init_env();
void  destroy_env();
void *xmalloc (int);
void *xcalloc(long, long);
char *xstrdup(const char *);
char *xstrndup(const char *, long);
void *xresize(void *, long);
void  monit_http(int);
int   check_httpd();
int   can_http();
char *format(const char *s, va_list ap, long *);
void  redirect_stdfd();
void  fd_close();
pid_t getpgid(pid_t);
int   check_file_stat(char *, char *, int);
void  state_save();
int   state_should_update();
void  state_update();
#if ! HAVE_MALLOC
void *rpl_malloc (size_t __size);
#endif
void unset_signal_block(sigset_t *);
void set_signal_block(sigset_t *, sigset_t *);
int  check_process(Service_T);
int  check_device(Service_T);
int  check_file(Service_T);
int  check_directory(Service_T);
int  check_remote_host(Service_T);


#endif
