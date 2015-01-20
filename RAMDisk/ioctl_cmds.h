/* 
 * File:   ioctl_cmds.h
 * Author: Tobias Sibera, Jens Overmoeller
 *
 * Created on 8. Januar 2015, 23:11
 */

#ifndef IOCTL_CMDS_H
#define	IOCTL_CMDS_H

#define IOCTLCMD_MAGIC 'r'

// Commands
#define IOCTLCMD_RAM_SIZE _IOR(IOCTLCMD_MAGIC, 0, int)

#endif
