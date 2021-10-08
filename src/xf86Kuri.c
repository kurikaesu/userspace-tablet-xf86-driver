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
#include <exevents.h>

#include <xserver-properties.h>

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

void dispatchEvents(InputInfoPtr pInfo) {
    int valuators[6];
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;
    struct KuriDeviceState* state = common->state;

    valuators[0] = state->x;
    valuators[1] = state->y;
    valuators[2] = 0;
    valuators[3] = state->tiltx;
    valuators[4] = state->tilty;

    xf86Msg(X_INFO, "Valuators-> x: %d, y: %d\n", valuators[0], valuators[1]);


    // Start sending digitizer events
    xf86PostMotionEventP(pInfo->dev, 1, 0, 2, valuators);
}

void parseAbsEvent(struct KuriCommonRec* common, struct input_event* event) {
    switch (event->code) {
        case ABS_X:
            common->state->x = event->value;
            break;
        case ABS_Y:
            common->state->y = event->value;
            break;

        case ABS_RZ:
            break;

        case ABS_TILT_X:
            common->state->tiltx = event->value;
            break;

        case ABS_TILT_Y:
            common->state->tilty = event->value;
            break;

        case ABS_PRESSURE:
            common->state->pressure = event->value;
            break;

        default:
            break;
    }
}

void parseSynEvent(InputInfoPtr pInfo, const struct input_event* event) {
    struct input_event* handledEvent;
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;

    if (event->code != SYN_REPORT) {
        return;
    }

    for (int i = 0; i < common->eventCount; ++i) {
        handledEvent = common->events + i;
        if (handledEvent->type == EV_ABS) {
            parseAbsEvent(common, handledEvent);
        } else if (handledEvent->type == EV_REL) {
            xf86Msg(X_INFO, "REL Message\n");
        } else if (handledEvent->type == EV_KEY) {
            xf86Msg(X_INFO, "KEY Message\n");
        }
    }

    // Dispatch all the events we have queued
    dispatchEvents(pInfo);

    common->eventCount = 0;
}

void parseEvent(InputInfoPtr pInfo, const struct input_event* event) {
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;

    if (common->eventCount >= 1024) {
        xf86Msg(X_ERROR, "Exceeded event queue\n");
        return;
    }

    common->events[common->eventCount++] = *event;

    switch (event->type) {
        case EV_SYN:
            parseSynEvent(pInfo, event);
            break;

        default:
            break;
    }
}

int parsePacket(InputInfoPtr pInfo, const unsigned char* data, int len) {
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

static void initAxis(DeviceIntPtr device, int index, Atom label, int min, int max, int res, int min_res, int max_res, int mode) {
    InitValuatorAxisStruct(device, index, label, min, max, res, min_res, max_res, mode);
}

static int initAxes(DeviceIntPtr pDev) {
    InputInfoPtr pInfo = (InputInfoPtr)pDev->public.devicePrivate;
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;

    Atom label;
    int index;
    int min, max, min_res, max_res, res;
    int mode;

    // First valuator: x
    index = 0;
    label = XIGetKnownProperty(AXIS_LABEL_PROP_ABS_X);
    min = priv->topX;
    max = priv->bottomX;
    min_res = 0;
    max_res = priv->resolutionX;
    res = priv->resolutionX;
    mode = Absolute;

    initAxis(pInfo->dev, index, label, min, max, res, min_res, max_res, mode);

    // Second valuator: y
    index = 1;
    label = XIGetKnownProperty(AXIS_LABEL_PROP_ABS_Y);
    min = priv->topY;
    max = priv->bottomY;
    min_res = 0;
    max_res = priv->resolutionY;
    res = priv->resolutionY;
    mode = Absolute;

    initAxis(pInfo->dev, index, label, min, max, res, min_res, max_res, mode);

    return TRUE;
}

static int kuriDevProc(DeviceIntPtr pDev, int what) {
    InputInfoPtr pInfo = (InputInfoPtr)pDev->public.devicePrivate;
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    Atom axis_labels[MAX_VALUATORS] = {0};

    int rc = !Success;

    switch (what) {
        case DEVICE_INIT:
            if (InitFocusClassDeviceStruct(pInfo->dev) == FALSE) {
                xf86Msg(X_ERROR, "%s: unable to init Focus class device\n", pInfo->name);
                goto out;
            }

            if (InitProximityClassDeviceStruct(pInfo->dev) == FALSE) {
                xf86Msg(X_ERROR, "%s: unable to init proximity class device\n", pInfo->name);
                goto out;
            }

            if (InitValuatorClassDeviceStruct(pInfo->dev, priv->naxes, axis_labels, GetMotionHistorySize(), Absolute | OutOfProximity) == FALSE) {
                xf86Msg(X_ERROR, "%s: unable to allocate Valuator class device\n", pInfo->name);
                goto out;
            }
            if (initAxes(pDev) != TRUE)
                goto out;

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
