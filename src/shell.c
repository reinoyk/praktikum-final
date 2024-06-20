#include "shell.h"
#include "kernel.h"
#include "std_lib.h"
#include "filesystem.h"

void shell() {
  char buf[64];
  char cmd[64];
  char arg[2][64];

  byte cwd = FS_NODE_P_ROOT;

  while (true) {
    printString("MengOS:");
    printCWD(cwd);
    printString("$ ");
    readString(buf);
    parseCommand(buf, cmd, arg);

    if (strcmp(cmd, "cd")) cd(&cwd, arg[0]);
    else if (strcmp(cmd, "ls")) ls(cwd, arg[0]);
    else if (strcmp(cmd, "mv")) mv(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cp")) cp(cwd, arg[0], arg[1]);
    else if (strcmp(cmd, "cat")) cat(cwd, arg[0]);
    else if (strcmp(cmd, "mkdir")) mkdir(cwd, arg[0]);
    else if (strcmp(cmd, "clear")) clearScreen();
    else printString("Invalid command\n");
  }
}

// TODO: 4. Implement printCWD function
void printCWD(byte cwd) {}

// TODO: 5. Implement parseCommand function
void parseCommand(char* buf, char* cmd, char arg[2][64]) {}

// TODO: 6. Implement cd function
void cd(byte* cwd, char* dirname) {}

// TODO: 7. Implement ls function
void ls(byte cwd, char* dirname) {}

// TODO: 8. Implement mv function
void mv(byte cwd, char* src, char* dst) {}


//TASK 9-11 : REINO YURIS KUSUMANEGARA - 5025231075
// TODO: 9. Implement cp function
void cp(byte cwd, char* src, char* dst) {
	struct file_metadata src_meta, dst_meta;
	enum fs_return status;
    
	strcpy(src_meta.node_name, src);
	src_meta.parent_index = cwd;
	fsRead(&src_meta, &status);

	if (status != FS_SUCCESS) {
    	printString("Source file not found\n");
    	return;
	}

	strcpy(dst_meta.node_name, dst);
	dst_meta.parent_index = cwd;
	memcpy(dst_meta.buffer, src_meta.buffer, src_meta.filesize);
	dst_meta.filesize = src_meta.filesize;
	fsWrite(&dst_meta, &status);

	if (status != FS_SUCCESS) {
    	printString("Could not create destination file\n");
	}
}


// TODO: 10. Implement cat function
void cat(byte cwd, char* filename) {
    struct file_metadata metadata;
    enum fs_return status;
    metadata.parent_index = cwd;
    strcpy(metadata.node_name, filename);
    metadata.filesize = 0;

    fsRead(&metadata, &status);

    if (status == FS_SUCCESS) {
	int i;
        for (i = 0; i < metadata.filesize; i++) {
            interrupt(0x10, 0xE << 8 | metadata.buffer[i], 0, 0, 0);
        }
        printString("\n");
    } else if (status == FS_R_NODE_NOT_FOUND) {
        printString("cat: File not found\n");
    } else if (status == FS_R_TYPE_IS_DIRECTORY) {
        printString("cat: Is a directory\n");
    } else {
        printString("cat: Unknown error\n");
    }
}



void mkdir(byte cwd, char* dirname) {
    struct node_fs nodes;
    struct map_fs map;
    int free_node_index = -1;
    int i;

    // Read the node and map sectors into memory
    readSector((byte*)&nodes, FS_NODE_SECTOR_NUMBER);
    readSector((byte*)&map, FS_MAP_SECTOR_NUMBER);

    // Check if the directory already exists
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (nodes.nodes[i].parent_index == cwd && strcmp(nodes.nodes[i].node_name, dirname) == 0) {
            printString("Directory already exists\n");
            return;
        }
    }

    // Find a free node
    for (i = 0; i < FS_MAX_NODE; i++) {
        if (nodes.nodes[i].node_name[0] == '\0') {
            free_node_index = i;
            break;
        }
    }

    if (free_node_index == -1) {
        printString("No free node available\n");
        return;
    }

    // Initialize the new directory node
    nodes.nodes[free_node_index].parent_index = cwd;
    nodes.nodes[free_node_index].data_index = FS_NODE_D_DIR;  
    strcpy(nodes.nodes[free_node_index].node_name, dirname);

    writeSector((byte*)&nodes, FS_NODE_SECTOR_NUMBER);

    printString("Directory created successfully\n");
}


