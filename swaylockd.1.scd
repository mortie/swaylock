swaylockd(1)

# NAME

swaylockd - launch swaylock(1) and re-launch if it dies


# SYNOPSIS

*swaylockd* [options...]

All options are directly passed to *swaylock(1)*.


# DESCRIPTION

*swaylockd* is a dumb launcher to spawn *swaylock(1)* and ensure it's running no matter what - it immediately restarts *swaylock(1)* if terminated by a signal (e.g. the process crashed) and also blocks all signals (except `SIGKILL` and `SIGSTOP`).
It also ensures that only one instance per user is running (using *flock(2)*).

*IMPORTANT*: *swaylockd* is not compatible with *swaylock(1)* option *--daemonize*!


# ENVIRONMENT

*XDG_RUNTIME_DIR*:
	This variable must be set and contain the path to the existing directory where the lock file will be created.
	Refer to "XDG Base Directory Specification" for more information.


# FILES

*${XDG_RUNTIME_DIR}/swaylockd.lock*
	A lock file created when *swaylockd* is executed and removed right before it quits.


# LOGGING

Error messages generated by *swaylockd* are printed to STDERR and logged to syslog with ident string "swaylockd" and facility code 1 (user).


# HISTORY

*swaylockd* has been developed mainly as a workaround for the *swaylock(1)* stability issues (see e.g. https://github.com/swaywm/swaylock/issues/181).

*swaylock* is a critical piece of security software - as a screen locker, any bug in the program that causes it to crash will cause the screen to unlock.
As soon as *swaylock* is no longer running, the screen is no longer locked.
Therefore, great care must be taken to ensure that it never crash.

*swaylockd* started (and is still available) as a standalone project at https://github.com/jirutka/swaylockd.
The code has been merged into sway-effects.


# AUTHORS

Jakub Jirutka


# SEE ALSO

*swaylock(1)*