#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <dirent.h>

typedef struct {
	char *name;
	unsigned int size;
} addonfile;

char *ztstr(FILE *file)
{
	int size;
	char chr, *dest;
	for (size = 1;; size++)
	{
		fread(&chr, 1, 1, file);
		if (chr == '\0')
			break;
	}

	fseek(file, -size, SEEK_CUR);
	dest = malloc(size);
	fread(dest, size, 1, file);
	return dest;
}

FILE *fopenwb(char *path)
{
	int len = strlen(path) + 1, i;
	char *dirpath;

	if (len == 1)
	{
		fputs("File in addon has no name\n", stderr);
		exit(1);
	}

	dirpath = malloc(len);
	for (i = 0; i < len; i++)
	{
		if (path[i] == '/')
		{
			dirpath[i] = '\0';
			mkdir(dirpath, 0755);
		}
		dirpath[i] = path[i];
	}

	free(dirpath);
	return fopen(path, "wb");
}

int isDirectory(const char *path) {
  struct stat statbuf;
  if (stat(path, &statbuf) != 0)
    return 0;

  return S_ISDIR(statbuf.st_mode);
}

int extractFile(const char *path, const char *outputDirectory) {
	unsigned int filecnt = 0, filenum, i, j;
	char *addonname, *addondesc, *addonauthor;
	int curbyte;
	addonfile *addonfiles = NULL, *curfile;

	FILE *in = fopen(path, "rb");
	if (in == NULL)
	{
		fprintf(stderr, "Error opening input file %s: %s\n", path, strerror(errno));
		return errno;
	}

	fread(&i, 4, 1, in);
	if (i != 0x44414d47)
	{
		fputs("Input file is not an addon\n\n", stderr);
		return 1;
	}

	fseek(in, 18, SEEK_CUR);
	addonname = ztstr(in);
	addondesc = ztstr(in);
	addonauthor = ztstr(in);
	fseek(in, 4, SEEK_CUR);

	puts("Attempting to extract file names...");
	for (;;)
	{
		fread(&filenum, 4, 1, in);
		if (filenum == 0)
			break;

		filecnt++;
		addonfiles = realloc(addonfiles, filecnt * sizeof(addonfile));
		curfile = &addonfiles[filecnt - 1];
		curfile->name = ztstr(in);
		fread(&curfile->size, 4, 1, in);
		fseek(in, 8, SEEK_CUR);
	}

	if (addonfiles == NULL)
	{
		puts("Addon is empty");
		return 0;
	}

  chdir("..");
  chdir(outputDirectory);
	mkdir(addonname, 0755);
	chdir(addonname);

  FILE* output;

	for (i = 0; i < filecnt; i++)
	{
		curfile = &addonfiles[i];
		printf("Extracting %s (%uB)\n", curfile->name, curfile->size);
		output = fopenwb(curfile->name);
		if (output == NULL)
		{
			fprintf(stderr, "Error opening output file %s: %s\n", curfile->name, strerror(errno));
			return errno;
		}

		for (j = 0; j < curfile->size; j++)
		{
			fread(&curbyte, 1, 1, in);
			fputc(curbyte, output);
		}

		fflush(output);
		fclose(output);
	}

	puts("Writing info.txt");

  output = fopen("info.txt", "wb");
	if (output == NULL)
	{
		fprintf(stderr, "Error opening info.txt: %s\n", strerror(errno));
		return errno;
	}

	fprintf(output,
		"\"AddonInfo\"\r\n{\r\n\t\"name\" \"%s\"\r\n\t\"author_name\" \"%s\"\r\n\t\"info\" \"%s\"\r\n}", addonname, addonauthor, addondesc);
	printf("Finished extracting addon %s\n", addonname);
  if (outputDirectory == NULL)
    chdir("..");
  else
    chdir("../../");
}

int main(const int argc, const char *argv[])
{
	printf("GMAD Multi-Extractor v3 by PixeL\n\n");
	if (argc < 2)
	{
		fputs("No file/directory specified\n", stderr);
		return 1;
	}

  // TODO: Redo directory navigation and such.
  if (argv[2] == NULL) {
    fprintf(stderr, "Please specify an output directory.\n");
    return 1;
  }

  // Make it so they can specify an output directory as well.
  const char *outputDirectory = argv[2];
  if (!strcmp(outputDirectory, ".") || !strcmp(outputDirectory, "./"))
    outputDirectory = NULL;

  if (outputDirectory != NULL && access(outputDirectory, F_OK) != 0){
    if (ENOENT == errno) {
      printf("The directory %s does not exist, creating now.\n", outputDirectory);
      mkdir(outputDirectory, 755);
    } else if (ENOTDIR == errno) {
      fprintf(stderr, "%s is not a directory!\n", outputDirectory);
    }
  }

  // TODO: Implement recursive directory search.
  if (isDirectory(argv[1])) {
    chdir(argv[1]);

    DIR *d;
    struct dirent *dir;
    d = opendir(".");

    if (d) {
      while ((dir = readdir(d)) != NULL) {
        if (!strcmp(dir->d_name, ".") || !strcmp(dir->d_name, "..")) continue;
        if (strlen(dir->d_name) > 4 && strcmp(dir->d_name + strlen(dir->d_name) - 4, ".gma")) continue;

        printf("Extracting %s...\n", dir->d_name);
        extractFile(dir->d_name, outputDirectory);

        chdir(argv[1]);
      }

      closedir(d);
    }

    chdir("..");
    return 0;
  }

  extractFile(argv[1], outputDirectory);

	return 0;
}
