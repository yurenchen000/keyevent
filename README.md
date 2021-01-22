# keyevent @ubuntu
a tool detect (at background, ubuntu) keyevent &amp; run cmd. 



on ubuntu  
 when vbox get focus, can't use `Ctrl+Alt+Left` to switch workspace.


this daemon tool, as one workaround:

- detect keyevent, then:
  - run cmd to switch workspace // depend `xdotool`

ref:
https://superuser.com/questions/108785/getting-host-to-capture-certain-key-presses-in-virtualbox/1611071

## choice eventX
ls -lh /dev/input/event*

//a. use `evtest`
```sh
sudo evtest
```

//b. use `xinput`
```sh
xinput list
sudo xinput test 17
```
