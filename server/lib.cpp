/*
 * To change this license header, choose License Headers in Project Properties.
 * To change this template file, choose Tools | Templates
 * and open the template in the editor.
 */

/* 
 * File:   lib.cpp
 * Author: mahdi
 * 
 * Created on December 12, 2017, 11:41 AM
 */
#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <stdint.h>
#include <iostream>
#include <fstream>
#include <sys/stat.h>
#include <sstream>
#include <iomanip>

#include "lib.hpp"

using namespace std;

/*
 * Author:  David Robert Nadeau
 * Site:    http://NadeauSoftware.com/
 * License: Creative Commons Attribution 3.0 Unported License
 *          http://creativecommons.org/licenses/by/3.0/deed.en_US
 */

#if defined(_WIN32)
#include <Windows.h>

#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
#include <unistd.h>
#include <sys/types.h>
#include <sys/param.h>
#include <string>
#include <fstream>

#if defined(BSD)
#include <sys/sysctl.h>
#endif

#else
#error "Unable to define getMemorySize( ) for an unknown OS."
#endif



/**
 * Returns the size of physical memory (RAM) in bytes.
 */
size_t getMemorySize() {
    std::string token;
    std::ifstream file("/proc/meminfo");
    while(file >> token) {
        if(token == "MemAvailable:") {
            unsigned long mem;
            if(file >> mem) {
                mem *= 1024;
                //TODO to be removed
                //cout << "from getMemorySize, getMemorySize=" << mem << endl;
                return mem;
            }
            else
                return 0;
        }
    }
    return 0; // nothing found
}
//size_t getMemorySize()
//{
//#if defined(_WIN32) && (defined(__CYGWIN__) || defined(__CYGWIN32__))
//	/* Cygwin under Windows. ------------------------------------ */
//	/* New 64-bit MEMORYSTATUSEX isn't available.  Use old 32.bit */
//	MEMORYSTATUS status;
//	status.dwLength = sizeof(status);
//	GlobalMemoryStatus( &status );
//	return (size_t)status.dwTotalPhys;
//
//#elif defined(_WIN32)
//	/* Windows. ------------------------------------------------- */
//	/* Use new 64-bit MEMORYSTATUSEX, not old 32-bit MEMORYSTATUS */
//	MEMORYSTATUSEX status;
//	status.dwLength = sizeof(status);
//	GlobalMemoryStatusEx( &status );
//	return (size_t)status.ullTotalPhys;
//
//#elif defined(__unix__) || defined(__unix) || defined(unix) || (defined(__APPLE__) && defined(__MACH__))
//	/* UNIX variants. ------------------------------------------- */
//	/* Prefer sysctl() over sysconf() except sysctl() HW_REALMEM and HW_PHYSMEM */
//
//#if defined(CTL_HW) && (defined(HW_MEMSIZE) || defined(HW_PHYSMEM64))
//	int mib[2];
//	mib[0] = CTL_HW;
//#if defined(HW_MEMSIZE)
//	mib[1] = HW_MEMSIZE;		/* OSX. --------------------- */
//#elif defined(HW_PHYSMEM64)
//	mib[1] = HW_PHYSMEM64;		/* NetBSD, OpenBSD. --------- */
//#endif
//	int64_t size = 0;		/* 64-bit */
//	size_t len = sizeof( size );
//	if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
//		return (size_t)size;
//	return 0L;			/* Failed? */
//
//#elif defined(_SC_AIX_REALMEM)
//	/* AIX. ----------------------------------------------------- */
//	return (size_t)sysconf( _SC_AIX_REALMEM ) * (size_t)1024L;
//
//#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGESIZE)
//	/* FreeBSD, Linux, OpenBSD, and Solaris. -------------------- */
//	return (size_t)sysconf( _SC_PHYS_PAGES ) *
//		(size_t)sysconf( _SC_PAGESIZE );
//
//#elif defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
//	/* Legacy. -------------------------------------------------- */
//	return (size_t)sysconf( _SC_PHYS_PAGES ) *
//		(size_t)sysconf( _SC_PAGE_SIZE );
//
//#elif defined(CTL_HW) && (defined(HW_PHYSMEM) || defined(HW_REALMEM))
//	/* DragonFly BSD, FreeBSD, NetBSD, OpenBSD, and OSX. -------- */
//	int mib[2];
//	mib[0] = CTL_HW;
//#if defined(HW_REALMEM)
//	mib[1] = HW_REALMEM;		/* FreeBSD. ----------------- */
//#elif defined(HW_PYSMEM)
//	mib[1] = HW_PHYSMEM;		/* Others. ------------------ */
//#endif
//	unsigned int size = 0;		/* 32-bit */
//	size_t len = sizeof( size );
//	if ( sysctl( mib, 2, &size, &len, NULL, 0 ) == 0 )
//		return (size_t)size;
//	return 0L;			/* Failed? */
//#endif /* sysctl and sysconf variants */
//
//#else
//	return 0L;			/* Unknown OS. */
//#endif
//}

