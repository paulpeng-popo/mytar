#include "tar.h"
#include <sys/types.h>
#include <pwd.h>
#include <grp.h>
#include <cstdio>
#include <cstdlib>
#include <sstream>
#include <iomanip>
#include <ctime>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

// filemode to string
string modeToStr(mode_t mode, char ftype)
{
	const char mode_str[12] = {
		"-hlcbdp-"[ftype ? ftype - '0' : 0],
		mode & S_IRUSR ? 'r' : '-',
		mode & S_IWUSR ? 'w' : '-',
		mode & S_IXUSR ? 'x' : '-',
		mode & S_IRGRP ? 'r' : '-',
		mode & S_IWGRP ? 'w' : '-',
		mode & S_IXGRP ? 'x' : '-',
		mode & S_IROTH ? 'r' : '-',
		mode & S_IWOTH ? 'w' : '-',
		mode & S_IXOTH ? 'x' : '-'};

	return string(mode_str);
}

void dec2oct(char *oct, int size, int dec)
{
	snprintf(oct, size, "%0*o", size - 1, dec);
}

int oct2dec(char *oct, int size)
{
	char buf[32];
	memset(buf, 0, sizeof(buf));
	strncpy(buf, oct, size);
	return strtol(buf, NULL, 8);
}

// 計算 header 的 checksum
// 先將 checksum 欄位填滿空白後再累加所有位元組
int computeChecksum(Tar header)
{
	memset(header.checksum, ' ', sizeof(header.checksum));
	unsigned char *raw = (unsigned char *)&header;
	int sum = 0;
	for (size_t i = 0; i < sizeof(Tar); i++)
	{
		sum += raw[i];
	}
	return sum;
}

bool isEmptyBlock(char *block, int size)
{
	for (int i = 0; i < size; i++)
	{
		if (block[i] != 0)
			return false;
	}
	return true;
}

TarFile::TarFile(string tarfile)
{
	this->tarfile = tarfile;
}

TarFile::~TarFile()
{
}

void TarFile::create(vector<string> files, bool verbose)
{
	ofstream out(tarfile, ios::binary);
	if (!out)
	{
		ERROR("Cannot open tar file for writing: %s\n", tarfile.c_str());
	}
	// 依序將 header 與檔案內容寫入
	for (auto &filename : files)
	{
		addFile(filename, out, verbose);
	}
	// 最後寫入兩個 512 位元的空區塊表示結束
	char block[512] = {0};
	out.write(block, 512);
	out.write(block, 512);
	out.close();
}

