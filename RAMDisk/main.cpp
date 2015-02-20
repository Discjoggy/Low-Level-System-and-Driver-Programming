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
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include "ioctl_cmds.h"

#include <iostream>
#include <stdio.h>
#include <fstream>  
#include <sstream>
#include <pwd.h>
#include <cstdlib>
#include <string>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <stdint.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/sendfile.h>

#include <time.h>

#define MB_TO_BYTE(value) (value << 20)
#define PATH_TO_DEVICE "/media/student/RAMDISK"
#define DEVICE_NAME "/dev/CFPGA251"
static unsigned long long max_size = MB_TO_BYTE(128);

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
    printf("Test (#01): %d, \tparam: %X\n", result, param);
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
 * ...
 * @param dev_fd  dev_fd Filedescriptor von dem Device
 * @return        0 falls der Test erfolgreich war, andernfalls != 0.
 */
/*int test_writing(const int &dev_fd) {
    unsigned long long size_of_files = 0;
    unsigned int number_of_files = 0;

    std::string path = getenv("HOME");

    while (number_of_files == 0) {
        std::cout << "Geben sie die Anzahl der Dateien an die erzeugt werden sollen!(1=Minimum)" << std::endl;
        std::cin >> number_of_files;
    }

    std::cout << "Geben sie die Größe in MB an!(1=Minimum)" << std::endl;
    std::cin >> size_of_files;
 
    path = PATH_TO_DEVICE;    
    struct stat sb;
    
    if (argc > 1) {
        path= argv[1];
        if (stat(path.c_str(), &sb) == 0 && S_ISDIR(sb.st_mode)) {
            //std::cout << path << " ist ein Verzeichnis!" << std::endl;
        }
        else {
            mkdir(path.c_str(), S_IRWXU);
            std::cout << "Verzeichnis angelegt!" << std::endl;
        }
    }
    std::cout << "Number of files: " << number_of_files << std::endl;
    std::cout << "Filesize: " << size_of_files << std::endl;
    std::cout << "Path: " << path << std::endl;

    //std::cout << sizeof(unsigned long long) << std::endl;
    for (unsigned i = 0 ; i < number_of_files; ++i) {
        char a[KBYTE]; // segfault bei 1024 * 1024
        std::stringstream sStream;
        sStream << path << "/FakeFile" << i;
        string tmpPath = sStream.str();

        FILE* pFile;
        pFile = fopen(tmpPath.c_str(), "wb+");    
        for (int j = 0; j < 1024; ++j) {       
            fwrite(a, 1, (size_of_files * sizeof(char)), pFile);
        }
        fclose(pFile);
    }
    
    return 0;
}
*/

/**
 * Tests mithilfe einer Dummy-Datei auf das Device zu schreiben / lesen.
 * Es wird die sendfile()-Methode verwendet.
 * @param dev_fd  Filedescriptor von dem Device
 * @return        0 falls der Test erfolgreich war, andernfalls != 0.
 */
int test_fast_writing(const int &dev_fd) {
    const char* testname = "Fast Writing / Reading";
    printf("\n%s: Tests in progess\n", testname);
    
    clock_t start, end;
    struct stat64 stat_buf;
    ssize_t total_bytes;
    off64_t offset;
    int file_fd;
    
    // TEST: WRTITING
    if ((file_fd = open("FILEPATH+FILENAME", O_RDONLY)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    fstat(file_fd, &stat_buf);
    
    // Write to Device
    start = clock();
    if ((total_bytes = sendfile64(dev_fd, file_fd, offset, stat_buf.st_size)) < 0) {
        printf("Test (#01) failed\n");
        return 1;
    }
    end = clock();
    printf("Total time taken by CPU: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    
    if ((close(dev_fd)) < 0) {
        printf("Error when closing file descriptor\n");
        return 1;
    }
    
    // TEST: READING
    if ((file_fd = open("FILEPATH+FILENAME", O_WRONLY)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    fstat(dev_fd, &stat_buf);
    
    // Read from Device
    start = clock();
    if ((total_bytes = sendfile64(file_fd, dev_fd, offset, stat_buf.st_size)) < 0) {
        printf("Test (#01) failed\n");
        return 1;
    }
    end = clock();
    printf("Total time taken by CPU: %f\n", (double)(end - start) / CLOCKS_PER_SEC);
    
    if ((close(dev_fd)) < 0) {
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
    if ((dev_fd = open(DEVICE_NAME, O_RDWR | O_CREAT)) < 0) {
        fprintf(stderr, "Error when opening file descriptor: %s\n", strerror(errno));
        return 1;
    }
    
    test_ver(dev_fd);
    test_space(dev_fd);
    //test_writing(dev_fd);
    test_fast_writing(dev_fd);
    
    if ((close(dev_fd)) < 0) {
        printf("Error when closing file descriptor\n");
        return 1;
    }
    return 0;
}
