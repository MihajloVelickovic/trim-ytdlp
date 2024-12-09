#include <stdio.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

int find_in(char** list, int size, char* element){
	for(int i=0; i<size; ++i)
		if(strcmp(list[i], element) == 0)
			return i;
	return -1;
}

void trim(char* filename, int* dirfd, char* newname){
	char* begin = strstr(filename, " [");
	char* end = strstr(filename, "].");
	if(!begin || !end) {
		fprintf(stderr, "File %s isn't a valid file for trimming\n", filename);
		return;
	}
	++end;
	char* newstr;
	if (!newname) {
		newstr = (char*) malloc ((strlen(filename) + 1) * sizeof(char));
		strcpy(newstr, filename);
		int pos = begin - filename;
		newstr[pos] = '\0';
		strcat(newstr, end);
	}
	else{
		newstr = (char*) malloc ((strlen(newname) + 1) * sizeof(char));
		strcpy(newstr, newname);
		strcat(newstr, end);
	}
	printf("Renaming %s to %s\n", filename, newstr);
	dirfd ? renameat(*dirfd, filename, *dirfd, newstr) : rename(filename, newstr);
	free(newstr);
	
}

void bulk_trim(const char* path, char* newname){
	
	DIR* dir;
	if(!(dir = opendir(path))){
		fprintf(stderr, "Error opening folder.\n");
		exit(-1);
	}

	int dir_fd = dirfd(dir);

	struct dirent* dir_entry;
	struct stat stats;
	int counter = 1;
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

		char num_string[(int)(ceil(log10(counter)+1)*sizeof(char))];
		sprintf(num_string, "%d", counter);
		char* newstr;
		char need_free = 0;
		if (!newname)
			newstr = newname;
		else {
			newstr = (char*) malloc(strlen(newname) + strlen(num_string) + 3);
			need_free = 1;
			strcpy(newstr, num_string);
			strcat(newstr, ". ");
			strcat(newstr, newname);
		}
		trim(dir_entry->d_name, &dir_fd, newstr);
		if (need_free > 0)
			free(newstr);
		++counter;

	}

	closedir(dir);

}


int main(int argc, char** argv){
	
	if(argc < 3){
		fprintf(stderr, "At least 2 arguments needed: mode and file/folder.\n");
		return -1;
	}
	
	int find_retval,
		name_retval = find_in(argv, argc, "-n");
	char* name = name_retval > 0 ? argv[name_retval+1] : NULL;

	if((find_retval = find_in(argv, argc, "-d")) > 0) {
		int lower_limit = find_retval + 1,
			upper_limit = argc;
		if(name){
			if(name_retval > find_retval)
				upper_limit = name_retval;
			else
				lower_limit = name_retval + 3;
		}
		for(int i=lower_limit; i<upper_limit; ++i)
			bulk_trim(argv[i], name);
	}
	
	else if((find_retval = find_in(argv, argc, "-f")) > 0){
		int lower_limit = find_retval + 1,
			upper_limit = argc;
		if(name){
			if(name_retval > find_retval)
				upper_limit = name_retval;
			else
				lower_limit = name_retval + 3;
		}
		for(int i=lower_limit; i<upper_limit; ++i)
			trim(argv[i], NULL, name);
	}
	else{
		fprintf(stderr, "Invalid configuration: use -d to bulk rename"
						"directory contents, and -f to rename files."
						"The parameter -n can be specified to rename to custom names\n");
		return -1;
	}

	return 0;

}
