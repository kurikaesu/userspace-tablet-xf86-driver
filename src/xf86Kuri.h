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
