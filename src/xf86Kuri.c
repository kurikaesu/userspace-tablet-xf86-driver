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

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include "xf86Kuri.h"
#include "definitions.h"
#include <unistd.h>
#include <errno.h>
#include <linux/input.h>

static void kuriDevReadInput(InputInfoPtr pInfo);
static int kuriDevProc(DeviceIntPtr pDev, int what);

struct KuriModule kuriModule =
{
        NULL,

        kuriDevReadInput,
        kuriDevProc,
};

static void kuriDevReadInput(InputInfoPtr pInfo) {
    int loop;
    const int maxReads = 10;

    for (loop = 0; loop < maxReads; ++loop) {
        if (!kuriReadPacket(pInfo))
            break;
    }
}

void parseEvent(InputInfoPtr pInfo, const struct input_event* event) {
    xf86Msg(X_INFO, "Event: type:(%d) code:(%d) value:(%d)\n", event->type, event->code, event->value);
}

int parsePacket(InputInfoPtr pInfo, const unsigned char* data, int len) {
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;
    struct input_event event;

    if (len < sizeof(struct input_event)) {
        return 0;
    }

    memcpy(&event, data, sizeof(event));
    parseEvent(pInfo, &event);
    return sizeof(struct input_event);
}

int kuriReadPacket(InputInfoPtr pInfo) {
    int len, remaining, cnt, pos;
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;

    int n = xf86WaitForInput(pInfo->fd, 0);
    if (n > 0) {
        remaining = sizeof(common->buffer) - common->bufpos;

        len = xf86ReadSerial(pInfo->fd, common->buffer + common->bufpos, remaining);

        if (len <= 0) {
            if (errno == ENODEV) {
                xf86RemoveEnabledDevice(pInfo);
            }
            return FALSE;
        }

        common->bufpos += len;
        len = common->bufpos;
        pos = 0;

        while (len > 0) {
            cnt = parsePacket(pInfo, common->buffer + pos, len);
            if (cnt <= 0) {
                break;
            }
            pos += cnt;
            len -= cnt;
        }

        common->bufpos = len;

        return TRUE;
    }

    return FALSE;
}

static int kuriDevProc(DeviceIntPtr pDev, int what) {
    InputInfoPtr pInfo = (InputInfoPtr)pDev->public.devicePrivate;

    int rc = !Success;

    switch (what) {
        case DEVICE_INIT:

            break;

        case DEVICE_ON:
            xf86Msg(X_INFO, "%s: On.\n", pInfo->name);
            if (pDev->public.on) {
                break;
            }
            pInfo->fd = xf86OpenSerial(pInfo->options);
            if (pInfo->fd < 0) {
                xf86Msg(X_ERROR, "%s: cannot open device.\n", pInfo->name);
                return BadRequest;
            }

            xf86FlushInput(pInfo->fd);
            xf86AddEnabledDevice(pInfo);
            pDev->public.on = TRUE;

            break;

        case DEVICE_OFF:
            xf86Msg(X_INFO, "%s: Off.\n", pInfo->name);
            if (!pDev->public.on) {
                break;
            }
            xf86RemoveEnabledDevice(pInfo);
            close(pInfo->fd);
            pInfo->fd = -1;
            pDev->public.on = FALSE;
            break;

        case DEVICE_CLOSE:

            break;

#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) * 100 + GET_ABI_MINOR(ABI_XINPUT_VERSION) >= 1901
        case DEVICE_ABORT:
            break;
#endif

        default:
            xf86Msg(X_ERROR, "Invalid mode.");
            goto out;
    }

    rc = Success;

out:

    if (rc != Success) {
        // Something bad happened
        xf86Msg(X_ERROR, "Was not able to handle devProc");
    }

    return rc;
}
