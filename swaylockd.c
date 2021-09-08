// vim: set ts=4:
// SPDX-FileCopyrightText: 2021 Jakub Jirutka <jakub@jirutka.cz>
// SPDX-License-Identifier: MIT

#define _POSIX_C_SOURCE 200809L

#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <spawn.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <syslog.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/wait.h>

#define PROGNAME "swaylockd"

#ifndef SWAYLOCK_PATH
  #define SWAYLOCK_PATH "/usr/bin/swaylock"
#endif

#define logerr(format, ...) \
	do { \
		fprintf(stderr, PROGNAME ": " format "\n", __VA_ARGS__); \
		if (syslog_enabled) syslog(LOG_ERR, format, __VA_ARGS__); \
	} while (0)


extern char **environ;

static bool syslog_enabled = true;

int main (int argc, char **argv) {
	char lock_path[PATH_MAX] = "\0";
	int lock_fd = -1;
	int rc = 1;

	// Open connection to syslog.
	openlog(PROGNAME, LOG_PID, LOG_USER);

	{
		const char *xdg_rd = getenv("XDG_RUNTIME_DIR");
		if (xdg_rd == NULL) {
			logerr("%s", "XDG_RUNTIME_DIR not set in the environment");
			return 100;
		}
		(void) snprintf(lock_path, sizeof(lock_path), "%s/%s.lock", xdg_rd, PROGNAME);
	}

	// Obtain exclusive lock.
	if ((lock_fd = open(lock_path, O_CREAT | O_RDWR | O_CLOEXEC, 0600)) < 0) {
		logerr("failed to write %s: %s", lock_path, strerror(errno));
		return 101;
	}
	if (flock(lock_fd, LOCK_EX | LOCK_NB) < 0) {
		logerr("another instance of %s is running", PROGNAME);
		return 102;
	}

	// Block (ignore) all signals we can (i.e. except SIGKILL and SIGSTOP).
	{
		sigset_t mask;
		sigfillset(&mask);
		sigprocmask(SIG_SETMASK, &mask, NULL);
	}

	argv[0] = SWAYLOCK_PATH;

	for (int i = 1;; i++) {
		pid_t pid;
		if (posix_spawn(&pid, SWAYLOCK_PATH, NULL, 0, argv, environ) != 0) {
			logerr("failed to spawn %s: %s", SWAYLOCK_PATH, strerror(errno));
			rc = 101;
			goto done;
		}

		int status = -1;
		(void) waitpid(pid, &status, 0);

		// Exit only if the child terminated normally, not via signal.
		if (WIFEXITED(status)) {
			rc = WEXITSTATUS(status);
			goto done;
		}
		logerr("%s terminated with signal %d, restarting (%d)",
		       SWAYLOCK_PATH, WIFSIGNALED(status) ? WTERMSIG(status) : -1, i);

		// Just to make sure we don't flood syslog in case of some bug...
		if (i > 100) syslog_enabled = false;
	}

done:
	if (unlink(lock_path) < 0) {
		logerr("failed to remove lock file %s: %s", lock_path, strerror(errno));
	}
	return rc;
}
