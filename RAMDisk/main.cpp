/* 
 * File:        :   main.cpp
 * Author       :   Tobias Sibera <Tobias.Sibera@HS-Osnabrueck.de>
 *                  Jens Overmoeller <Jens.Overmoeller@HS-Osnabrueck.de>
 * Copyright    :   GPLv3
 * Created on   :   18.12.2014, 02:59
 * Description  :   Enthält Tests zur Überprüfung des korrekt arbeitenden Block-Treibers.
 */
#include <sys/ioctl.h>
#include <sys/mman.h>
#include <sys/sendfile.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <iostream>
#include <stdio.h>
#include <fstream>  
#include <sstream>
#include <pwd.h>
#include <cstdlib>
#include <string>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <time.h>

#include "ioctl_cmds.h"

#define FILE_NAME "FakeFile"
//#define PATH_TO_DEVICE "/dev/CFPGA251"
#define DEVICE "/dev/CFPGA251"
#define PATH_TO_DEVICE "/media/tosibera/RAMDISK1"
#define MEGABYTE 1048576 // (1024 * 1024)
#define MB_TO_BYTE(value) (value << 20)
#define BYTE_TO_MB(value) (value >> 20)

static unsigned long long max_size      = MB_TO_BYTE(128);
static unsigned long long file_size     = MB_TO_BYTE(32);
static unsigned int file_count          = 1;

/**
 * Tests die Versionsnr. des Devices auszulesen.
 * @param dev_fd   Filedescriptor von dem Device
 * @return         0 falls der Test erfolgreich war, andernfalls != 0.
 */
int test_ver(const int &dev_fd) {
    const char* testname = "VER";
    printf("\n%s: Tests in progess\n", testname);
    
    int param = 0;
    int result = ioctl(dev_fd, IOCTLCMD_RAM_PCI_VER, &param); 
    printf("Test (#01): %d, \tparam: %X\n", result, param);
    if (result != 0) {
        printf("Test (#01) failed\n");
        return 1;
    }
    
    printf("%s: Tests done\n", testname);
    return 0;
}

/**
 * Tests den verfügbaren Speicher des Devices auszulesen.
 * @param dev_fd   Filedescriptor von dem Device
 * @return         0 falls der Test erfolgreich war, andernfalls != 0.
 */
int test_space(const int &dev_fd, bool replace_max_val = 1) {
    const char* testname = "SPACE";
    printf("\n%s: Tests in progess\n", testname);
    
    unsigned long long param = 0;
    int result = ioctl(dev_fd, IOCTLCMD_RAM_SIZE, &param); 
    printf("Test (#01): %d, \tparam: %llu\n", result, param);
    if (result != 0) {
        printf("Test (#01) failed\n");
        return 1;
    }
    if (replace_max_val) {
        max_size = param;
    }
    
    printf("%s: Tests done\n", testname);
    return 0;
}

/**
 * Testet mithilfe von fwrite das erstellen und beschreiben von Dateien.
 * @param dev_fd  dev_fd Filedescriptor von dem Device
 * @return        0 falls der Test erfolgreich war, andernfalls != 0.
 */
int test_writing(const int &dev_fd) {
    const char* testname = "WRITING";
    printf("\n%s: Tests in progess\n", testname);
    
    while (file_count <= 0) {
        printf("Geben sie die Anzahl der Dateien an, die erzeugt werden sollen (1 min)\n");
        std::cin >> file_count;
    }
    while (file_size <= 0 || file_size > max_size) {
        printf("Geben sie die Groesse der Dateien in MB an (16 min, 128 max)\n");
        int tmp;
        std::cin >> tmp;
        file_size = MB_TO_BYTE(tmp);
    }
    
    std::string file_path = getenv("HOME");
    file_path = "/home/tosibera";
    //file_path = PATH_TO_DEVICE;
    /*if (argc > 1) {
        struct stat stat_buf;
        if (!(stat(argv[1].c_str(), &sb) == 0 && S_ISDIR(stat_buf.st_mode))) {
            mkdir(argv[1].c_str(), S_IRWXU);
            std::cout << "Verzeichnis angelegt!" << std::endl;
        }
        file_path = argv[1];
    }*/
    printf("Number of files: %u\n", file_count);
    printf("Filesize: %llu MB (%llu Bytes)\n", BYTE_TO_MB(file_size), file_size);
    printf("Path(/Filename): %s(/%s)\n", file_path.c_str(), FILE_NAME);
    
    FILE* pFile;
    char a[MEGABYTE];
    for (unsigned i = 0 ; i < file_count; i++) {
        int total_bytes = 0;
        std::stringstream tmpFile;
        tmpFile << file_path << "/" << FILE_NAME << '_' << i;
        if ((pFile = fopen(tmpFile.str().c_str(), "w")) == NULL) {
            fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
            return 1;
        }
        for (unsigned j = 0; j < BYTE_TO_MB(file_size); ++j) {   
            if ((total_bytes = fwrite(a, sizeof(char), MEGABYTE, pFile)) != MEGABYTE) {
                printf("%u Bytes should be written, but the result was %u\n", MEGABYTE, total_bytes);
                return 1;
            }
        }
        if (fclose(pFile) != 0) {
            fprintf(stderr, "Error when closing file descriptor: %s\n", strerror(errno));
                return 1;
        }
    }
    
    printf("%s: Tests done\n", testname);
    return 0;
}

