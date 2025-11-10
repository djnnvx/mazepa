~ simple linux keylogger

# 1. disclaimer

in practice, this software _will_ be detected. you wont get any use from deploying
this. this is just meant as a demonstration on how malware for linux can be written,
and to practice. i have decided to open-source this so that others can learn from it
too but it would be **extremely dumb** from you to deploy it and use it against
an oblivious victim (and also very illegal). Please only use this to learn about
Linux's internals, like I have and run it in a Virtual Machine or a Linux environment
**you** own that is safe to infect.

I do not endorse the use of security tools for malicious use, nor do i wish to be
responsible for your stupidity. thank u <3

# 2. overall design & explanation

> I plan to release a blog-post about this pretty soon [here](https://djnn.sh) !


## 2.a Logging keys

There is many steps to do a project of the sort.

First, you will need to find a keyboard of course !
For this, I invite you to check on your Linux install the directory `/dev/input/by-path/`
and you should see a symlink ending with `kbd`.
That's your keyboard. You can see that it points to an event in the
parent directory.

You should be able to find this event using the `readlink()` function.
You can also take into account that there might be multiple keyboards, because
of an USB keyboard plugged in, for instance.

## 2.b persistence

You can create a `systemd` service like so, in order to have it running at
each reboot, if you'd like.

```systemd
# Create a malicious systemd unit
[Unit]
Description=Keylogger

[Service]
ExecStart=/usr/bin/implant

[Install]
WantedBy=multi-user.target
```

This is a fairly common technique, as is appending in `~/.profile` or `~/.bashrc`
a call to your keylogger.

Another method is compiling the program as a dynamic library (`.so`), and abusing the
`LD_PRELOAD` / `/etc/ld.so.preload` trick to have it being ran every time the function
you're hooking to is being called.

However, this will be left as an exercise to you. :)~

> Here is an [excellent resource](https://pberba.github.io/security/2022/02/06/linux-threat-hunting-for-persistence-initialization-scripts-and-shell-configuration/) on persistence for linux

# 3. debug mode

A `debug` mode is supported out of the box, to allow for a simpler development.
To compile a debug binary, run `make fclean && make debug` in the `./implant`
directory.

It will allow you to troubleshoot memory issues and enable some logging and nice-to-have
features. Here is the full detail here:
* debug logs on error (either through `perror()` or using a simple logging system)
* add support for a `-h` argument when running the binary, that explains the usage
* compile with debug flags enabled, in order to use `valgrind` etc

