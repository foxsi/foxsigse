This one has 10 frames with a random number of garbage words inserted between each frame.  (random garbage is between 0 and 100 words long.)  It turns out that the "mystery word" 60304 is EB90, so somehow the sync word from the next frame is getting included in your previous frame's data.  I made no changes there.  I did change the channel mask so it matches the simulated data, for consistency's sake.  :)

There may still be junk at the end of the file -- this doesn't show up when I use a program to read the file so I guess just ignore it?

Ok, got to go teach some exponents!  Let me know if there are questions...