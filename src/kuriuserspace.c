#ifdef HAVE_CONFIG_H
#include <config.h>
#endif
#include <xorg/xf86.h>
#include <xorg/xf86Xinput.h>
#include <xorg/xf86Module.h>
#include <xorg/xorgVersion.h>

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

static void kuriUninit(InputDriverPtr drv, InputInfoPtr pInfo, int flags) {

}

static int kuriPreInit(InputDriverPtr drv, InputInfoPtr pInfo, int flags) {
    return 0;
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

_X_EXPORT XF86ModuleData kuriUserspaceModuleData =
        {
        &kuriUserspaceVersionRec,
        kuriUserspacePlug,
        kuriUserspaceUnplug
        };