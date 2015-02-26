/* 
 * File:        :   ramdisk.h
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   07.02.2015, 02:47
 * Description  :   Enthält die Logik des RAMDisk-Treibers.
 */
#ifndef RAMDISK_H
#define	RAMDISK_H

#include <linux/blkdev.h>
#include <linux/hdreg.h>
#include "globals.h"
#include "ramdisk_device.h"
#if !USE_VMALLOC
#include "pcidev.h"
#endif

#if USE_QUEUE
/**
 * Reagiert auf Schreib- oder Leseanfragen des Kernels.
 * @param q    Request-Queue
 * @param bio  Block-I/O-Struktur
 */
static void ramdisk_request(struct request_queue *q, struct bio *bio) {
    char *kaddr, *maddr;
    struct bio_vec *bvec;
    int segnr;
    sector_t mpage;

    blk_queue_bounce(q, &bio); // High-Memory-Support
    mpage = KERNEL_SECTOR_SIZE * bio->bi_sector;
    bio_for_each_segment(bvec, bio, segnr) {
        kaddr = bio_data(bio);
#if USE_VMALLOC
        maddr = Device.data + mpage;
#else
        //DEBUG_MSG(KERN_DEBUG "<%s, %d> | %lX||%lX||%lX||%lX||%lX %p\n", __FUNCTION__, __LINE__, (mpage >> 19), (mpage >> 18) & 0xFF, (mpage >> 17) & 0xFF, (mpage >> 16) & 0xFF, (mpage >> 15) & 0xFF, Device.data + mpage);
        iow(DEV_DDRC_SEGMENT, mpage);
        maddr = Device.data + (mpage & 0x7FFFF);
#endif
        //DEBUG_MSG(KERN_DEBUG "<%s, %d> | \n%p\n%p\n", __FUNCTION__, __LINE__, Device.data + (mpage & 0x7FFFF), Device.data + ior(DEV_DDRC_SEGMENT) * bio->bi_sector);
        //DEBUG_MSG(KERN_DEBUG "<%s, %d> | maddr-v_addr: %p || mpage: %lX || sec: %lu || len: %d \n", __FUNCTION__, __LINE__, (void *)(maddr - v_addr_bar), mpage, bio->bi_sector, bio->bi_size);
        if (bio_data_dir(bio) == READ || bio_data_dir(bio) == READA) {
            memcpy(kaddr, maddr, bio->bi_size);
        } 
        else {
            memcpy(maddr, kaddr, bio->bi_size);
        }
    };
    bio_endio(bio, 0);
}
#else
/**
 * Reagiert auf Schreib- oder Leseanfragen des Kernels.
 * @param q  Request-Queue
 */
static void ramdisk_request(struct request_queue *q) {
    char *kaddr, *maddr;
    struct req_iterator iter;
    struct bio_vec *bvec;
    struct request *req;
    sector_t mpage;
    
    req = blk_fetch_request(q);
    while (req != NULL) {
        if (req->cmd_type != REQ_TYPE_FS) {
            if (!__blk_end_request_cur(req, 0)) {
                DEBUG_MSG(KERN_NOTICE "<%s, %d> | continue req\n", __FUNCTION__, __LINE__);
                req = blk_fetch_request(q);
            }
            continue;
        }

        rq_for_each_segment(bvec, req, iter) {
            kaddr = page_address(bvec->bv_page) + bvec->bv_offset;
            mpage = KERNEL_SECTOR_SIZE * iter.bio->bi_sector;
#if USE_VMALLOC
            maddr = Device.data + mpage;
#else
            //iow(DEV_DDRC_SEGMENT, (mpage / LOGICAL_BLOCK_SIZE) << 19);
            iow(DEV_DDRC_SEGMENT, mpage);
            maddr = Device.data + (mpage & 0x7FFFF);
            //DEBUG_MSG(KERN_DEBUG "#1: %lu #2: %lu\n", mpage, ior(DEV_DDRC_SEGMENT));
            //DEBUG_MSG(KERN_DEBUG "<%s, %d> | maddr-v_addr: %p || mpage: %lX || sec: %lu || len: %d \n", __FUNCTION__, __LINE__, (void *)(maddr - v_addr_bar), mpage, iter.bio->bi_sector, bvec->bv_len); 
#endif
            if (bio_data_dir(iter.bio) == READ) {
                memcpy(kaddr, maddr, bvec->bv_len);
            }
            else {
                memcpy(maddr, kaddr, bvec->bv_len);
            }
        };
        
        if (!__blk_end_request_cur(req, 0)) {
            req = blk_fetch_request(q);
        }
    }
}
#endif

