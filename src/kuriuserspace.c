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
    pInfo->dev = NULL;
    pInfo->private = priv;

    priv->next = NULL;
    priv->pInfo = pInfo;

    priv->nPressCtrl[0] = 0;
    priv->nPressCtrl[1] = 0;
    priv->nPressCtrl[2] = 100;
    priv->nPressCtrl[3] = 100;

    priv->common = kuriNewCommon();

    return 1;
error:
    free(priv);
    return 0;
}

static void kuriUninit(InputDriverPtr drv, InputInfoPtr pInfo, int flags) {

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