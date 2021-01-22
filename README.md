# keyevent @ubuntu
a tool detect (at background, ubuntu) keyevent &amp; run cmd. 



on ubuntu  
 when vbox get focus, can't use `Ctrl+Alt+Left` to switch workspace.


this daemon tool, as one workaround:

- detect keyevent, then:
  - run cmd to switch workspace // depend `xdotool`

ref:
https://superuser.com/questions/108785/getting-host-to-capture-certain-key-presses-in-virtualbox/1611071

## 1. choice eventX
ls -lh /dev/input/event*

//a. use `evtest`
```sh
sudo evtest
```

~~//b. use `xinput`~~ // it has different device num , not need root (through x input)
```sh
xinput list 
# not need root
xinput test 18
```
//b. use `scan.sh`

a script to scan all event*
```sh
sudo ./scan.sh
```
`Ctrl+C` or `Enter` to exit.

// assume input device is `/dev/input/event18`

## 2. run keyevent

```sh
sudo ./keyevent /dev/input/event18
```

## 3. run background
```sh
# start
INPUT=/dev/input/event18 ./key-vbox.sh start

# stop
key-vbox.sh stop
```
