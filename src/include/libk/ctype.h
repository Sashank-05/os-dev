#pragma once

#include <stddef.h>
#include <stdint.h>
#include "string.h"

int isdigit(int c) {
    return c >= '0' && c <= '9';
}

int isspace(int c) {
    return c == ' ' || (c >= '\t' && c <= '\r');
}

int isupper(int c) {
    return c >= 'A' && c <= 'Z';
}

int islower(int c) {
    return c >= 'a' && c <= 'z';
}

int isalpha(int c) {
    return isupper(c) || islower(c);
}

int isalnum(int c) {
    return isalpha(c) || isdigit(c);
}

int isprint(int c) {
    return c >= 0x20 && c <= 0x7E;
}

int isxdigit(int c) {
    return isdigit(c) || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

int tolower(int c) {
    if (isupper(c)) {
        return c + 32;
    }
    return c;
}

int toupper(int c) {
    if (islower(c)) {
        return c - 32;
    }
    return c;
}

int ends_with(const char *str, const char *suffix) {
    size_t str_len = strlen(str);
    size_t suffix_len = strlen(suffix);

    if (suffix_len > str_len) {
        return 0;
    }

    return !memcmp(str + str_len - suffix_len, suffix, suffix_len);
}

int starts_with(const char *str, const char *prefix) {
    size_t prefix_len = strlen(prefix);
    size_t str_len = strlen(str);

    if (prefix_len > str_len) {
        return 0;
    }

    return !memcmp(str, prefix, prefix_len);
}

