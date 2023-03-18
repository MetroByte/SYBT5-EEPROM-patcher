# APC Symmetra SYBT5 EEPROM patcher
 
Patches binary dumps for re-program it back into the battery block controller's EEPROM chip.

Removes error records by zeroing corresponding bytes. Optionally it can change manufacture date and serial number (with checksum recalculation).

This is a pure C remake of the Python implementation: https://github.com/atmega1337/sybt5fixtool

# Usage

```
sybt5patcher.exe eeprom_dump.bin
```

Or just drag-and-drop EEPROM dump file on top of the executable *(Windows)*.

It will ask you for *Serial Number* and *Manufacture date* change. You can confirm that or reject *(default)*, if you need to change S/N and manufacture date. 

At the end, it will create a patched file in the current working directory, named with suffix `-fix.bin`.

# Binaries

Available in the Releases section. If you need another architecture or OS, you can build a source for your specific architecture or OS.

# Building

Source code is written with C89 compatibility.

It can be compiled on *Windows*, *Linux* and *DOS* with appropriate building tools.

## with `makefile`:

```
make
```

## Manually

### With gcc:
```
gcc -std=c89 -O2 main.c -o sybt5patcher
```

*On Windows, change `sybt5patcher` to `sybt5patcher.exe`*
### With MSVC:
```
cl /O2 /Fosybt5patcher.obj main.c /Fesybt5patcher.exe
```

### DOS:

Using `Turbo C` or another tools, such as [DJGPP](http://www.delorie.com/djgpp).

# Warning
* **Use it at your own risk**
* **Exitsting files will be overwritten!**
* DOSes have filename length limitation of 8 characters (excluding extension). If you are planning to use this software in DOS, make sure your input EEPROM dump file name is shorter than 8 characters.

## Short note about flashing with CH341A

EEPROM 93Cxx chips require special pin redirection. Do not put it right away onto CH341A without proper preparation.

Pins numeration on 93Cxx chips (and for BIOS part of CH341A programmer):

| Left | Right |
|------|-------|
| 1    | 8     |
| 2    | 7     |
| 3    | 6     |
| 4    | 5     |

## Pins redirection

**Left column:** from chip

**Right column:** to CH341A

**Empty:** don't connect

| 93Cxx | CH341A |
|-------|--------|
| 1     | 1      |
| 2     | 6      |
| 3     | 5      |
| 4     | 2      |
| 5     | 4      |
| 6     |        |
| 7     |        |
| 8     | 8      |
