# CentralNode 7.1

## 7111 notice
This is a <b>TESTING</b> build, this program is <b>NOT</b> complete.

## Info
So I've decided to make this spaghetti public, in case there are good souls willing to test it and hunt for bugs.  
They are all over the place, creeping from the bushes, waiting to throw `SEGFAULTS` at people.  
If you find a bug, please create a [new issue](https://github.com/x0reaxeax/CentralNode71/issues/new/choose),  
or shoot me a message on Discord `xoreaxeax#9705`  
Thank you very much.
  

## Supported terminals:
#### Feel free to create a pull request if you find additional terminals that work with CNODE
`gnome-terminal`  
`terminator`  
`WSL terminal`  
`rxvt`  
`rxvt-unicode`  

## Unsupported terminals:
#### These terminals don't support changing color palettes, if you find any new, fire up a pull request. Thank you
`qterminal`     - qterminal is demented, along with it's color palette that cannot be changed (at least I couldn't find it in preferences)  
`xterm`         - raw xterm doesn't seem to support ncurses colors  
`konsole`       - :( that's just sad and depressing  

## Dependencies:
`uuid-dev`  
`libcurl4`  
`ncurses`  
`libssl`  

## Building tools:
`make`  
`nasm`  
`gcc`  

## Building:
`make`  

## Code notes:

### Coding style:
As of right now, just stick to these few basic rules, everything will be polished either way when complete
1. Keep the opening curly bracket on the same line as function definitions.
2. `case` is indented by one level from `switch`:
```c
switch (number) {
    case 1:
        break;
    default:
        break;
}
```
3. `NODE_SETLASTERR()` needs to be the first thing called on error, inside the function where the error happened.

### Spaghetti
I know this code is a lot of spaghetti bolognese, but at this point, I just wanna push it out to (semi) functional state while slowly working on untangling it all, piece by piece.  
If you feel like improving some code, shoot a pull request  

### Include files
I know this may sound non-standard, but internal headers are included before system headers, so please follow that, or stuff will go break break (wow, wonderful) 

### NODE_SETLASTERR() rule:
As mention above, this is always called inside the function where error happens, never outside of it.  
`NODE_SETLASTERR()` calls `log_write(LOG_ERROR, ...)` and automataically logs errors, if possible.  
`log_write_trace()` only logs messages with `DEBUG` logging level.  

### Memory leaks
As of build 7111, all memory allocations are free'd. Valgrind will show some lost bytes, but all of them should be tied to ncurses window functions. This is NOT a leak.
Address Sanitizer is set-up in `Makefile`, please test Central Node with ASAN present.

### `log_write()` warning
`log_write()` does NOT check for erroneous format specifiers, neither does the compiler. This is why logging should be done by `NODE_SETLASTERR()`, but if it's necessary, always double check the stuff and arguments you feed `log_write()` with.  

### TODO:
- [ ] tinc daemon ctl
- [ ] tinc config exchange system
- [ ] Config editing from TUI
- [ ] Panic mode
- [ ] nodeWD

## Additional notes
There is an issue present with generation 2 of Windows Subsystem Linux, where the system reports an incorrect number of network interfaces,
which are passsed to `ioctl()` by Central Node. This results in the following error message being written to the log file over and over again:  
`[Mon Jan 01 12:00:00 00]-[ERR]: I/O control operation failed: Cannot assign requested address`  
This issue is not present on generation 1 WSL. I'll figure out a solution to this soon.


## FAQ
See [FAQ](https://centralnode.xyz/faq.html)

## DISCLAIMER
This is software is licensed under the **GNU General Public License v3.0** (hereinafter referred to as "License").  
Permission to distribute this software is granted under the obligation of disclosing the source code and documenting **ALL** changes made to it.  
The license does **NOT** apply to any standalone server code designed to communicate with this software.  
Central Node is NOT affiliated or associated with any standalone server code designed to communicate with it. Therefore, in addition to the license, under no circumstances can the creators and/or contributors associated with this software be held responsible or liable in any way for any claims, damages, losses, expenses, costs, or liabilities linked to or caused by **ANY** remote code designed to communicate with this software.  

Copyright (c) 2022 x0reaxeax
Central Node and centralnode.xyz are hosted and powered by VegaLabs (vegalabs.org)