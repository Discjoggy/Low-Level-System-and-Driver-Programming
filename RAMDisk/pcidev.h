/* 
 * File:        :   pcidev.h
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   10.12.2014, 17:11
 * Description  :   Algorithmus: Strassen. Parallelisierung mit tbb.
 */
#ifndef PCIDEV_H
#define	PCIDEV_H

#include <linux/pci.h>

static dev_t pci_dev_nr;
static struct class *pci_cls;

static struct pci_device_id pci_drv_tbl[] __initdata_or_module = {
    {       
        .vendor         = VENDOR_ID,    // Hersteller ID
        .device         = DEVICE_ID,    // Geraete ID
        .subvendor      = PCI_ANY_ID,   // Hersteller-Subsystem ID
        .subdevice      = PCI_ANY_ID,   // Geraete-Subsystem ID
        .class          = 0,            // Geraeteklasse
        .class_mask     = 0,            // Maske fuer die Geraeteklasse
        .driver_data    = 0,            // Treiberspezifische Daten
    },                  
    {
        0, 
    },
};

/**
 * Registriert das FPGA-PCI-Device.
 * @param pdev  PCI-Device
 * @param id    PCI-Device-ID
 * @return      0 falls das Device erfolgreich registriert wurden, andernfalls != 0.
 */
static int pci_probe(struct pci_dev *pdev, const struct pci_device_id *id) {
    int i;
    struct device *pci_dev;
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    
    if (pci_enable_device(pdev) < 0) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | Error when enabling device\n", __FUNCTION__, __LINE__);
        return -EIO;
    }
    
    if ((v_addr_bar = (unsigned long)pci_ioremap_bar(pdev, 0)) == 0) {
        DEBUG_MSG(KERN_DEBUG "<%s, %d> | Error when ioremap\n", __FUNCTION__, __LINE__);
        goto f_ioremap;
    }
    
    mem_start  = pci_resource_start(pdev, 0);
    mem_end    = pci_resource_end(pdev, 0);

    if (request_mem_region(mem_start + DEV_DDR_DATA, mem_end - (mem_start + DEV_DDR_DATA) + 1, pdev->dev.kobj.name) == NULL) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | Memory address conflict for device\n", __FUNCTION__, __LINE__);
        dev_err(&pdev->dev, "Memory address conflict for device\n");;
        goto f_mem_region;   
    }
 
    pci_set_master(pdev);
    
    if (IS_ERR(pci_cls = class_create(THIS_MODULE, DRIVER_NAME))) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | No udev support avaiable\n", __FUNCTION__, __LINE__);
        pr_err("No udev support avaiable\n");
        goto f_class_create;
    }
    
    for (i = 0; i < MINOR_CNT; ++i) {
        if (IS_ERR(pci_dev = device_create(pci_cls, NULL, pci_dev_nr + i, NULL, DRIVER_NAME "_%d", i))) {
            DEBUG_MSG(KERN_ERR "<%s, %d> | Failed to create class no. %d\n", __FUNCTION__, __LINE__, i);
            goto f_device_create;
        }
    }

    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done [Test I/O (Vers.): %X]\n", __FUNCTION__, __LINE__, ior(DEV_VER_DATA));
    return 0;
    
    f_device_create:
        class_unregister(pci_cls);
        class_destroy(pci_cls);
        
    f_class_create:
    f_mem_region:
        iounmap((void *)v_addr_bar);

    f_ioremap:
        pci_disable_device(pdev);

	return -EIO;
}

/**
 * Deregistriert das FPGA-PCI-Device.
 * @param pdev  PCI-Device
 */
static void pci_remove(struct pci_dev *pdev) {
    int i;
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);    
    
    iow(DEV_DDRC_SEGMENT, 0);
    for (i = 0; i < MINOR_CNT; ++i) {
        device_destroy(pci_cls, pci_dev_nr + i);
    }
    class_unregister(pci_cls);
    class_destroy(pci_cls);
    release_mem_region(mem_start + DEV_DDR_DATA, mem_end - (mem_start + DEV_DDR_DATA) + 1);
    if (v_addr_bar) {
        iounmap((void *)v_addr_bar);
    }
    pci_disable_device(pdev);

    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}

static struct pci_driver pci_drv = {
    .name       = DRIVER_NAME,      // Name des Geraetes
    .probe      = pci_probe,        // Registrierungs-Funktion
    .remove     = pci_remove,       // Remove-Funktion
    .id_table   = pci_drv_tbl,      // Geraetetreibertabelle
};

#endif
