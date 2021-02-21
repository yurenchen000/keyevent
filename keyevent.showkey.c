/*
this code based on `kbd` package, showkey.c
  GPL v2
*/
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <signal.h>
#include <termios.h>
#include <sys/ioctl.h>

#include <linux/kd.h>
#include <linux/keyboard.h>

#include <string.h>
#include <stdarg.h>

#include <locale.h>
#include <libintl.h>


//------------------------ vars

int fd;
int oldkbmode;
struct termios old;


// #define _(Text) gettext(Text)
#define _(Text) (Text)
// #define LC_ALL C

#define PACKAGE_NAME "kbd"
#define LOCALEDIR "/usr/share/locale"


//------------------------
//------------------------
//------------------------

//------------------------ getfd
static char *conspath[] = {
	"/proc/self/fd/0",
	"/dev/tty",
	"/dev/tty0",
	"/dev/vc/0",
	"/dev/systty",
	"/dev/console",
	NULL
};

/*
 * getfd.c
 *
 * Get an fd for use with kbd/console ioctls.
 * We try several things because opening /dev/console will fail
 * if someone else used X (which does a chown on /dev/console).
 */

static int
is_a_console(int fd)
{
	char arg;

	arg = 0;
	return (isatty(fd) && ioctl(fd, KDGKBTYPE, &arg) == 0 && ((arg == KB_101) || (arg == KB_84)));
}

static int
open_a_console(const char *fnam)
{
	int fd;

	/*
	 * For ioctl purposes we only need some fd and permissions
	 * do not matter. But setfont:activatemap() does a write.
	 */
	fd = open(fnam, O_RDWR);
	if (fd < 0)
		fd = open(fnam, O_WRONLY);
	if (fd < 0)
		fd = open(fnam, O_RDONLY);
	if (fd < 0)
		return -1;
	return fd;
}

int getfd(const char *fnam)
{
	int fd, i;

	if (fnam) {
		if ((fd = open_a_console(fnam)) >= 0) {
			if (is_a_console(fd))
				return fd;
			close(fd);
		}
		fprintf(stderr, _("Couldn't open %s\n"), fnam);
		exit(1);
	}

	for (i = 0; conspath[i]; i++) {
		if ((fd = open_a_console(conspath[i])) >= 0) {
			if (is_a_console(fd))
				return fd;
			close(fd);
		}
	}

	for (fd = 0; fd < 3; fd++)
		if (is_a_console(fd))
			return fd;

	fprintf(stderr,
	        _("Couldn't get a file descriptor referring to the console\n"));

	/* total failure */
	exit(1);
}

static int
kbmode(int fd) {
	int mode;

	if (ioctl(fd, KDGKBMODE, &mode) < 0)
		return -1;
	else
		return mode;
}


//------------------------ kbd_error

static const char *progname         = NULL;

static inline const char *
set_progname(const char *name)
{
	char *p;
	p = strrchr(name, '/');
	return (p && p + 1 ? p + 1 : name);
}


void
    __attribute__((format(printf, 2, 3)))
    kbd_warning(const int errnum, const char *fmt, ...)
{
	va_list ap;

	va_start(ap, fmt);

	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);

	if (errnum > 0)
		fprintf(stderr, ": %s\n", strerror(errnum));

	va_end(ap);
	return;
}

void
    __attribute__((noreturn))
    __attribute__((format(printf, 3, 4)))
    kbd_error(const int exitnum, const int errnum, const char *fmt, ...)
{
	va_list ap;
	va_start(ap, fmt);

	fprintf(stderr, "%s: ", progname);
	vfprintf(stderr, fmt, ap);

	va_end(ap);

	if (errnum > 0)
		fprintf(stderr, ": %s\n", strerror(errnum));

	exit(exitnum);
}


//------------------------

/*
 * version 0.81 of showkey would restore kbmode unconditially to XLATE,
 * thus making the console unusable when it was called under X.
 */
static void
get_mode(void)
{
	char *m;

	if (ioctl(fd, KDGKBMODE, &oldkbmode)) {
		kbd_error(EXIT_FAILURE, errno, "ioctl KDGKBMODE");
	}
	switch (oldkbmode) {
		case K_RAW:
			m = "RAW";
			break;
		case K_XLATE:
			m = "XLATE";
			break;
		case K_MEDIUMRAW:
			m = "MEDIUMRAW";
			break;
		case K_UNICODE:
			m = "UNICODE";
			break;
		default:
			m = _("?UNKNOWN?");
			break;
	}
	printf(_("kb mode was %s\n"), m);
	if (oldkbmode != K_XLATE) {
		printf(_("[ if you are trying this under X, it might not work\n"
		         "since the X server is also reading /dev/console ]\n"));
	}
	printf("\n");
}


