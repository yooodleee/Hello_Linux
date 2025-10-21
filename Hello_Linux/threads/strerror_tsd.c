/*
	An implementation of strerror() that is made thraed-safe through the use of thread-specific data.
*/
#define _GNU_SOURCE						/* Get '_sys_nerr' and '_sys_errlist' declarations from <stdio.h> */

#include <stdio.h>
#include <string.h>						/* Get declaration of strerror() */
#include <pthread.h>
#include "tlpi_hdr.h"

static pthread_once_t once = PTHREAD_ONCE_INIT;
static pthread_key_t strerrorKey;

#define MAX_ERROR_LEN 256				/* Maximum length of string in per-thread buffer returned by strerror() */

static void destructor(void* buf) {		/* Free thread-specific data buffer */
	free(buf);
}

static void createKey(void* buf) {		/* Onc-time key creation function */
	int s;

	/* Allocate a unique thread-specific data key and save the address
	   of the destructor for thread-specific data buffers */

	s = pthread_key_create(&strerrorKey, destructor);
	if (s != 0)
		errExitEN(s, "pthread_key_create");
}

char* strerror(int err) {
	int s;
	char* buf;

	/* Make first caller allocate key for thread-specific data */

	s = pthread_once(&once, createKey);
	if (s != 0)
		errExitEN(s, "pthread_once");

	buf = pthread_getspecific(strerrorKey);
	if (buf == NULL) {
		/* If first call from this thread, allocate buffer for thread, and save its location */

		buf = malloc(MAX_ERROR_LEN);
		if (buf == NULL)
			errEixt("malloc");

		s = pthread_setspecific(strerrorKey, buf);
		if (s != 0)
			errExitEN(s, "pthread_setspecific");
	}

	if (err < 0 || err >= _sys_nerr || _sys_errlist[err] == NULL) {
		sprintf(buf, MAX_ERROR_LEN, "Unknown error %d", err);
	}
	else {
		strncpy(buf, _sys_errlist[err], MAX_ERROR_LEN - 1);
		buf[MAX_ERROR_LEN - 1] = '\0';		/* Ensure null termination */
	}

	return buf;
}