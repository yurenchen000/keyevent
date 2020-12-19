#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
//#include <sys/fcntl.h>
#include <fcntl.h>

#include <stdint.h>
#include <string.h>


/*
key is pressed, value = 1, code = 29
key is release, value = 0, code = 29
key is pressed, value = 1, code = 56
key is release, value = 0, code = 56
key is pressed, value = 1, code = 105
key is release, value = 0, code = 105
key is pressed, value = 1, code = 106
key is release, value = 0, code = 106
*/

#define KEY_CTRL 29
#define KEY_ALT  56
#define KEY_LEFT  105
#define KEY_RIGHT 106

#define K_REPT 2
#define K_DOWN 1
#define K_UP   0

int st_ctrl = 0;
int st_alt  = 0;
char cmd_l[256];
char cmd_r[256];


// system("xdotool key --window 0 'ctrl+alt+Left'");
// system("xdotool set_desktop 1");
// system("xdotool set_desktop --relative -- -1");
// system("[ \"`xdotool get_desktop`\" == 2 ] && xdotool set_desktop --relative -- -1");
// system("/usr/bin/test \"`xdotool get_desktop`\" == 2 && xdotool set_desktop --relative -- -1");
/*
$ xdotool getwindowpid
$ xdotool getwindowname 56623117
win7_dev [Running] - Oracle VM VirtualBox
*/

int key_event(struct input_event *ev) {

	switch(ev->code){
		case KEY_CTRL:
			st_ctrl = ev->value; break;
		case KEY_ALT:
			st_alt  = ev->value; break;

		case KEY_LEFT:
			if(st_alt & st_ctrl){
				if(ev->value){
					printf("\e[033m key-combine: ctrl+alt+left! \e[0m\n");
					system(cmd_l);
				}
			}
			break;
		case KEY_RIGHT:
			if(st_alt & st_ctrl){
				if(ev->value){
					printf("\e[033m key-combine: ctrl+alt+right! \e[0m\n");
					system(cmd_r);
				}
			}
			break;
	}
	return 0;
}


int main(int argc, char *argv[])
{
	//---args
	char *dev = "/dev/input/event3";
	char *num = "2";
	char *lop = "";

	if(argc>1 && strcmp(argv[1],"-h") == 0) {
		printf("usage: %s [dev_path] [workspace_num] [loop]\n\n", argv[0]);
		exit(2);
	}
	if(argc>1) dev = argv[1];
	if(argc>2) num = argv[2];
	if(argc>3) lop = argv[3];


	printf("   device: %s\n", dev);
	printf("workspace: %s\n", num);
	printf(" loopback: %s\n", lop);
	printf("\n");
	if(strcmp(lop, "loop") == 0){
		snprintf(cmd_l, 255, "/usr/bin/test \"`xdotool get_desktop`\" == %s && xdotool set_desktop --relative -- -1", num);
		snprintf(cmd_r, 255, "/usr/bin/test \"`xdotool get_desktop`\" == %s && xdotool set_desktop --relative --  1", num);
	}else{
		snprintf(cmd_l, 255, "c=`xdotool get_desktop`; /usr/bin/test $c == %s %s && xdotool set_desktop --relative -- -1", num, "-a $c -gt 0");
		snprintf(cmd_r, 255, "c=`xdotool get_desktop`; /usr/bin/test $c == %s %s && xdotool set_desktop --relative --  1", num, "-a $((c+1)) -lt `xdotool get_num_desktops`");
	}

	//---main
	int fd = -1;
	size_t rb;

	struct input_event ev;

	if ((fd = open(dev, O_RDONLY)) < 0) {
		perror("open error");
		exit(1);
	}
	//struct size
	// printf("sizeof(struct input_event): %ld\n", sizeof(struct input_event));
	// printf("sizeof(ev.time): %ld\n", sizeof(ev.time));
	// printf("sizeof(ev.type): %ld\n", sizeof(ev.type));
	// printf("sizeof(ev.code): %ld\n", sizeof(ev.code));
	// printf("sizeof(ev.value): %ld\n", sizeof(ev.value));


	while(1) {
		rb = read(fd, &ev, sizeof(struct input_event));
		if (rb < sizeof(struct input_event)) {
			perror("read error");
			exit(1);
		}

		if (ev.type == EV_KEY) {
			if(ev.value==2) continue; //ignore auto repeat

			key_event(&ev);

			if (!ev.value){
				printf("key is release, value = %d, code = %d\n", ev.value, ev.code);
			}else{
				printf("key is pressed, value = %d, code = %d\n", ev.value, ev.code);

			}
		}
	}

	close(fd);

	return 0;
}
