/* 
 * File:   ramdisk_device.h
 * Author: Tobias Sibera, Jens Overmoeller
 *
 * Created on 8. Januar 2015, 20:43
 */
#ifndef RAMDISK_DEVICE_H
#define	RAMDISK_DEVICE_H

#define KERNEL_SECTOR_SIZE  512UL
#define LOGICAL_BLOCK_SIZE  512UL
#define NO_SECTORS          256UL
// TOTAL SIZE = LOGICAL_BLOCK_SIZE * NO_SECTORS = 1024 * 2048 bytes = 2048 KiB

static struct bd_device {
    unsigned int size;
    spinlock_t lock;
    u8 *data;
    struct request_queue *queue;
    struct gendisk *disk;
} Device;

static void ramdisk_dev_write(sector_t sector_off, u8 *buffer, unsigned int sectors) {
    memcpy(Device.data + sector_off * KERNEL_SECTOR_SIZE, buffer, sectors * KERNEL_SECTOR_SIZE);
}

static void ramdisk_dev_read(sector_t sector_off, u8 *buffer, unsigned int sectors) {
    memcpy(buffer, Device.data + sector_off * KERNEL_SECTOR_SIZE, sectors * KERNEL_SECTOR_SIZE);
}

#endif
