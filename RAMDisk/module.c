/* 
 * File:   module.c
 * Author: Tobias Sibera, Jens Overmoeller
 *
 * Created on 07. Februrary 2015, 02:47
 */
#include <linux/module.h>
#include "ramdisk.h"

static int inv_module_init(void) {
    int retval = 0;
    printk(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    if ((retval = ramdisk_init()) != 0) {
        printk(KERN_DEBUG "<%s, %d> | Error in ramdisk_init\n", __FUNCTION__, __LINE__);
        return retval;
    }
    printk(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
    return 0;
}

static void inv_module_exit(void) {
    printk(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    ramdisk_exit();
    printk(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}

module_init(inv_module_init);
module_exit(inv_module_exit);

MODULE_AUTHOR("Tobias Sibera, Jens Overmoeller");
MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("RAMDisc-Driver-Example");
MODULE_SUPPORTED_DEVICE(DRIVER_NAME);
