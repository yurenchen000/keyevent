#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <linux/input.h>
//#include <sys/fcntl.h>
#include <fcntl.h>

#include <stdint.h>

char key[5]={0};
char *cmd[5]={0};

int parse_conf(){
	return 0;
}

#define KEY_POWER  116
#define KEY_RESET  117
#define KEY_DISARM 118

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

int key_event(int keycode){
	switch(keycode){
		case KEY_RESET:
			// system("/etc/key_reset.sh &");
			break;	
		case KEY_DISARM:
			// system("/etc/key_disarm.sh &");
			break;
		default:
			break;
	}
	return 0;
}

int main(int argc, char *argv[])
{
	int fd = -1;
	size_t rb;

	struct input_event ev;

	if ((fd = open("/dev/input/event3", O_RDONLY)) < 0) {
		perror("open error");
		exit(1);
	}
	//struct size
	printf("sizeof(struct input_event): %ld\n", sizeof(struct input_event));
	printf("sizeof(ev.time): %ld\n", sizeof(ev.time));
	printf("sizeof(ev.type): %ld\n", sizeof(ev.type));
	printf("sizeof(ev.code): %ld\n", sizeof(ev.code));
	printf("sizeof(ev.value): %ld\n", sizeof(ev.value));
	//endian
	uint16_t a=0x1122;
	uint8_t *x=(uint8_t *)&a;
	uint8_t *y=x+1;
	printf("%hx = %hhx %hhx\n", a, *x, *y);

	while(1) {
		rb = read(fd, &ev, sizeof(struct input_event));
		if (rb < sizeof(struct input_event)) {
			perror("read error");
			exit(1);
		}

		if (ev.type == EV_KEY) {
			if(ev.value==2) continue; //ignore auto repeat

			switch(ev.code){
				case KEY_CTRL:
					st_ctrl = ev.value; break;
				case KEY_ALT:
					st_alt  = ev.value; break;
				case KEY_LEFT:
					if(st_alt & st_ctrl){
						printf("\e[033m key-combine: ctrl+alt+left! \e[0m\n");
						// system("xdotool key --window 0 'ctrl+alt+Left'");
						// system("xdotool set_desktop 1");
						// system("xdotool set_desktop --relative -- -1");
						// system("[ \"`xdotool get_desktop`\" == 2 ] && xdotool set_desktop --relative -- -1");
						system("/usr/bin/test \"`xdotool get_desktop`\" == 2 && xdotool set_desktop --relative -- -1");
/*
getwindowpid
$ xdotool getwindowname 56623117
win7_dev [Running] - Oracle VM VirtualBox
*/
					}
					break;
			}

			if (!ev.value){
				// printf("key is released, code = %d\n", ev.code);
				printf("key is release, value = %d, code = %d\n", ev.value, ev.code);
			}else{
				printf("key is pressed, value = %d, code = %d\n", ev.value, ev.code);

				// key_event(ev.code);
			}
		}
	}

	close(fd);

	return 0;
}