int error(const char *t_msg) {
    perror(t_msg);
    abort();
}

int min(const size_t t_a, const size_t t_b) {
    return (t_a>t_b) ? t_b : t_a;
}

int max(const size_t t_a, const size_t t_b) {
    return (t_a<t_b) ? t_b : t_a;
}

int readFileToBuffer(const string fileName, char *buffer, size_t size) {
    size_t fileSize = getFileSize(fileName);
    ifstream fileStream(fileName, ios::binary);
    fileStream.read(buffer, size);
    if (size > fileSize)
        for (size_t l=fileSize; l<size; l++)
            buffer[l]='\0';
    fileStream.close();
    return 0;
}

size_t getFileSize(const string fileName) {
    size_t size = 0;
    ifstream fileStream(fileName, ios::binary);
    string errorMsg = "Unable to open the file: " + fileName;
    if (!fileStream.is_open()) error(errorMsg.c_str());
    fileStream.seekg (0, fileStream.end);
    size = fileStream.tellg();
    fileStream.close();
    return size;
}

bool fileExists(const string& t_fileName) {
    struct stat buf;
    if (stat(t_fileName.c_str(), &buf) != -1)
        return true;
    return false;
}

int coutBuff(const char* t_sToDisplay, const int t_P, const size_t t_sizeInBytes) {
    cout << "Buff= ";
    switch (t_P) {
        case 1: {
            uint8_t* sInt = (uint8_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes; i++)
                cout << hex << (int)sInt[i] << ", ";
            break;}
        case 8: {
            uint8_t* sInt = (uint8_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 16: {
            uint16_t* sInt = (uint16_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/2; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 32: {
            uint32_t* sInt = (uint32_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/4; i++)
                cout << (int)sInt[i] << ", ";
            break;}
        case 64: {
            uint64_t* sInt = (uint64_t*) t_sToDisplay;
            for (size_t i=0; i<t_sizeInBytes/8; i++)
                cout << (int)sInt[i] << ", ";
            break;}
    }
    cout << endl;
}

#define HEX(x) setw(2) << setfill('0') << hex << (int)(x)

int char2int(char input)
{
    if(input >= '0' && input <= '9')
        return input - '0';
    if(input >= 'A' && input <= 'F')
        return input - 'A' + 10;
    if(input >= 'a' && input <= 'f')
        return input - 'a' + 10;
    throw std::invalid_argument("Invalid input string");
}

// This function assumes src to be a zero terminated sanitized string with
// an even number of [0-9a-f] characters, and target to be sufficiently large

int hex2bin(string src, char* target, size_t binSize)
{
    for (size_t i=0; i<binSize; i++)
        target[i] = char2int(src.at(i*2))*16 + char2int(src.at(i*2+1));
}
    
string bin2hex(const char* src, size_t binSize) {
    stringstream seedSS;
    for (int i=0; i<binSize; i++)
        seedSS << HEX((uint8_t)src[i]);
    return seedSS.str();
}
