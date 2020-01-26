#ifndef __globalVars_h__
#define __globalVars_h__

#include "mbed.h"
#include <stdio.h>
#include "SDIOBlockDevice.h"
#include "FATFileSystem.h"
#include "Rotor.h"

#ifdef COMPONENT_SPIF
#include "SPIFBlockDevice.h"
#include "LittleFileSystem.h"

extern SPIFBlockDevice spif;
extern LittleFileSystem lfs;

void formatSPIFlash(FileSystem *fs); 
void print_SPIF_info();
#endif

void print_log(const char *format, ... );

typedef struct 
{
    float adcValues[4];
    float rotorPosActual;
    float rotorPosSetpoint;
} GlobalVars;

extern SDIOBlockDevice bd;
extern FATFileSystem fs;

extern DigitalOut led1;

extern Rotor rotor1;

extern GlobalVars globalVars;

void print_dir(FileSystem *fs, const char* dirname);

#endif