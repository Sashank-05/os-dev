#include <fs/vfs.h>

#include <common/stdinfo.h>
#include <debug/debug.h>
#include <libk/string.h>
#include <libk/stdio.h>

struct tar_header {
    char name[100];
    char mode[8];
    char uid[8];
    char gid[8];
    char size[12];
    char mtime[12];
    char checksum[8];
    char typeflag;
    char linkname[100];
    char magic[6];
    char version[2];
    char uname[32];
    char gname[32];
    char devmajor[8];
    char devminor[8];
    char prefix[155];
    char pad[12];
} __attribute__((packed));

static struct vfs_node nodes[VFS_MAX_ENTRIES];
static size_t node_count = 0;

static size_t tar_parse_octal(const char* field, size_t field_size) {
    size_t value = 0;
    for (size_t i = 0; i < field_size; i++) {
        char c = field[i];
        if (c == '\0' || c == ' ') {
            continue;
        }
        if (c < '0' || c > '7') {
            break;
        }
        value = (value << 3) | (size_t)(c - '0');
    }
    return value;
}

static size_t align_up_512(size_t value) {
    return (value + 511) & ~((size_t)511);
}

static void vfs_add_node(const char* path, uint8_t* data, size_t size, enum vfs_node_type type) {
    if (node_count >= VFS_MAX_ENTRIES) {
        return;
    }

    struct vfs_node* node = &nodes[node_count++];
    memset(node, 0, sizeof(*node));

    size_t path_len = strlen(path);
    if (path_len >= VFS_PATH_MAX) {
        path_len = VFS_PATH_MAX - 1;
    }

    memcpy(node->path, path, path_len);
    node->path[path_len] = '\0';
    node->data = data;
    node->size = size;
    node->type = type;
}

static const char* normalize_path(const char* path) {
    while (path[0] == '/') {
        path++;
    }

    if (path[0] == '.' && path[1] == '/') {
        return path + 2;
    }

    return path;
}

static void initrd_mount(void) {
    if (!osinfo.initrd || !osinfo.initrd->module_count) {
        return;
    }

    struct limine_file* module = osinfo.initrd->modules[0];
    if (!module || !module->address || !module->size) {
        return;
    }

    uint8_t* archive = (uint8_t*)module->address;
    uint8_t* end = archive + module->size;

    node_count = 0;

    while (archive + sizeof(struct tar_header) <= end) {
        struct tar_header* header = (struct tar_header*)archive;

        if (header->name[0] == '\0') {
            break;
        }

        size_t file_size = tar_parse_octal(header->size, sizeof(header->size));
        enum vfs_node_type type = (header->typeflag == '5') ? VFS_NODE_DIR : VFS_NODE_FILE;

        char full_path[VFS_PATH_MAX];
        memset(full_path, 0, sizeof(full_path));

        if (header->prefix[0] != '\0') {
            snprintf(full_path, sizeof(full_path), "%s/%s", header->prefix, header->name);
        }
        else {
            snprintf(full_path, sizeof(full_path), "%s", header->name);
        }

        vfs_add_node(normalize_path(full_path), archive + 512, file_size, type);

        archive += 512 + align_up_512(file_size);
    }

    log(INFO, "VFS mounted: %zu nodes\n", node_count);
}

void vfs_init(void) {
    initrd_mount();
}

size_t vfs_count(void) {
    return node_count;
}

const struct vfs_node* vfs_find(const char* path) {
    path = normalize_path(path);
    for (size_t i = 0; i < node_count; i++) {
        if (strcmp(nodes[i].path, path) == 0) {
            return &nodes[i];
        }
    }

    return NULL;
}

const struct vfs_node* vfs_find_by_index(size_t index) {
    if (index >= node_count) {
        return NULL;
    }
    return &nodes[index];
}

size_t vfs_read(const char* path, void* buffer, size_t size, size_t offset) {
    const struct vfs_node* node = vfs_find(path);
    if (!node || node->type != VFS_NODE_FILE || offset >= node->size) {
        return 0;
    }

    size_t available = node->size - offset;
    size_t to_copy = available < size ? available : size;
    memcpy(buffer, node->data + offset, to_copy);
    return to_copy;
}