void TarFile::addFile(string filename, ofstream &out, bool verbose)
{
	// 取得檔案資訊
	struct stat sb;
	if (stat(filename.c_str(), &sb) < 0)
	{
		ERROR("Cannot stat file: %s\n", filename.c_str());
	}

	Tar header;
	memset(&header, 0, sizeof(Tar));

	bool isDir = S_ISDIR(sb.st_mode);
	string tarname = filename;
	if (isDir)
	{
		if (tarname.back() != '/')
			tarname += "/";
		strncpy(header.filename, tarname.c_str(), sizeof(header.filename));
		header.type = DIRECTORY;
		dec2oct(
			header.filesize,
			sizeof(header.filesize),
			0);
	}
	else
	{
		strncpy(header.filename, filename.c_str(), sizeof(header.filename));
		header.type = REGULAR;
		dec2oct(
			header.filesize,
			sizeof(header.filesize),
			sb.st_size);
	}

	dec2oct(
		header.filemode,
		sizeof(header.filemode),
		sb.st_mode & 0777);
	dec2oct(
		header.userid,
		sizeof(header.userid),
		sb.st_uid);
	dec2oct(
		header.groupid,
		sizeof(header.groupid),
		sb.st_gid);
	dec2oct(
		header.mtime,
		sizeof(header.mtime),
		sb.st_mtime);

	// unix standard
	strncpy(header.UStar_id, "ustar", sizeof(header.UStar_id));
	strncpy(header.UStar_ver, "00", sizeof(header.UStar_ver));
	struct passwd *pw = getpwuid(sb.st_uid);
	if (pw)
	{
		strncpy(header.username, pw->pw_name, sizeof(header.username));
	}
	else
	{
		strncpy(header.username, "unknown", sizeof(header.username));
	}
	struct group *gr = getgrgid(sb.st_gid);
	if (gr)
	{
		strncpy(header.groupname, gr->gr_name, sizeof(header.groupname));
	}
	else
	{
		strncpy(header.groupname, "unknown", sizeof(header.groupname));
	}

	// 計算 checksum
	memset(header.checksum, ' ', sizeof(header.checksum));
	int sum = computeChecksum(header);
	snprintf(
		header.checksum,
		sizeof(header.checksum),
		"%06o", sum);
	header.checksum[6] = '\0'; // 結尾字元
	header.checksum[7] = ' ';  // 根據標準最後一個字元為空白

	// 寫入 header 區塊（512 bytes）
	out.write((char *)&header, sizeof(Tar));

	// 不是目錄，寫入檔案內容
	if (!isDir)
	{
		ifstream in(filename, ios::binary);
		if (!in)
		{
			ERROR("Cannot open file for reading: %s\n", filename.c_str());
		}

		// 寫入檔案內容
		const int bufSize = 4096;
		char buffer[bufSize];
		size_t bytesRemaining = sb.st_size;
		while (bytesRemaining > 0)
		{
			size_t toRead = (bytesRemaining < bufSize) ? bytesRemaining : bufSize;
			in.read(buffer, toRead);
			out.write(buffer, toRead);
			bytesRemaining -= toRead;
		}
		// 若檔案內容不足 512 的倍數，補零填滿區塊
		int pad = 512 - (sb.st_size % 512);
		if (pad < 512)
		{
			char padding[512] = {0};
			out.write(padding, pad);
		}

		in.close();
	}

	// verbose mode
	if (verbose)
	{
		string perm = modeToStr(sb.st_mode, header.type);
		char timebuf[64];
		struct tm *tm_info = localtime(&sb.st_mtime);
		strftime(
			timebuf,
			sizeof(timebuf),
			"%b %e %H:%M",
			tm_info);
		printf(
			"%s 0 %s %s %10ld %s %s\n",
			perm.c_str(),
			header.username,
			header.groupname,
			(isDir) ? 0L : (long)sb.st_size,
			timebuf,
			header.filename);
	}
	else
	{
		printf("a %s\n", header.filename);
	}

	if (isDir)
	{
		DIR *dir = opendir(filename.c_str());
		if (dir)
		{
			struct dirent *entry;
			while ((entry = readdir(dir)) != NULL)
			{
				if (strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0)
					continue;
				string subfile = filename + "/" + entry->d_name;
				addFile(subfile, out, verbose);
			}
			closedir(dir);
		}
	}
}

void TarFile::list(bool verbose)
{
	ifstream in(tarfile, ios::binary);
	if (!in)
	{
		ERROR("Cannot open tar file for reading: %s\n", tarfile.c_str());
	}
	while (true)
	{
		Tar header;
		in.read((char *)&header, sizeof(Tar));

		// read 0 bytes means end of file
		if (in.gcount() == 0)
			break;

		// header size is not correct
		if (in.gcount() < sizeof(Tar))
		{
			ERROR("Corrupted tar header in %s\n", tarfile.c_str());
		}

		// 若連續兩個 512 位元區塊都是空區塊，表示結束
		if (isEmptyBlock((char *)&header, sizeof(Tar)))
		{
			char block[512];
			in.read(block, 512);
			if (in.gcount() == 0 || isEmptyBlock(block, 512))
				break;
			else
				in.seekg(-512, ios::cur);
		}

		int size = oct2dec(header.filesize, sizeof(header.filesize));
		int mtime = oct2dec(header.mtime, sizeof(header.mtime));
		int modeInt = strtol(header.filemode, NULL, 8);
		string perm = modeToStr(modeInt, header.type);
		char timebuf[64];
		time_t mod_time = mtime;
		struct tm *tm_info = localtime(&mod_time);
		strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);
		if (verbose)
		{
			printf(
				"%s 0 %s %s %10d %s %s\n",
				perm.c_str(),
				header.username,
				header.groupname,
				size,
				timebuf,
				header.filename);
		}
		else
		{
			printf("%s\n", header.filename);
		}
		// 略過檔案內容區塊（依 512 區塊對齊）
		int blocks = (size + 511) / 512;
		in.seekg(blocks * 512, ios::cur);
	}
	in.close();
}

