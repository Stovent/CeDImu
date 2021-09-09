# CeDImu User Manual

This document aims to provide help and information on how to use CeDImu.

## Configuration

To configure CeDImu, open the settings by clicking the "Options -> Settings" menu.
A window will appear with all the configuration settings.

The reference website is the [ICDIA player comparison](http://icdia.co.uk/players/comparison.html), which lists the configuration to be used by each type of BIOS. (The PCB column is the board type).

Once you are done configuring, click "Save" to save the settings to the "CeDImu.ini" file.

Click "Cancel" or close the Settings window to discard your changes.

### General

- Discs directory : this is the directory that will appear when selecting the "File -> Open disc" menu.
- system BIOS : this is the path to the BIOS file to use.
- Board : the board (or PCB) used by the BIOS as listed on ICDIA, or "Auto" to let CeDImu try to guess it.
- 32 KB NVRAM : if the BIOS uses 32KB of NVRAM (see ICDIA), then this has to be checked. In "Auto" board type, CeDImu tries to detect the correct NVRAM size.
- PAL : if checked, BIOS will be emulated in PAL video mode. If unchecked, NTSC mode is used.
- Initial time : the initial time given to the BIOS. The given time is a [UNIX timestamp](https://en.wikipedia.org/wiki/Unix_time).
  Leave it empty to use the current real-life time. Enter "0" to continue from the time saved in the NVRAM.

### Controls

Click on the button of the control you want to change, then press the appropriate key on the keyboard.

