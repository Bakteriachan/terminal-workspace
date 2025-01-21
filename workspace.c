#include <stdio.h>
#include <json-c/json.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include "workspace.h"

void get_directory(char* location) {
	char *loc = "/home/bakteria/.config/terminal-workspaces/workspaces.json";
	location = strcpy(location, loc);
}


json_object *get_json_object(char* file_dir) {
	json_object *root = json_object_from_file(file_dir);

	if(root == NULL) {
		return json_object_new_object();
	}

	return root;
}

json_object* get_workspace_json_from_root_json(json_object* root, char* workspace) {
	json_object* result;
	if(!json_object_object_get_ex(root, workspace, &result)) {
		result = json_object_new_object();
		json_object_object_add(root, workspace, result);
		return result;
	}
	return result;
}


void add_variable(json_object* workspace, char* key, char* value) {
	json_object* obj = json_object_new_string(value);
	if(json_object_object_add(workspace, key, obj) < 0) {
		fprintf(stderr, "error while adding new string to workspace\n");
	}
}

void get_variable(json_object* workspace, char* key, char** value) {
	json_object* obj;
	if(json_object_object_get_ex(workspace, key, &obj)) {
		const char *buff = json_object_get_string(obj);
		strcpy(*value, buff);
	} else {
		*value = NULL;
	}
}

void save_json_object(json_object* root, char* directory) {
	int fd = open(directory, O_TRUNC | O_WRONLY);
	if(fd < 0) {
		int err_v = errno;
		fprintf(stderr, "Error saving json object: %s\n", strerror(err_v));
	} else {
		const char* buff = json_object_to_json_string_ext(root, JSON_C_TO_STRING_PRETTY);
		if(write(fd, buff, strlen(buff)) < 0) {
			int err_v = errno;
			fprintf(stderr, "Error while saving json object: %s\n", strerror(err_v));
		}
	}
}

// Active workspace is selected in this order:
// 		1 -> Environment variable
// 		2 -> Current active workspace (obtained from file `.active`)
// 		3 -> Default workspace
int get_active_workspace_name(char* workspace) {
	char* default_w = "DEFAULT";

	int isActive = 0;
	char *buff = malloc(WORKSPACE_MAX_SIZE);

	int fd = open(".active", O_RDONLY);
	if(fd != -1) {
		int r = read(fd, buff, WORKSPACE_MAX_SIZE);
		//realloc(buff, r);
		if(r == -1) {
			int err_v = errno;
			dprintf(2, "Error reading: %s\n", strerror(err_v));
			return -1;
		}
		isActive = 1;
		close(fd);
	}

	char* env_w = getenv("WORKSPACE_NAME");
	if(env_w == NULL) {
		if(isActive) {
			sprintf(workspace, "%s", buff);
		} else {
			strcpy(workspace, default_w);
		}
	} else {
		sprintf(workspace, "%s", env_w);
	}
	return 0;
}
