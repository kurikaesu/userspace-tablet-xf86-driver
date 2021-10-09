/*
xf86-input-kuriuserspace
Copyright (C) 2021 - Aren Villanueva <https://github.com/kurikaesu/>

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef XF86_INPUT_KURIUSERSPACE_XF86KURI_H
#define XF86_INPUT_KURIUSERSPACE_XF86KURI_H

#include <xorg/xf86.h>
#include <xorg/xf86Xinput.h>
#include <xorg/xf86Module.h>
#include <xorg/xf86_OSproc.h>
#include <linux/input.h>

#define AC_CODE             0x0000ffff	/* Mask to isolate button number or key code */
#define AC_KEY              0x00010000	/* Emit key events */
#define AC_MODETOGGLE       0x00020000	/* Toggle absolute/relative mode */
#define AC_DBLCLICK         0x00030000	/* DEPRECATED: use two button events instead */
#define AC_DISPLAYTOGGLE    0x00040000  /* DEPRECATED: has no effect (used to toggle among screens) */
#define AC_PANSCROLL        0x00050000  /* Enter/exit panscroll mode */
#define AC_BUTTON           0x00080000	/* Emit button events */
#define AC_TYPE             0x000f0000	/* The mask to isolate event type bits */
#define AC_KEYBTNPRESS      0x00100000  /* bit set for key/button presses */
#define AC_CORE             0x10000000	/* DEPRECATED: has no effect */
#define AC_EVENT            0xf00f0000	/* Mask to isolate event flag */

struct KuriModule {
    InputDriverPtr dev;
    void (*DevReadInput)(InputInfoPtr pInfo);
    int (*DevProc)(DeviceIntPtr pDev, int what);
};

extern struct KuriModule kuriModule;

int kuriReadPacket(InputInfoPtr pInfo);
void parseEvent(InputInfoPtr pInfo, const struct input_event* event);
int parsePacket(InputInfoPtr pInfo, const unsigned char* data, int len);

#endif //XF86_INPUT_KURIUSERSPACE_XF86KURI_H
