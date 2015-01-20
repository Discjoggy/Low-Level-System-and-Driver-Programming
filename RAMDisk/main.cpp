/* 
 * File:   main.cpp
 * Author: Tobias Sibera, Jens Overmoeller
 *
 * Created on 08. Februrary 2015, 02:59
 */
#include <dirent.h>
#include <sys/errno.h>
#include <sys/mount.h>

#define DEVICE "/cdev/crdisk"
#define FILESYS "/mnt"

int main() {
    int fd = 0;
    if ((fd = mount(DEVICE, FILESYS, MS_MGC_VAL | MS_RDONLY | MS_NOSUID, "")) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return fd;
    }
    
    mkdir(FILESYS + "/testX");
    
    
    /*if ((fd = umount2(FILESYS, MNT_FORCE)) < 0) {
        printf("Error when closing file descriptor\n");
        return fd;
    }*/
    return 0;
}
