#include <net/net.h>

#include <debug/debug.h>
#include <libk/string.h>
#include <pci/pci.h>

#define NET_MAX_DEVICES 8

static struct net_device devices[NET_MAX_DEVICES];
static size_t device_count = 0;
static uint8_t loopback_buffer[2048];
static size_t loopback_length = 0;

static void net_register_device(const char* name) {
    if (device_count >= NET_MAX_DEVICES) {
        return;
    }

    struct net_device* device = &devices[device_count++];
    memset(device, 0, sizeof(*device));
    size_t name_length = strlen(name);
    if (name_length >= NET_DEVICE_NAME_MAX) {
        name_length = NET_DEVICE_NAME_MAX - 1;
    }
    memcpy(device->name, name, name_length);
    device->name[name_length] = '\0';
}

void net_init(void) {
    device_count = 0;
    loopback_length = 0;
    pci_init();
    net_register_device("lo0");
    debug("Network stack initialized with loopback\n");
}

size_t net_device_count(void) {
    return device_count;
}

const struct net_device* net_device_find(const char* name) {
    for (size_t i = 0; i < device_count; i++) {
        if (strcmp(devices[i].name, name) == 0) {
            return &devices[i];
        }
    }

    return NULL;
}

size_t net_send(const char* name, const uint8_t* data, size_t length) {
    struct net_device* device = NULL;
    for (size_t i = 0; i < device_count; i++) {
        if (strcmp(devices[i].name, name) == 0) {
            device = &devices[i];
            break;
        }
    }

    if (!device || !data || length == 0) {
        return 0;
    }

    if (strcmp(device->name, "lo0") == 0) {
        if (length > sizeof(loopback_buffer)) {
            length = sizeof(loopback_buffer);
        }
        memcpy(loopback_buffer, data, length);
        loopback_length = length;
        device->bytes_tx += length;
        debug("net: loopback queued %zu bytes\n", length);
        return length;
    }

    device->bytes_tx += length;
    return length;
}

size_t net_receive(const char* name, uint8_t* buffer, size_t length) {
    if (!buffer || length == 0) {
        return 0;
    }

    const struct net_device* device = net_device_find(name);
    if (!device) {
        return 0;
    }

    if (strcmp(device->name, "lo0") != 0 || loopback_length == 0) {
        return 0;
    }

    size_t to_copy = loopback_length < length ? loopback_length : length;
    memcpy(buffer, loopback_buffer, to_copy);
    loopback_length = 0;
    ((struct net_device*)device)->bytes_rx += to_copy;
    return to_copy;
}