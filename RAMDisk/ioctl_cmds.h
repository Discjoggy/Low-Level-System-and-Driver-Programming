/* 
 * File:        :   ioctl_cmds.h
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   08.01.2015, 23:11
 * Description  :   IOCTL-Header, welcher für die Verwendung von IOCTLs nötig ist.
 */
#ifndef IOCTL_CMDS_H
#define	IOCTL_CMDS_H

#define IOCTLCMD_MAGIC 'r'
#define IOCTLCMD_MAXNR 1

// Commands
#define IOCTLCMD_RAM_PCI_VER    _IOR(IOCTLCMD_MAGIC, 0, int)
#define IOCTLCMD_RAM_SIZE       _IOR(IOCTLCMD_MAGIC, 1, int)

#endif