/**
 * Tests mithilfe einer Dummy-Datei auf das Device zu schreiben / lesen.
 * Es wird die sendfile()-Methode verwendet.
 * @param dev_fd  Filedescriptor von dem Device
 * @return        0 falls der Test erfolgreich war, andernfalls != 0.
 */
int test_fast_writing(const int &dev_fdXXXX) {
    const char* testname = "Fast Writing / Reading";
    printf("\n%s: Tests in progess\n", testname);    
    clock_t start, end;
    struct stat stat_buf;
    ssize_t total_bytes;
    off_t offset;
    
    int file_fd;
    int dev_fd;
    std::string file_path = "/home/tosibera/FastWriteFile";
    std::string dest_file = "/media/tosibera/RAMDISK1/FastWriteFile";
    printf("Type in the path + filename\n");
    //std::cin >> file_path;
    std::string file_path_BAK = file_path + "_BAK";
    
    // TEST: WRTITING
    if ((dev_fd = open(dest_file.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    if ((file_fd = open(file_path.c_str(), O_RDONLY)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    fstat(file_fd, &stat_buf);
    start = clock();
    if ((total_bytes = sendfile(dev_fd, file_fd, &offset, stat_buf.st_size)) != stat_buf.st_size) {
        printf("%lu Bytes should be written, but the result was %lu\n", stat_buf.st_size, total_bytes);
        fprintf(stderr, "Error %s\n", strerror(errno));
        return 1;
    }
    end = clock();
    printf("Total time taken by CPU: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    if ((close(file_fd)) < 0 || (close(dev_fd)) < 0) {
        printf("Error when closing file descriptor\n");
        return 1;
    }
    
    // TEST: READING
    if ((dev_fd = open(dest_file.c_str(), O_RDONLY)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    if ((file_fd = open(file_path_BAK.c_str(), O_WRONLY | O_CREAT | O_TRUNC)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    fstat(dev_fd, &stat_buf);
    start = clock();
    if ((total_bytes = sendfile(file_fd, dev_fd, &offset, stat_buf.st_size)) != stat_buf.st_size) {
        printf("%lu Bytes should be readed, but the result was %lu\n", stat_buf.st_size, total_bytes);
        return 1;
    }
    end = clock();
    printf("Total time taken by CPU: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    if ((close(file_fd)) < 0 || (close(dev_fd)) < 0) {
        printf("Error when closing file descriptor\n");
        return 1;
    }
    
    printf("%s: Tests done\n", testname);
    return 0;
}

/**
 * Einstiegsmethode.
 * @param argc  Anzahl der übergebenen Argumente
 * @param argv  Werte der Argumente
 * @return      0 falls kein Fehler aufgetreten ist, andernfalls != 0.
 */
int main(int argc, char** argv) {
    int dev_fd = 0;
    if ((dev_fd = open(DEVICE, O_RDWR | O_CREAT | O_APPEND)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    
    //test_ver(dev_fd);
    test_space(dev_fd);
    test_writing(dev_fd);
    test_fast_writing(dev_fd);
    
    if ((close(dev_fd)) < 0) {
        printf("Error when closing file descriptor\n");
        return 1;
    }
    
    return 0;
}
