/*

module: errlib.c

purpose: library of error functions

reference: Stevens, Unix network programming (2ed), p.922

*/

#include <stdio.h>
#include <stdlib.h> /* exit() */
#include <stdarg.h>
#include <syslog.h>
#include <string.h>
#include <errno.h>
#include <semaphore.h>


#define MAXLINE 4095

int daemon_proc = 0; /* set to 0 if stdout/stderr available, else set to 1 */
sem_t sem_err;

static void err_doit (int errnoflag, int level, const char *fmt, va_list ap)
{
  int errno_save = errno;
  size_t n;
  char buf[MAXLINE+1];

  sem_wait(&sem_err);
  vsnprintf (buf, MAXLINE, fmt, ap);
  n = strlen(buf);
  if (errnoflag)
    snprintf (buf+n, MAXLINE-n, ": %s", strerror(errno_save));
  strcat (buf, "\n");

  if (daemon_proc)
    syslog (level, buf);
  else
  {
    fflush (stdout);
    fputs (buf, stderr);
    fflush (stderr);
  }
  sem_post(&sem_err);
}

void err_ret (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  err_doit (1, LOG_INFO, fmt, ap);
  va_end (ap);
  return;
}

void err_sys (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  err_doit (1, LOG_ERR, fmt, ap);
  va_end (ap);
  exit (1);
}

void err_msg (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  err_doit (0, LOG_INFO, fmt, ap);
  va_end (ap);
  return;
}

void err_quit (const char *fmt, ...)
{
  va_list ap;

  va_start (ap, fmt);
  err_doit (0, LOG_ERR, fmt, ap);
  va_end (ap);
  exit (1);
}

void err_init (){
	sem_init(&sem_err,0,1);
}


