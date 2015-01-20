/* 
 * File:   ramdisk.h
 * Author: Tobias Sibera, Jens Overmoeller
 *
 * Created on 07. Februrary 2015, 02:47
 */
#ifndef RAMDISK_H
#define	RAMDISK_H

#include <linux/blkdev.h>
#include "ioctl_cmds.h"
#include "ramdisk_device.h"

#define MINOR_FIRST 0U
#define MINOR_CNT   1U
#define DRIVER_NAME "CFPGA"

#define USE_VMALLOC 1
#define USE_QUEUE 1

static unsigned int major = 0U;
module_param(major, int, 0);

#if USE_QUEUE
static void ramdisk_request(struct request_queue *q, struct bio *bio) {
    char *kaddr, *maddr;
    struct bio_vec *bvec;
    int segnr;
    printk(KERN_DEBUG "<%s, %d> | bd_make_request(%p)\n", __FUNCTION__, __LINE__, bio);
    
    blk_queue_bounce(q, &bio); // higmemory-support
    bio_for_each_segment(bvec, bio, segnr) {
        kaddr = bio_data(bio);
        maddr = Device.data + (KERNEL_SECTOR_SIZE * bio->bi_sector);
        if (bio_data_dir(bio) == READ || bio_data_dir(bio) == READA) {
            printk(KERN_DEBUG "<%s, %d> | R: %p - %p len = %d\n", __FUNCTION__, __LINE__, kaddr, maddr, bio->bi_size);
            memcpy(kaddr, maddr, bio->bi_size);
        } 
        else {
            printk(KERN_DEBUG "<%s, %d> | R: %p - %p len = %d\n", __FUNCTION__, __LINE__, maddr, kaddr, bio->bi_size);
            memcpy(maddr, kaddr, bio->bi_size);
        }
    };
    bio_endio(bio, 0);
}
#else
static void ramdisk_request(struct request_queue *q) {
    u8 *kaddr, *maddr;
    struct req_iterator iter;
    struct bio_vec *bvec = NULL;
    struct request *req;
    printk(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    
    while ((req = blk_fetch_request(q)) != NULL) {
        if (req->cmd_type != REQ_TYPE_FS) {
            if (!__blk_end_request_cur(req, 0)) {
                req = blk_fetch_request(q);
            }
            continue;
        }
        
        rq_for_each_segment(bvec, req, iter) {
            kaddr = page_address(bvec->bv_page) + bvec->bv_offset;
            maddr = Device.data + (KERNEL_SECTOR_SIZE * iter.bio->bi_sector);
            if (bio_data_dir(iter.bio) == READ) {
                printk(KERN_DEBUG "<%s, %d> | R: %p - %p len = %d\n", __FUNCTION__, __LINE__, kaddr, maddr, bvec->bv_len);
                memcpy(kaddr, maddr, bvec->bv_len);
            }
            else {
                printk(KERN_DEBUG "<%s, %d> | W: %p - %p len = %d\n", __FUNCTION__, __LINE__, maddr, kaddr, bvec->bv_len);
                memcpy(maddr, kaddr, bvec->bv_len);
            }
        };
        
        if (!__blk_end_request_cur(req, 0)) {
            req = blk_fetch_request(q);
        }
    }
    printk(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}
#endif

static int ramdisk_ioctl(struct block_device *instance, fmode_t mode, unsigned cmd, unsigned long arg) {
    switch (cmd) {
        case IOCTLCMD_RAM_SIZE:
            if (copy_to_user((int *)arg, &Device.size, sizeof(int)) != 0) {
                return -ENOTTY;
            }
            return 0;
        default:
            printk(KERN_WARNING "<%s, %d> | Invalid cmd: %u\n", __FUNCTION__, __LINE__, cmd);
            return -EINVAL;
    }
}

static struct block_device_operations bops = {
    .owner              = THIS_MODULE,
    .ioctl              = ramdisk_ioctl,
    /*.open             = ramdisk_open,
    .release            = ramdisk_release,
    .getgeo             = ramdisk_getgeo,
    .locked_ioctl       = ramdisk_locked_ioctl,
    .compat_ioctl       = ramdisk_compat_ioctl,
    .direct_access      = ramdisk_direct_access,
    .media_changed      = ramdisk_media_changed,
    .revalidate_disk    = ramdisk_revalidate_disk,*/
};

static int ramdisk_init(void) {
    printk(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    
    Device.size = LOGICAL_BLOCK_SIZE * NO_SECTORS;
#if USE_VMALLOC
    if (!(Device.data = vmalloc(Device.size))) {
        printk(KERN_DEBUG "<%s, %d> | vmalloc failed\n", __FUNCTION__, __LINE__);
        return -ENOMEM;
    }
#else 
    if (!(Device.data = kmalloc(Device.size, GFP_KERNEL))) {
        printk(KERN_DEBUG "<%s, %d> | kmalloc failed\n", __FUNCTION__, __LINE__);
        return -ENOMEM;
    }
#endif
    
    if ((major = register_blkdev(0, DRIVER_NAME)) <= 0) {
        printk(KERN_DEBUG "<%s, %d> | can not get a majorno.\n", __FUNCTION__, __LINE__);
        goto fail_register_blkdev;
    }
  
#if USE_QUEUE
    if ((Device.queue = blk_alloc_queue(GFP_KERNEL)) == NULL) {
        printk(KERN_DEBUG "<%s, %d> | blk_alloc_queue failed\n", __FUNCTION__, __LINE__);
        goto fail_init_queue;
    }
    blk_queue_make_request(Device.queue, ramdisk_request);
#else
    spin_lock_init(&Device.lock);
    if ((Device.queue = blk_init_queue(ramdisk_request, &Device.lock)) == NULL) {
        printk(KERN_DEBUG "<%s, %d> | blk_init_queue failed\n", __FUNCTION__, __LINE__);
        goto fail_init_queue;
    }
    blk_queue_logical_block_size(Device.queue, KERNEL_SECTOR_SIZE);    
#endif
    
    if (!(Device.disk = alloc_disk(MINOR_CNT))) {
        printk(KERN_DEBUG "<%s, %d> | alloc_disk failed\n", __FUNCTION__, __LINE__);
        goto fail_alloc_disk;
    } 
    
    Device.disk->major         = major;
    Device.disk->first_minor   = MINOR_FIRST;
    Device.disk->fops          = &bops;
    Device.disk->queue         = Device.queue;
    sprintf(Device.disk->disk_name, "%s%d", DRIVER_NAME, major);
    set_capacity(Device.disk, Device.size);
    add_disk(Device.disk);
    
    printk(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
    return 0;
                   
    fail_alloc_disk:
        blk_cleanup_queue(Device.queue);
        unregister_blkdev(major, DRIVER_NAME);
        
    fail_init_queue:
    fail_register_blkdev:
        Device.size = 0U;
#if USE_VMALLOC
        vfree(Device.data);
#else 
        kfree(Device.data);
#endif
        return -ENOMEM;
}

static void ramdisk_exit(void) {
    printk(KERN_DEBUG "<%s, %d> | Start\n", __FUNCTION__, __LINE__);
    unregister_blkdev(major, DRIVER_NAME);
    del_gendisk(Device.disk);
    put_disk(Device.disk);
    blk_cleanup_queue(Device.queue);
    Device.size = 0U;
#if USE_VMALLOC
    vfree(Device.data);
#else
    kfree(Device.data);
#endif
    printk(KERN_DEBUG "<%s, %d> | Done\n", __FUNCTION__, __LINE__);
}

#endif
