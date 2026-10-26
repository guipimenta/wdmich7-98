@echo off
rem Saves the HD Audio driver codec snapshots to C:\HDASTATE.REG
regedit /e C:\HDASTATE.REG HKEY_LOCAL_MACHINE\Enum\PCI
echo Saved C:\HDASTATE.REG
