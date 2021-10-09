#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <xorg/xf86.h>
#include <xorg/xf86Xinput.h>
#include <xorg/xf86Module.h>
#include <xorg-server.h>
#include "definitions.h"
#include "xf86Kuri.h"
#include "kuriCommon.h"

#define BITS_PER_LONG	(sizeof(long) * 8)
#define NBITS(x)	((((x)-1)/BITS_PER_LONG)+1)
#define LONG(x)		((x)/BITS_PER_LONG)
#define BIT(x)		(1UL<<((x) & (BITS_PER_LONG - 1)))
#define ISBITSET(x,y)	((x)[LONG(y)] & BIT(y))

static const char *default_options[] =
        {
        "StopBits", "1",
        "DataBits", "8",
        "Parity", "None",
        "Vmin", "1",
        "Vtime", "10",
        "FlowControl", "Xoff",
        NULL
        };

static int kuriAllocate(InputInfoPtr pInfo) {
    struct KuriDeviceRec* priv = NULL;

    priv = calloc(1, sizeof(struct KuriDeviceRec));
    if (!priv) {
        goto error;
    }

    pInfo->device_control = kuriModule.DevProc;
    pInfo->read_input = kuriModule.DevReadInput;
    pInfo->control_proc = NULL;
    pInfo->switch_mode = NULL;
    pInfo->dev = NULL;
    pInfo->private = priv;

    priv->next = NULL;
    priv->pInfo = pInfo;

    // Just hardcoding some dummy values for testing
    priv->naxes = 6;
    priv->maxCurve = 8191;

    priv->nPressCtrl[0] = 0;
    priv->nPressCtrl[1] = 0;
    priv->nPressCtrl[2] = 100;
    priv->nPressCtrl[3] = 100;

    priv->common = kuriNewCommon();
    priv->common->state = calloc(1, sizeof(struct KuriDeviceState));
    priv->common->state->pInfo = pInfo;

    return 1;
error:
    free(priv);
    return 0;
}

static void kuriUninit(InputDriverPtr drv, InputInfoPtr pInfo, int flags) {

}

int getRanges(InputInfoPtr pInfo) {
    struct input_absinfo absinfo;
    unsigned long ev[NBITS(EV_CNT)] = {0};
    unsigned long abs[NBITS(ABS_MAX)] = {0};
    struct KuriDeviceRec* priv = (struct KuriDeviceRec*)pInfo->private;
    struct KuriCommonRec* common = priv->common;

    if (ioctl(pInfo->fd, EVIOCGBIT(0, sizeof(ev)), ev) < 0) {
        xf86Msg(X_ERROR, "%s: unable to get ioctl event bits.\n", pInfo->name);
        return !Success;
    }

    if (!ISBITSET(ev, EV_ABS)) {
        xf86Msg(X_ERROR, "%s: no abs bits\n", pInfo->name);
        return !Success;
    }

    if (ioctl(pInfo->fd, EVIOCGBIT(EV_ABS, sizeof(abs)), abs) < 0) {
        xf86Msg(X_ERROR, "%s: unable to ioctl max values.\n", pInfo->name);
        return !Success;
    }

    if (ioctl(pInfo->fd, EVIOCGABS(ABS_X), &absinfo) < 0) {
        xf86Msg(X_ERROR, "%s: unable to ioctl xmax value.\n", pInfo->name);
        return !Success;
    }

    priv->topX = absinfo.minimum;
    priv->bottomX = absinfo.maximum;

    if (absinfo.resolution > 0) {
        priv->resolutionX = absinfo.resolution * 1000;
    }

    if (ioctl(pInfo->fd, EVIOCGABS(ABS_Y), &absinfo) < 0) {
        xf86Msg(X_ERROR, "%s: unable to ioctl ymax value.\n", pInfo->name);
        return !Success;
    }

    priv->topY = absinfo.minimum;
    priv->bottomY = absinfo.maximum;

    if (absinfo.resolution > 0) {
        priv->resolutionY = absinfo.resolution * 1000;
    }

    if (ISBITSET(abs, ABS_PRESSURE) && !ioctl(pInfo->fd, EVIOCGABS(ABS_PRESSURE), &absinfo)) {

    }

    xf86Msg(X_INFO, "%s: Size probed to X: %d Y: %d\n", pInfo->name, priv->bottomX, priv->bottomY);
    xf86Msg(X_INFO, "%s: Resolution probed to X: %d Y: %d\n", pInfo->name, priv->resolutionX, priv->resolutionY);

    return Success;
}

static int kuriPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags) {
    char *type, *device;

    device = xf86SetStrOption(pInfo->options, "Device", NULL);
    type = xf86SetStrOption(pInfo->options, "Type", NULL);

    if (!kuriAllocate(pInfo)) {
        return BadMatch;
    }

    if (!device) {
        return BadMatch;
    }

    free(type);
    getRanges(pInfo);

    return Success;
}

static InputDriverRec KURIUSERSPACE =
        {
        1,
        "kuriuserspace",
        NULL,
        kuriPreInit,
        kuriUninit,
        NULL,
#if GET_ABI_MAJOR(ABI_XINPUT_VERSION) >= 12
        default_options,
#endif
#ifdef XI86_DRV_CAP_SERVER_FD
        XI86_DRV_CAP_SERVER_FD,
#endif
        };

static pointer kuriUserspacePlug(pointer module, pointer options, int* errmaj, int* errmin) {
    xf86AddInputDriver(&KURIUSERSPACE, module, 0);
    return module;
}

static void kuriUserspaceUnplug(pointer module) {

}

static XF86ModuleVersionInfo kuriUserspaceVersionRec =
        {
            "kuriuserspace",
            MODULEVENDORSTRING,
            MODINFOSTRING1,
            MODINFOSTRING2,
            XORG_VERSION_CURRENT,
            PACKAGE_VERSION_MAJOR, PACKAGE_VERSION_MINOR, PACKAGE_VERSION_PATCHLEVEL,
            ABI_CLASS_XINPUT,
            ABI_XINPUT_VERSION,
            MOD_CLASS_XINPUT,
            {0, 0, 0, 0}
        };

_X_EXPORT XF86ModuleData kuriuserspaceModuleData =
        {
        &kuriUserspaceVersionRec,
        kuriUserspacePlug,
        kuriUserspaceUnplug
        };