static void
clean_up(void)
{
	if (ioctl(fd, KDSKBMODE, oldkbmode)) {
		kbd_error(EXIT_FAILURE, errno, "ioctl KDSKBMODE");
	}
	if (tcsetattr(fd, 0, &old) == -1)
		kbd_warning(errno, "tcsetattr");
	close(fd);
}

static void __attribute__((noreturn))
die(int x)
{
	printf(_("caught signal %d, cleaning up...\n"), x);
	clean_up();
	exit(EXIT_FAILURE);
}

static void __attribute__((noreturn))
watch_dog(int x __attribute__((unused)))
{
	clean_up();
	exit(EXIT_SUCCESS);
}


//------------------------ main

int main(int argc, char *argv[])
{

	int show_keycodes = 1;

	struct termios new;
	unsigned char buf[18]; /* divisible by 3 */
	int i, n;

	setlocale(LC_ALL, "");
	bindtextdomain(PACKAGE_NAME, LOCALEDIR);
	textdomain(PACKAGE_NAME);

	//------------------------

	if ((fd = getfd(NULL)) < 0)
		kbd_error(EXIT_FAILURE, 0, _("Couldn't get a file descriptor referring to the console"));

	/* the program terminates when there is no input for 10 secs */
	signal(SIGALRM, watch_dog);

	/*
	  if we receive a signal, we want to exit nicely, in
	  order not to leave the keyboard in an unusable mode
	*/
	signal(SIGHUP, die);
	signal(SIGINT, die);
	signal(SIGQUIT, die);
	signal(SIGILL, die);
	signal(SIGTRAP, die);
	signal(SIGABRT, die);
	signal(SIGIOT, die);
	signal(SIGFPE, die);
	signal(SIGKILL, die);
	signal(SIGUSR1, die);
	signal(SIGSEGV, die);
	signal(SIGUSR2, die);
	signal(SIGPIPE, die);
	signal(SIGTERM, die);
#ifdef SIGSTKFLT
	signal(SIGSTKFLT, die);
#endif
	signal(SIGCHLD, die);
	signal(SIGCONT, die);
	signal(SIGSTOP, die);
	signal(SIGTSTP, die);
	signal(SIGTTIN, die);
	signal(SIGTTOU, die);

	get_mode();
	if (tcgetattr(fd, &old) == -1)
		kbd_warning(errno, "tcgetattr");
	if (tcgetattr(fd, &new) == -1)
		kbd_warning(errno, "tcgetattr");

	new.c_lflag &= ~(ICANON | ECHO | ISIG);
	new.c_iflag     = 0;
	new.c_cc[VMIN]  = sizeof(buf);
	new.c_cc[VTIME] = 1; /* 0.1 sec intercharacter timeout */

	if (tcsetattr(fd, TCSAFLUSH, &new) == -1)
		kbd_warning(errno, "tcsetattr");
	if (ioctl(fd, KDSKBMODE, show_keycodes ? K_MEDIUMRAW : K_RAW)) {
		kbd_error(EXIT_FAILURE, errno, "ioctl KDSKBMODE");
	}

	printf(_("press any key (program terminates 10s after last keypress)...\n"));


	//------------------------



	/* show keycodes - 2.6 allows 3-byte reports */
	while (1) {
		alarm(10);
		n = read(fd, buf, sizeof(buf));
		i = 0;
		while (i < n) {
			int kc;
			char *s;

			s = (buf[i] & 0x80) ? _("release") : _("press");

			if (i + 2 < n && (buf[i] & 0x7f) == 0 && (buf[i + 1] & 0x80) != 0 && (buf[i + 2] & 0x80) != 0) {
				kc = ((buf[i + 1] & 0x7f) << 7) |
				     (buf[i + 2] & 0x7f);
				i += 3;
			} else {
				kc = (buf[i] & 0x7f);
				i++;
			}
			printf(_("\rkeycode %3d %s\n"), kc, s);
		}
	}

	clean_up();
	return EXIT_SUCCESS;
}
