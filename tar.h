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

// file type
#define REGULAR '\0'
#define NORMAL '0'
#define HARDLINK '1'
#define SYMLINK '2'
#define CHAR '3'
#define BLOCK '4'
#define DIRECTORY '5'
#define FIFO '6'
#define CONTIGUOUS '7'

#define ERROR(fmt, ...)                      \
	{                                        \
		fprintf(stderr, fmt, ##__VA_ARGS__); \
		exit(1);                             \
	}

#define WARN(fmt, ...)                       \
	{                                        \
		fprintf(stderr, fmt, ##__VA_ARGS__); \
	}

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

	/* UStar Section */
	char UStar_id[6];
	char UStar_ver[2];
	char username[32];
	char groupname[32];
	char devmajor[8];
	char devminor[8];
	char prefix[155];
	char pad[12];
} Tar;

string modeToStr(mode_t mode, char ftype);
void dec2oct(char *oct, int size, int dec);
int oct2dec(char *oct, int size);
int computeChecksum(Tar header);
bool isEmptyBlock(char *block, int size);

class TarFile
{
public:
	TarFile(string tarfile);
	~TarFile();
	void create(vector<string> files, bool verbose);
	void list(bool verbose);
	void extract(bool verbose);

private:
	string tarfile;
	vector<Tar> entries;

	void addFile(string filename, ofstream &out, bool verbose);
	void extractFile(string filename, int size, ifstream &in);
};

#endif
