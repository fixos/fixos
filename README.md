FiXos, alternative UNIX-like kernel for fx9860G 
===============================================

Informations
-------------

FiXos is a project of kernel, targetting Casio's calculators in a first place,
designed to be UNIX-like, even if full POSIX compatibility is _not aimed_.

As Casio's default OS on these platforms never switches to user mode, the kernel
can be started like an Addin and deals directly with protected mode.
A bootloader is included, and allows to run ELF files as a kernel (so this is not
really a "bootloader", the real boot is in all case done by Casio's code for
now).

The CPU primarly targeted is the SH3 7705 modified by Casio, used on fx9860, but
other SuperH support may be added later.
This CPU has a modern architecture, with priviligied/user modes, MMU for virtual
memory and memory access control, cache, embedded USB device controller...
All these things allow to have a modern kernel, with a good process isolation.

Finally, do not forget the goal is not to do an amazing kernel. I am working on
that because it's _fun_, really interesting, and to do some proof-of-concept.



Build and Install
-------------

**First of all** : be sure about what you are doing before to start!
This software is **not** a stable release of your favorite linux kernel. Any part
of the program may cause damage to your machine, especially if you are using it
on a not tested platform.
Of course, I assure you I have never issued important problem when testing the
kernel on my own calculator, but if for any reason this happens for you, you
are warned.


To build the project, you must use a GCC toolchain configured for SuperH targets.
In theory, the whole project use -nostdlib option when invoking GCC, so only
binutils and compiler itself is needed.
In addition, you must have a G1A Wrapper tool. Two tools exist : my own,
[C G1A  Wrapper][1] and the original version from Andreas Bertheussen, the
[fxSDK's G1A Wrapper][2] which is better for icons but depends on Qt 4 SDK.
Finaly, even if you can build everything manually, the project uses GNU make
to be build.

If you are using make tool, please check and modify the values of Makefile
variables for the different tool names and locations (in most case, only
"global.mk" need to be changed, or in simplest case you can add option
"TOOLCHAIN_PREFIX={prefix}" to make command).
Check root Makefile for the kernel, bootloader/Makefile for the bootloader and
user/Makefile for userspace things.

Once all is configured, simply execute :
```
make all
```
to build the kernel, bootloader and userland test programs.
If everything works, to run the kernel using the bootloader, you will need
to copy "bootloader/bootldr.g1a", "bootloader/bootldr.cfg", "fixos" and
"user/test.elf" on the root of your calculator SMEM file system.
Any shared library (\*.so file) should be put in a directory named "LIB" located
on the root of SMEM.

To run the kernel, start bootldr.g1a (its icon shows "FiXos"), and select the
first loadable entry with [EXE].



[1]: http://github.com/Kristaba/C-G1A-Wrapper
[2]: http://sourceforge.net/apps/trac/fxsdk/wiki/g1awrapper



Notes
----------

Due to a bug into GNU ld 2.21 and earlier, please use a more recent version.
The fixos.ld script was tested using binutils 2.22 (11/21/2011).
If you have some troubles like "fixos.ld:xx cannot move location counter backwards"
it's probably a ld problem.


