#include "globalVars.h"

GlobalVars globalVars;

SDIOBlockDevice bd;
FATFileSystem fs("sda", &bd);

SPIFBlockDevice spif(PB_5, PB_4, PB_3, PB_0);
LittleFileSystem lfs("sdb", &spif);

Rotor rotor1(PC_0, PD_9, PD_10);


void formatSPIFlash()
{
    spif.init();
    spif.erase(0, spif.get_erase_size());
    spif.deinit();
    
    lfs.format(&spif);
}


void print_dir(FileSystem *fs, const char* dirname) {
    Dir dir;
    struct dirent ent;

    dir.open(fs, dirname);
    printf("contents of dir: %s\n", dirname);
    printf("----------------------------------------------------\n");

    while (1) {
        size_t res = dir.read(&ent);
        if (0 == res) {
            break;
        }
        printf(ent.d_name);
        printf("\n");
    }
    dir.close();
}

void print_SPIF_info() 
{
        spif.init();
        printf("spif size: %llu\n",         spif.size());
        printf("spif read size: %llu\n",    spif.get_read_size());
        printf("spif program size: %llu\n", spif.get_program_size());
        printf("spif erase size: %llu\n",   spif.get_erase_size());
        spif.deinit();
}

Mutex mutexPrintLog;
void print_log(const char *format, ...)
{
    mutexPrintLog.lock();

    va_list arg;
    va_start(arg, format);
    vprintf(format, arg);
    va_end(arg);
    fflush(stdout);
    
    mutexPrintLog.unlock();
}