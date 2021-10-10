# Kuri's Linux X11 input driver

This is a companion driver to https://github.com/kurikaesu/userspace-tablet-driver-daemon.
Where the daemon interprets the raw USB device packets, this driver generates the X11 events required to move the cursor, register pen touches, tilt etc.

This is a WIP driver and is not ready for production use.

## Why this driver?
This driver is an attempt to decouple Kuri's user-space drivers from the Wacom specific drivers. While the Wacom drivers have so far worked quite well, it is specifically designed for Wacom tablets and we've had to use a bunch of hacky work-arounds in order to support tablets from other manufacturers.

Ultimately this X11 input driver is designed to handle a specific set of messages that are universal to all tablets that the user-space driver creates and may also become usable by the Digimend project in order to better support tablets that have a large amount of shortcut/express keys and assortments of dials/touch-strips.

## Todo
- Finish implementing the p0 version of the driver
  - Bind the stylus tip and buttons to default actions
- p1 version
  - Support tablet orientation (ex. Left-handed mode)
  - Apply binding configuration generated/used by the user-space driver and GUI. This way the user-space driver no longer has to apply the bindings and can send raw event codes like BTN_0 instead.