/**
 * Fängt IOCTLs ab und reagiert auf entsprechende CMDs.
 * @param block_device  PCI-Device
 * @param mode          Modus
 * @param cmd           Kommando
 * @param arg           Argument
 * @return              0 falls keine Fehler aufgetreten sind, andernfalls != 0.
 */
static int ramdisk_ioctl(struct block_device *block_device, fmode_t mode, unsigned cmd, unsigned long arg) {
    if (_IOC_TYPE(cmd) != IOCTLCMD_MAGIC || _IOC_NR(cmd) > IOCTLCMD_MAXNR) {
    	   DEBUG_MSG(KERN_WARNING "<%s, %d> | Wrong cmd: %u\n", __FUNCTION__, __LINE__, cmd);
        return -ENOTTY;
    }
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start, cmd: %u\n", __FUNCTION__, __LINE__, cmd);
    switch (cmd) {
        //case 21297: // What's it good for?
        /*case HDIO_GETGEO: {
            struct hd_geometry geo;
            DEBUG_MSG(KERN_INFO "<%s, %d> | GEO-IOCTL\n", __FUNCTION__, __LINE__);
            geo.cylinders   = (Device.size / KERNEL_SECTOR_SIZE & ~0x3F) >> 6;  // 4096
            geo.heads       = 4;
            geo.sectors     = 16;
            geo.start       = 0;
            if (0 != copy_to_user((void __user *)arg, &geo, sizeof(geo))) {
    	           DEBUG_MSG(KERN_WARNING "<%s, %d> | copy_to_user won't work\n", __FUNCTION__, __LINE__);
                return -EFAULT;
            }
            return 0; }*/
        case IOCTLCMD_RAM_PCI_VER: {
            int retval = ior(DEV_VER_DATA);
            DEBUG_MSG(KERN_INFO "<%s, %d> | DEV_VER_DATA: %lu\n", __FUNCTION__, __LINE__, DEV_VER_DATA);
            if (0 != copy_to_user((void __user *)arg, &retval, sizeof(int))) {
    	           DEBUG_MSG(KERN_WARNING "<%s, %d> | copy_to_user won't work\n", __FUNCTION__, __LINE__);
                return -ENOTTY;
            }
            return 0; }
        case IOCTLCMD_RAM_SIZE: {
            DEBUG_MSG(KERN_INFO "<%s, %d> | size: %llu\n", __FUNCTION__, __LINE__, Device.size);
            if (0 != copy_to_user((void __user *)arg, &Device.size, sizeof(unsigned long long))) {
    	           DEBUG_MSG(KERN_WARNING "<%s, %d> | copy_to_user won't work\n", __FUNCTION__, __LINE__);
                return -ENOTTY;
            }
            return 0; }
        default: {
            DEBUG_MSG(KERN_WARNING "<%s, %d> | Invalid cmd: %u\n", __FUNCTION__, __LINE__, cmd);
            return -EINVAL; }
    }
}

/**
 * Gibt die Geometrie der RAMDisk wieder.
 * @param block_device  PCI-Device
 * @param geo           Drive-Geometrie
 * @return              0 falls keine Fehler aufgetreten sind, andernfalls != 0.
 */
