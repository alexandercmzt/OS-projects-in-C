I ssh-ed into trottier and ran my program there, and got the following results:

test 1: 0 errors

test 2: 4-5 errors

test 3: fuse wrappers all working perfectly fine

As far as I am aware the overall program works without any issues.

NOTE!!
ls doesn't work at first for some reason using FUSE, but does work after adding 3-4 files!! I tested this with Jit and we didn't know what to do to fix this, but please don't assume ls doesn't work at all! If you add a couple files it starts working and the directory works like it should. That's the only issue we found with the program that we found at the office hours. I also messed around a little with test file 1 when trying to debug stuff (added some code to it) so you can replace that one when testing them.
