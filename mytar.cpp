#include "tar.h"
#include <unistd.h>

const int FLAG_T = 0x01; // 0001
const int FLAG_X = 0x02; // 0010
const int FLAG_C = 0x04; // 0100
const int FLAG_V = 0x08; // 1000

void print_usage(string execname)
{
	cout << "Usage: " << execname << " -[ctxv] <tarfile> [folder | file1 file2 ...]" << endl;
}

// Add .tar extension if not present
string ensure_tarfile(string tarfile)
{
	if (tarfile.size() < 4 || tarfile.substr(tarfile.size() - 4) != ".tar")
	{
		tarfile += ".tar";
	}
	return tarfile;
}

int main(int argc, char *argv[])
{
	string execname = argv[0];

	if (argc < 3)
	{
		print_usage(execname);
		return 1;
	}

	int flags = 0;
	string tarfile = "";
	vector<string> files;

	for (int i = 1; i < argc; i++)
	{
		string arg = argv[i];
		if (arg[0] == '-')
		{
			for (int j = 1; j < arg.size(); j++)
			{
				switch (arg[j])
				{
				case 'c':
					flags |= FLAG_C;
					break;
				case 't':
					flags |= FLAG_T;
					break;
				case 'x':
					flags |= FLAG_X;
					break;
				case 'v':
					flags |= FLAG_V;
					break;
				default:
					print_usage(execname);
					return 1;
				}
			}
		}
		else if (i <= 2 && tarfile.empty())
		{
			tarfile = ensure_tarfile(arg);
		}
		else
		{
			files.push_back(arg);
		}
	}

	if (flags & FLAG_C)
	{
		if (files.empty())
		{
			ERROR("tar: no files or directories specified\n");
		}
		else
		{
			TarFile tar(tarfile);
			tar.create(files, flags & FLAG_V);
		}
	}
	else if (flags & FLAG_T)
	{
		TarFile tar(tarfile);
		tar.list(flags & FLAG_V);
	}
	else if (flags & FLAG_X)
	{
		if (files.empty())
		{
			TarFile tar(tarfile);
			tar.extract(flags & FLAG_V);
		}
		else
		{
			string folder = files[0];
			string oldpath = getcwd(NULL, 0);

			// Change to target directory
			chdir(folder.c_str());

			TarFile tar(tarfile);
			tar.extract(flags & FLAG_V);

			// Change back to original directory
			chdir(oldpath.c_str());
		}
	}
	else
	{
		print_usage(execname);
		return 1;
	}

	return 0;
}
