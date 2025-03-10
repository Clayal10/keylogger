# keylogger
This is a kernel module for logging potential passwords by analyzing all keystrokes made on the keyboard.

## Use it!
To use this module, run the following in the src/ directory:
```bash
make
sudo insmod keylog.ko
```
To remove it:
```bash
sudo rmmod keylog
make clean
```
>Note: this is for educational purposes and not to be used
>for nefarious means.
