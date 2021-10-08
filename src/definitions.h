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

#ifndef XF86_INPUT_KURIUSERSPACE_DEFINITIONS_H
#define XF86_INPUT_KURIUSERSPACE_DEFINITIONS_H

#include <xorg/xf86.h>
#include <xorg/xf86Xinput.h>
#include <xorg/xf86Module.h>
#include <xorg/xf86_OSproc.h>
#include <linux/input.h>

#define BUFFER_SIZE 256
#define MAX_EVENTS 1024

struct KuriDeviceState {
    InputInfoPtr pInfo;
    int x;
    int y;
    int pressure;
    int tiltx;
    int tilty;
};

struct KuriDeviceRec {
    char* name;
    struct KuriDeviceRec* next;
    InputInfoPtr pInfo;
    int debugLevel;

    int topX;
    int topY;
    int bottomX;
    int bottomY;
    int resolutionX;
    int resolutionY;

    int naxes;

    struct KuriCommonRec* common;

    int nPressCtrl[4];
};

struct KuriCommonRec {
    int kuriPktLength;

    int bufpos;
    unsigned char buffer[BUFFER_SIZE];

    int eventCount;
    struct input_event events[MAX_EVENTS];

    struct KuriDeviceState* state;
};

#endif //XF86_INPUT_KURIUSERSPACE_DEFINITIONS_H
