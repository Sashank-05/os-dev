#pragma once

#include <stddef.h>
#include <stdint.h>

#define NET_DEVICE_NAME_MAX 16

struct net_device {
    char name[NET_DEVICE_NAME_MAX];
    uint64_t bytes_tx;
    uint64_t bytes_rx;
};

void net_init(void);
size_t net_device_count(void);
const struct net_device* net_device_find(const char* name);
size_t net_send(const char* name, const uint8_t* data, size_t length);
size_t net_receive(const char* name, uint8_t* buffer, size_t length);
