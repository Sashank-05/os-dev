#pragma once

#include <stddef.h>
#include <stdint.h>

#define VFS_PATH_MAX 256
#define VFS_MAX_ENTRIES 128

enum vfs_node_type {
    VFS_NODE_FILE = 1,
    VFS_NODE_DIR = 2,
};

struct vfs_node {
    char path[VFS_PATH_MAX];
    uint8_t* data;
    size_t size;
    enum vfs_node_type type;
};

void vfs_init(void);
size_t vfs_count(void);
const struct vfs_node* vfs_find(const char* path);
const struct vfs_node* vfs_find_by_index(size_t index);
size_t vfs_read(const char* path, void* buffer, size_t size, size_t offset);
