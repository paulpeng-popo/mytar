# tar - a simple tar implementation

This project is inspired by the [calccrypto/tar](https://github.com/calccrypto/tar)

Reference:

- [Wikipedia:tar](<https://en.wikipedia.org/wiki/Tar_(computing)>)
- [GNU tar](https://www.gnu.org/software/tar/manual/html_node/Standard.html)
- [tar.h (IBM)](https://www.ibm.com/docs/en/aix/7.3?topic=files-tarh-file)

**The program should run on the Linux/Unix system.**

## To build

    make         - prepare to execute
    make clean   - delete intermediate files

## Usage

    ./mytar -[ctxv] <tarfile> [folder | file1, file2, ...]

    Options:
        -v for verbose informations.
    Archive:
        ./mytar -c <tarfile> [folder | file1, file2, ...]
    Extract:
        ./mytar -x <tarfile>
    List
        ./mytar -tv <tarfile>

## Example

    drwxr-xr-x 0 pengpaul staff          0 Mar  2 21:45 testdata
    -rw-r--r-- 0 pengpaul staff        226 Mar  2 19:15 testdata/Makefile
    -rw-r--r-- 0 pengpaul staff      62912 Mar  2 21:45 testdata/tar.o
    -rw-r--r-- 0 pengpaul staff       1615 Mar  2 21:45 testdata/tar.h
    -rw-r--r-- 0 pengpaul staff       1945 Mar  2 21:22 testdata/mytar.cpp
    -rw-r--r-- 0 pengpaul staff       1209 Mar  2 18:19 testdata/README.md
    -rw-r--r-- 0 pengpaul staff       9542 Mar  2 21:45 testdata/tar.cpp
    -rw-r--r-- 0 pengpaul staff         10 Mar  2 14:33 testdata/.gitignore
    -rwxr-xr-x 0 pengpaul staff     125304 Mar  2 21:45 testdata/mytar
    -rw-r--r-- 0 pengpaul staff      69216 Mar  2 21:45 testdata/mytar.o
    -rwxr-xr-x 0 pengpaul staff       1257 Mar  2 21:43 testdata/test.sh

## TODOs

- [ ] long file name support
  - prefix
  - pax header
- [ ] devmajor, devminor: to support the device file
- [ ] lname (link name): to support the symbolic link
