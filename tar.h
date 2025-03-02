#ifndef __TAR_H__
#define __TAR_H__

#include <iostream>
#include <fstream>
#include <cstring>
#include <iomanip>
#include <vector>
#include <ctime>
#include <sys/stat.h>
using namespace std;

// file type values (1 octet)
#define REGULAR          0
#define NORMAL          '0'
#define HARDLINK        '1'
#define SYMLINK         '2'
#define CHAR            '3'
#define BLOCK           '4'
#define DIRECTORY       '5'
#define FIFO            '6'
#define CONTIGUOUS      '7'

#define ERROR(fmt, ...) (cerr << "Error: " << fmt << endl, ##__VA_ARGS__); return -1;

void Usage();
unsigned int Oct2uint(string oct);
int Iszero(string str);

typedef struct TarHeader
{
	char filename[100];
	char filemode[8];
	char userid[8];
	char groupid[8];
	char filesize[12];
	char mtime[12];
	char checksum[8];
	char type;
	char lname[100];
			
	/* USTAR Section */
	char USTAR_id[6];
	char USTAR_ver[2];
	char username[32];
	char groupname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
}Tar;

class TarFile{
public:
	TarFile(const char* file);
	int tarRead();
	int tarLs();
	int LsEntry(Tar aTar);
private:
	const char* filename;
	vector<Tar> tarVec;
};

#endif
