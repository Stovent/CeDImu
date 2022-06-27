# CeDImu User Manual

This document aims to provide help and information on how to use CeDImu.

The CD-i is technically only a disc format, which means any hardware that conforms to the Green Book (the CD-i specifications) can play CD-i discs. This means any manufacturer can create their own CD-i player with the hardware and software they want.

CeDImu is in fact a CD-i **player** emulator, which emulates only some of the players that were manufactured, but not all of them. The consequence is that each BIOS is associated to a given hardware configuration, and the BIOS and its configuration has to be provided by the user.

The list of the emulated hardware systems and the known BIOSes that work are available in the [README](https://github.com/Stovent/CeDImu/blob/master/README.md#compatibility) (Compatibility section).

## Configuration

To configure CeDImu, open the settings by clicking the "Options -> Settings" menu.
A window will appear with all the configuration settings.

The reference website is the [ICDIA player comparison](http://icdia.co.uk/players/comparison.html), which lists the configuration to be used by each type of BIOS. (The PCB column is the board type).

Once you are done configuring, click "Save" to save the settings in a file. The location of the settings file will depend on your OS. It is `C:/Users/<you>/AppData/Roaming/CeDImu.ini` on Windows and `/home/<you>/.CeDImu` on Linux.

The changes are not taken in consideration until you restart CeDImu, reload the BIOS in the "File -> Start BIOS" menu or click "Emulation -> Reload core".

Click "Cancel" or close the Settings window to discard your changes.

Once your BIOSes and controls are configured, boot the BIOS you want by selecting it in the "File -> Start BIOS" menu. If "Emulation -> Pause" is ticked, then the BIOS will not run until you untick it.

### General

- Discs directory : this is the directory that will appear when selecting the "File -> Open disc" menu.

### BIOS

Since there is a lot of possible BIOS/hardware configurations, you do not configure a single BIOS but instead a list of BIOSes. On the first boot of CeDImu, there are no BIOS configured.

To add a new configuration, click the "New" button and it will ask you a name for the new configuration. Then fill in the fields :

- BIOS file : the path to the BIOS ROM file. Select it by clicking on the "Select BIOS file" button.
- Associated NVRAM filename : This is the name that will be used to store the NVRAM content when exiting the emulator.
- Board : the board (or PCB) used by the BIOS as listed on ICDIA, or "Auto" to let CeDImu try to guess it.
- PAL : if checked, BIOS will be emulated in PAL video mode. If unchecked, NTSC mode is used.
- 32 KB NVRAM : if the BIOS uses 32KB of NVRAM (see ICDIA), then this has to be checked. In "Auto" board type, CeDImu tries to detect the correct NVRAM size.
- Initial time : the initial time given to the BIOS, in [UNIX timestamp](https://en.wikipedia.org/wiki/Unix_time) format.
  Leave it empty to use the current real-life time. Enter "0" to continue from the time saved in the NVRAM.

### Controls

Click on the button of the control you want to change, then press the appropriate key on the keyboard.

