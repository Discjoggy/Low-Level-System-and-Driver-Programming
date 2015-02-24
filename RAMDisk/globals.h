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

#define DRIVER_NAME             "CFPGA" // Geraetename
#define VENDOR_ID               0x10EE  // Register der Hersteller ID
#define DEVICE_ID               0x0007  // Register der Geraete ID
#define MINOR_FIRST             0U      // Erste Minor-Nr.
#define MINOR_CNT               1U      // Anzahl der Minors

#define USE_VMALLOC             1       // 1: Betriebssystem-RAMs verwenden
#define USE_QUEUE               0       // 1: Treiberoptimierte Variante
#define DEBUG                   1       // 1: Debugging einschalten
#ifdef DEBUG
#define DEBUG_MSG(format, msg...) printk(format, ## msg)
#else
#define DEBUG_MSG(format, msg...)
#endif

#define DEV_DDR_DATA            0x80000 // 1 Segment (512 KiB) des dyn. Speichers
#define DEV_DDRC_SEGMENT        0x02700 // Segmentauswahl fuer dyn. Speicher
#define DEV_VER_DATA            0x02500 // Versionsinfos des FPGAs

static int major                = 0U;   // Major-Nr.
static unsigned long v_addr_bar = 0UL;  // Ph. gemappter PCI-Adressanfang 
static unsigned long mem_start  = 0UL;  // Anfang des PCI-Speichers
static unsigned long mem_end    = 0UL;  // Groesse des PCI-Speichers

/**
 * Schreibt einen Wert in die mit übergebene Adresse.
 * @param address  Adresse in die der Wert geschrieben werden soll
 * @param value    Wert der in die Adresse geschrieben werden soll
 */
static void iow(unsigned long address, unsigned int value) {
    //wmb(); 
    iowrite32(value, (void *)v_addr_bar + address);
}

/**
 * Gibt den Wert der übergebenen Adresse zurück.
 * @param address  Adresse aus der der Wert ausgelesen werden soll
 * @return         Wert der Adresse
 */
static unsigned int ior(unsigned long address) {
    //rmb(); 
    return ioread32((void *)v_addr_bar + address);
}

#endif
