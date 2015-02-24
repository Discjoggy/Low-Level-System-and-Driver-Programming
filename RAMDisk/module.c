/* 
 * File:        :   module.c
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   07.02.2015, 02:47
 * Description  :   Enthält die Init- und Exit-Methoden zur (De-)Registierung des Treibers.
 */
#include <linux/module.h>
#include "ramdisk.h"

/**
 * Registriert den Treiber im Kernel.
 * @return  0 falls kein Fehler aufgetreten ist, andernfalls != 0.
 */
static int inv_module_init(void) {
    int retval = 0;
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    if ((retval = ramdisk_init()) != 0) {
        DEBUG_MSG(KERN_DEBUG "<%s, %d> | Error in ramdisk_init\n", __FUNCTION__, __LINE__);
        return retval;
    }
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
    return 0;
}

/**
 * Deregistriert den Treiber aus dem Kernel.
 */
static void inv_module_exit(void) {
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    ramdisk_exit();
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}

module_init(inv_module_init);
module_exit(inv_module_exit);

MODULE_AUTHOR("Tobias Sibera, Jens Overmoeller");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RAMDisk-Driver-Example");
MODULE_SUPPORTED_DEVICE(DRIVER_NAME);
