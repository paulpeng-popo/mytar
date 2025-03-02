#include "tar.h"

int main(int argc, char *argv[])
{
	if(argc < 2)
	{
		Usage();
		return -1;
	}
    TarFile handle_tar(argv[1]);
	if(handle_tar.tarRead() < 0)
		return -1;
    handle_tar.tarLs();
    return 0;
}
