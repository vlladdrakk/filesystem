#include <stdio.h>
#include <stdlib.h>
#include "fs.h"
#include "common.h"
#include "disk_utils.h"

extern superblock* super;

int dump_to_disk(void* partition, char* text_file) {
	// check that the partition exists
	if (partition == NULL || super == NULL) {
		#ifdef DEBUG
		printf("dump_to_disk: No existing partition\n");
		#endif
		return FAILURE;
	}

	// Open the file
	FILE* fd = fopen(text_file, "wb");

	// Write partition data to file
	int num_bytes = super->num_blocks * BLK_SIZE;
	fwrite(partition, sizeof(byte), num_bytes, fd);

	fclose(fd);
	return SUCCESS;
}

void* load_from_disk(char* text_file) {
	// Open image file
	FILE* fd = fopen(text_file, "rb");

	// Get the size of the file
	fseek(fd, 0L, SEEK_END);
	long file_size = ftell(fd);
	rewind(fd);

	// Create the partition
	void* partition = malloc(sizeof(byte) * file_size);

	fread(partition, 1, file_size, fd);

	fclose(fd);

	return partition;
}