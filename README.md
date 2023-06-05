# simple linux keylogger

## disclaimer

in practice, this software _will_ be detected. you wont get any use from deploying
this. this is just meant as a demonstration on how malware for linux can be written,
and to practice. i have decided to open-source this so that others can learn from it
too but it would be **extremely dumb** from you to deploy it and use it against
an oblivious victim (and also very illegal). Please only use this to learn about
Linux's internals, like I have and run it in a Virtual Machine or a Linux environment
**you** own that is safe to infect.

I do not endorse the use of security tools for malicious use, nor do i wish to be
responsible for your stupidity. thank u <3

## overall design & explanation

> I plan to release a blog-post about this pretty soon [here](https://djnn.sh) !


### Logging keys

There is many steps to do a project of the sort.

First, you will need to find a keyboard of course !
For this, I invite you to check on your Linux install the directory `/dev/input/by-path/`
and you should see a symlink ending with `kbd`.

That's your keyboard. You can see that it points to an event in the
parent directory.

You should be able to find this event using the `readlink()` function.

You can also take into account that there might be multiple keyboards, because
of an USB keyboard plugged in, for instance.

To circonvent that, I chose to store every file descriptor matching the description
of a keyboard, and use the `select` syscall to find one when I can read on one of them.
I invite you to read the man page of select for more information about this.

To read the keyboard(s), you can just use `read` but you might also need to take
into account that not every user has a `QWERTY` keyboard. For simplicity, we use
the `libxkbcommon` (because once again, we are just striving for a demonstration here)
but you might get away with just implementing a key table yourself if you are trying
to keep it simple. You could also try to load handles from existing libraries on the
system and use that depending on what is installed, but this will heavily complicate
the logic of the program, and i just didnt feel like it.

Then, I have implemented a message queue, so that reports of the keys are not
spammed through the wire as much.

### communications


### obfuscation, anti-debug, etc...

## credits, shoutouts, etc

- [cpl0](https://cpl0.zip) & wintermute -- for providing me motivation to rework this
- [jenaye](https://jenaye.fr) -- for encouraging me to get into maldev :)~