void TarFile::extract(bool verbose)
{
	ifstream in(tarfile, ios::binary);
	if (!in)
	{
		ERROR("Cannot open tar file for reading: %s\n", tarfile.c_str());
	}
	while (true)
	{
		Tar header;
		in.read((char *)&header, sizeof(Tar));

		// read 0 bytes means end of file
		if (in.gcount() == 0)
		{
			cout << "end of file" << endl;
			break;
		}

		// header size is not correct
		if (in.gcount() < sizeof(Tar))
		{
			ERROR("Corrupted tar header in %s\n", tarfile.c_str());
		}

		// 若連續兩個 512 位元區塊都是空區塊，表示結束
		if (isEmptyBlock((char *)&header, sizeof(Tar)))
		{
			char block[512];
			in.read(block, 512);
			if (in.gcount() == 0 || isEmptyBlock(block, 512))
				break;
			else
				in.seekg(-512, ios::cur);
		}

		int size = oct2dec(header.filesize, sizeof(header.filesize));
		int modeInt = strtol(header.filemode, NULL, 8);
		// 若是目錄則建立目錄
		if (S_ISDIR(modeInt) || header.type == DIRECTORY)
		{
			cout << "create dir" << endl;
		}
		else
		{
			// 取得檔案名稱
			string filepath = header.filename;
			// 建立目錄
			size_t pos = filepath.find_last_of('/');
			if (pos != string::npos)
			{
				string dir = filepath.substr(0, pos);
				if (access(dir.c_str(), F_OK) != 0)
				{
					mkdir(dir.c_str(), 0755);
				}
			}
			// 寫入檔案內容
			extractFile(filepath, size, in);
		}
		if (verbose)
		{
			string perm = modeToStr(modeInt, header.type);
			int mtime = oct2dec(header.mtime, sizeof(header.mtime));
			char timebuf[64];
			time_t mod_time = mtime;
			struct tm *tm_info = localtime(&mod_time);
			strftime(timebuf, sizeof(timebuf), "%b %e %H:%M", tm_info);
			printf(
				"%s 0 %s %s %10d %s %s\n",
				perm.c_str(),
				header.username,
				header.groupname,
				size,
				timebuf,
				header.filename);
		}
		else
		{
			printf("x %s\n", header.filename);
		}
		// 略過因補齊 512 區塊而多出的 padding
		int pad = 512 - (size % 512);
		if (pad < 512)
		{
			in.seekg(pad, ios::cur);
		}
	}
	in.close();
}

void TarFile::extractFile(string filename, int size, ifstream &in)
{
	// 寫入檔案內容
	ofstream out(filename, ios::binary);
	if (!out)
	{
		ERROR("Cannot open file for writing: %s\n", filename.c_str());
	}
	const int bufSize = 4096;
	char buffer[bufSize];
	int bytesRemaining = size;
	while (bytesRemaining > 0)
	{
		int toRead = (bytesRemaining < bufSize) ? bytesRemaining : bufSize;
		in.read(buffer, toRead);
		out.write(buffer, toRead);
		bytesRemaining -= toRead;
	}
	out.close();
}
