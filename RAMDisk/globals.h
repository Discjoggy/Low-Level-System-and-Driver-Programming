/* 
 * File:        :   globals.h
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   07.01.2015, 12:11
 * Description  :   Enthält globale Variablen sowie hilfreiche Funktionen.
 */
#ifndef GLOBALS_H
#define	GLOBALS_H
#include "ioctl_cmds.h"

// Important
#define DRIVER_NAME         "CFPGA"
#define VENDOR_ID           0x10EE
#define DEVICE_ID           0x0007
#define MINOR_FIRST         0U
#define MINOR_CNT           1U

// Options
#define USE_VMALLOC         1
#define USE_QUEUE           0
#define DEBUG               1
#ifdef DEBUG
#define DEBUG_MSG(format, msg...) printk(format, ## msg)
#else
#define DEBUG_MSG(format, msg...)
#endif

// Registers
#define DEV_DDR_DATA        0x80000 // 1 Segment (512 KiB) des dyn. Speichers
#define DEV_DDRC_SEGMENT    0x02700 // Segmentauswahl für dyn. Speicher
#define DEV_VER_DATA        0x02500 // Versionsinfos des FPGAs

// PCI-Dev Globals
static int major                    = 0U;
static unsigned long v_addr_bar     = 0UL; 
static unsigned long mem_start      = 0UL;
static unsigned long mem_end        = 0UL;
        
// Helper-Functions
/**
 * Schreibt einen Wert in die mit übergebene Adresse.
 * @param address  Adresse in die der Wert geschrieben werden soll
 * @param value    Wert der in die Adresse geschrieben werden soll
 */
static void iow(unsigned long address, unsigned int value) {
    //wmb(); 
    writel(value, (void *)v_addr_bar + address);
}

/**
 * Gibt den Wert der übergebenen Adresse zurück.
 * @param address  Adresse aus der der Wert ausgelesen werden soll
 * @return         Wert der Adresse
 */
static unsigned long ior(unsigned long address) {
    //rmb(); 
    return readl((void *)v_addr_bar + address);
}

#endif
