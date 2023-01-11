# svr4-src
The UNIX System V Release 4 source code

This repository contains the UNIX System V Release 4 source code. The following scripts are used to build the source code, as there is no central Makefile.

`:mk` - Builds everything. (commands, libraries, kernel, the `/usr/ucb` BSD-compatibility commands, etc.)

`mk.add-on` - I don't really have an idea for which purpose this is. Looking at the script it seems to be for a cross-compiler environment.

`:mk.arch` - Some machine-dependant code.

`:mkcmd` - Builds all commands.

`:mk.csds` - Builds the developer tools (C compiler, linker, assembler, etc.)

`:mk.fnd` - Builds the entire system execept for the CSDS (developer tools)

`:mkhead` - Installs headers in `/usr/include`.

`:mk.i386` - Builds for the i386 platform.

`:mklib` - Builds libraries (`libc`, `libadm`, `libpkg`, etc.)

`:mkoam` - Builds the OAM components.

`:mksyshead` - Installs system headers in `/usr/include/sys`.

`:mkucb` - Builds the `/usr/ucb` BSD-compatibility commands and libraries.

`:mkucbcmd` - Builds the `/usr/ucb` BSD-compatibility commands only.

`:mkucbhead` - Installs the headers for the `/usr/ucb` BSD-compatibility commands in `/usr/ucbinclude`.

`:mkucblib` - Builds the libraries for the `/usr/ucb` BSD-compatibility commands.

`:mkuts` - Builds the SysVr4 kernel.

`:mkxcp` - Builds the XENIX Compatibility Package.

`:mkxcpcmd` - Builds the commands of the XENIX Compatibility Package.

`:mkxcplib` - Builds the libraries of the XENIX Compatibility Package.

