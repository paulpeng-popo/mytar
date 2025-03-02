#include "tar.h"

void Usage()
{
	cout << "Usage: ./mytar " << "[tar_file]" << endl;
}

unsigned int Oct2uint(string oct)
{
	unsigned int out = 0;
	int size = oct.size();
    int i = 0;
    while ((i < size) && oct[i])
        out = (out << 3) | (unsigned int) (oct[i++] - '0');

    return out;
}

int Iszero(string str)
{
	int size = str.size();
	for(int i = 0; i < size; i++)
	{
        if(str[i]-'0')
            return 0;
    }
    
    return 1;
}

TarFile::TarFile(const char* file)
{
	filename = file;
}

int TarFile::tarRead()
{
    int count = 0;
    char check[32];
    char buf[512];
    unsigned int offset = 0;
    ifstream file(filename);
    if(!file)
    {
    	ERROR("tar_file is not found");
		return -1;
    }
	
    for(count = 0; ; count++)
    {
    	file.seekg(offset + 256,ios::beg);
		if(file.read(check, 6))
		{
			if(!strcmp(check, "ustar "))
			{
				ERROR("File is not in Ustar format.");
				return -1;
			}
		}
		
        file.read(buf, 512);
		if(Iszero(buf))
		{
			file.read(buf, 512);
			if(Iszero(buf))
			{
				file.seekg(0, ios::end);
				break;
			}
		}
		
		// cout << buf << endl;
		// cin.get();

        file.seekg(offset, ios::beg);
        
		Tar entry;
		file.read((char*)&entry, 512);
		tarVec.push_back(entry);
		
        unsigned int jump = Oct2uint(entry.filesize);
        if(jump % 512)
            jump += 512 - (jump % 512);

        offset += 512 + jump;
        file.seekg(jump, ios::cur);
    }
    
    file.close();
    return count;
}

int TarFile::tarLs()
{
	int size = tarVec.size();
	for(int i = 0; i < size; i++)
	{
		if(LsEntry(tarVec[i]) < 0)
			return -1;
	}
	
	return 0;
}

int TarFile::LsEntry(Tar aTar)
{
	const mode_t mode = Oct2uint(aTar.filemode);
    const char mode_str[26] = { "-hlcbdp-" [aTar.type ? aTar.type-'0' : 0],
                                mode & S_IRUSR ? 'r' : '-',
                                mode & S_IWUSR ? 'w' : '-',
                                mode & S_IXUSR ? 'x' : '-',
                                mode & S_IRGRP ? 'r' : '-',
                                mode & S_IWGRP ? 'w' : '-',
                                mode & S_IXGRP ? 'x' : '-',
                                mode & S_IROTH ? 'r' : '-',
                                mode & S_IWOTH ? 'w' : '-',
                                mode & S_IXOTH ? 'x' : '-', 0};
	cout << mode_str << ' ' << aTar.username << '/' << aTar.groupname << ' ';
	
	switch(aTar.type)
	{
		case REGULAR: case NORMAL: case CONTIGUOUS:
			cout << setw(9) << setfill(' ') << Oct2uint(aTar.filesize) << ' ';
			break;
		case HARDLINK: case SYMLINK: case DIRECTORY: case FIFO:
			cout << setw(9) << setfill(' ') << Oct2uint(aTar.filesize) << ' ';
			break;
		case CHAR: case BLOCK:
			cout << setw(9) << setfill(' ') << Oct2uint(aTar.devmajor) << ',' << Oct2uint(aTar.devminor) << ' ';
			break;
	}

	time_t mtime = Oct2uint(aTar.mtime);
	struct tm* time = localtime(&mtime);
	cout << time -> tm_year + 1900 << '-' << setw(2) << setfill('0') 
	     << time -> tm_mon + 1 << '-' << setw(2) << setfill('0') 
	     << time -> tm_mday << ' ' << setw(2) << setfill('0') 
	     << time -> tm_hour << ':' << setw(2) << setfill('0') 
	     << time -> tm_min << ' ';

	cout << aTar.filename;

	switch(aTar.type)
	{
		case HARDLINK:
			cout << " link to " << aTar.lname;
			break;
		case SYMLINK:
			cout << " -> " << aTar.lname;
			break;
	}
	
	cout << endl;
    return 0;
}




