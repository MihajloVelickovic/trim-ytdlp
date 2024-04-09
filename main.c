#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

void trim(char* filename, int* dirfd){
		
	char* begin = strstr(filename, " [");
	char* end = strstr(filename, "].");	
	if(!begin || !end){
		fprintf(stderr, "File %s isn't a valid file for trimming\n", filename);
		return;
	}
	
	char newstr[strlen(filename) + 1];
	strcpy(newstr, filename);
	int pos = begin - filename;
	newstr[pos] = '\0';

	++end;
	strcat(newstr, end);
	printf("Renaming %s to %s\n", filename, newstr);
	dirfd ? renameat(*dirfd, filename, *dirfd, newstr) : rename(filename, newstr);
	
}

void bulk_trim(const char* path){
	
	DIR* dir;
	if(!(dir = opendir(path))){
		fprintf(stderr, "Error opening folder.\n");
		exit(-1);
	}

	int dir_fd = dirfd(dir);

	struct dirent* dir_entry;
	struct stat stats;
	
	while((dir_entry = readdir(dir))){

		char file_path[strlen(path) + strlen(dir_entry->d_name) + 2];
		strcpy(file_path, path);
		strcat(file_path, "/");
		strcat(file_path, dir_entry->d_name);

		if(stat(file_path, &stats) < 0){
			perror("Error getting file info.\n");
			exit(-1);
		}

		if(!S_ISREG(stats.st_mode))
			continue;
	
		trim(dir_entry->d_name, &dir_fd);
		
	}

	closedir(dir);

}

int main(int argc, char** argv){
	
	if(argc < 3){
		fprintf(stderr, "At least 2 arguments needed: mode and file/folder.\n");
		return -1;
	}

	if(strcmp(argv[1], "-d") == 0)
		for(int i=2; i<argc; ++i)
			bulk_trim(argv[i]);
	
	else if(strcmp(argv[1], "-f") == 0)
		for(int i=2; i<argc; ++i)
			trim(argv[i], NULL);
	
	else{
		fprintf(stderr, "Invalid configuration: use -d to bulk rename"
						"directory contents, and -f to rename files.\n");
		return -1;
	}

	return 0;

}
