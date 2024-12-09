#include <stdio.h>
#include <dirent.h>
#include <math.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>

int find_in(char** list, const int size, const char* element){
	for(int i=0; i<size; ++i)
		if(strcmp(list[i], element) == 0)
			return i;
	return -1;
}

void trim(char* filename, const int* dirfd, const char* newname, int* enumerator){
	char* begin = strstr(filename, " [");
	char* end = strstr(filename, "].");
	if(!begin || !end) {
		fprintf(stderr, "File %s isn't a valid file for trimming\n", filename);
		return;
	}
	++end;
	char* new_str;
	if (!newname) {
		new_str = (char*) malloc ((strlen(filename) + 1) * sizeof(char));
		strcpy(new_str, filename);
		int pos = begin - filename;
		new_str[pos] = '\0';
		strcat(new_str, end);
	}
	else{
		if (enumerator) {
			char num_string[(int)(ceil(log10(*enumerator)+1)*sizeof(char))];
			sprintf(num_string, "%d", *enumerator);
			new_str = (char*) malloc ((strlen(newname) + strlen(num_string) + strlen(end) + 3 ) * sizeof(char));
			strcpy(new_str, num_string);
			strcat(new_str, ". ");
			strcat(new_str, newname);
			strcat(new_str, end);
			++(*enumerator);
		}
		else {
			new_str = (char*) malloc ((strlen(newname) + strlen(end) + 1) * sizeof(char));
			strcpy(new_str, newname);
			strcat(new_str, end);
		}
	}
	printf("Renaming %s to %s\n", filename, new_str);
	dirfd ? renameat(*dirfd, filename, *dirfd, new_str) : rename(filename, new_str);
	free(new_str);
	
}

void bulk_trim(const char* path, const char* newname){
	
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

		trim(dir_entry->d_name, &dir_fd, newname, &counter);

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
		if(name) {
			if(name_retval > find_retval)
				upper_limit = name_retval;
			else
				lower_limit = name_retval + 3;
		}
		int diff = upper_limit - lower_limit;
		int* sender = diff > 1 ? &diff : NULL;
		diff = 1;
		for(int i=lower_limit; i<upper_limit; ++i)
			trim(argv[i], NULL, name, sender);
	}
	else{
		fprintf(stderr, "Invalid configuration: use -d to bulk rename"
						"directory contents, and -f to rename files."
						"The parameter -n can be specified to rename to custom names\n");
		return -1;
	}

	return 0;

}