static int ramdisk_getgeo(struct block_device *block_device, struct hd_geometry *geo) {
    DEBUG_MSG(KERN_INFO "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    geo->cylinders   = (Device.size / KERNEL_SECTOR_SIZE & ~0x3F) >> 6; // 4096
    geo->heads       = 4;
    geo->sectors     = 16;
    geo->start       = 0;
    DEBUG_MSG(KERN_INFO "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
    return 0;
}

static struct block_device_operations bops = {
    .owner              = THIS_MODULE,          // Besitzer (Pflichtangabe) 
    .ioctl              = ramdisk_ioctl,        // Zur Bearbeitung der IOCTL-Kommandos
    .getgeo             = ramdisk_getgeo,       // Zum Bearbeiten von Geometrieanfragen
    /*.open             = ramdisk_open,
    .release            = ramdisk_release,
    .locked_ioctl       = ramdisk_locked_ioctl,
    .compat_ioctl       = ramdisk_compat_ioctl,
    .direct_access      = ramdisk_direct_access,
    .media_changed      = ramdisk_media_changed,
    .revalidate_disk    = ramdisk_revalidate_disk,*/
};

/**
 * Initialisiern die RAMDisk.
 * @return  0 falls keine Fehler aufgetreten sind, andernfalls != 0.
 */
static int ramdisk_init(void) {
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
#if !USE_VMALLOC
    if (pci_register_driver(&pci_drv) < 0) {
        printk(KERN_ERR "<%s, %d> | Error when register pci driver\n", __FUNCTION__, __LINE__);
        return -1;
    }
#endif
    
    if ((major = register_blkdev(0, DRIVER_NAME)) <= 0) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | can not get a majorno.\n", __FUNCTION__, __LINE__);
        goto fail_register_blkdev;
    }
  
#if USE_QUEUE
    if ((Device.queue = blk_alloc_queue(GFP_KERNEL)) == NULL) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | blk_alloc_queue failed\n", __FUNCTION__, __LINE__);
        goto fail_init_queue;
    }
    blk_queue_make_request(Device.queue, ramdisk_request);
#else
    spin_lock_init(&Device.lock);
    if ((Device.queue = blk_init_queue(ramdisk_request, &Device.lock)) == NULL) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | blk_init_queue failed\n", __FUNCTION__, __LINE__);
        goto fail_init_queue;
    }
#endif
    blk_queue_segment_boundary(Device.queue, 0x80000);
    blk_queue_logical_block_size(Device.queue, KERNEL_SECTOR_SIZE);
    //blk_queue_max_segment_size(Device.queue, KERNEL_SECTOR_SIZE);
    
    if (!(Device.disk = alloc_disk(MINOR_CNT))) {
        DEBUG_MSG(KERN_ERR "<%s, %d> | alloc_disk failed\n", __FUNCTION__, __LINE__);
        goto fail_alloc_disk;
    } 
    
    Device.size                 = LOGICAL_BLOCK_SIZE * NO_SECTORS;
#if USE_VMALLOC
    Device.data                 = vmalloc(Device.size);
#else
    Device.data                 = (void *)v_addr_bar + DEV_DDR_DATA;
#endif
    Device.disk->major          = major;
    Device.disk->first_minor    = MINOR_FIRST;
    Device.disk->private_data   = &Device;
    Device.disk->fops           = &bops;
    Device.disk->queue          = Device.queue;
    //Device.disk->flags          = GENHD_FL_SUPPRESS_PARTITION_INFO; // Partitionsinfos verbergen
    sprintf(Device.disk->disk_name, "%s%d", DRIVER_NAME, major);
    set_capacity(Device.disk, Device.size / KERNEL_SECTOR_SIZE);
    add_disk(Device.disk);
    
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
    return 0;
                   
    fail_alloc_disk:
        blk_cleanup_queue(Device.queue);
        unregister_blkdev(major, DRIVER_NAME);
        
    fail_init_queue:
    fail_register_blkdev:
        Device.size = 0U;
    
    return -ENOMEM;
}

/**
 * Deinitialisiert die RAMDisk.
 */
static void ramdisk_exit(void) {
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    unregister_blkdev(major, DRIVER_NAME);
    del_gendisk(Device.disk);
    put_disk(Device.disk);
    blk_cleanup_queue(Device.queue);
    Device.size = 0U;
#if USE_VMALLOC
    vfree(Device.data);
#else
    pci_unregister_driver(&pci_drv);
#endif
    DEBUG_MSG(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}

#endif
