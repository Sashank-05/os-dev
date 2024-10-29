#ifndef kernel_h
#define kernel_h


void main(void);
extern int cursor_x;
extern int cursor_y;

inline int abs(int x)
{
    return x < 0 ? -x : x;
}

inline char* reverse(char *s)
{
    int i = 0;
    int j = strlen(s) - 1;
    while (i < j)
    {
        char c = s[i];
        s[i] = s[j];
        s[j] = c;
        i++;
        j--;
    }
    return s;
}

inline atoi (const char *s)
{
    int n = 0;
    while (*s >= '0' && *s <= '9')
    {
        n = n * 10 + (*s - '0');
        s++;
    }
    return n;
}

inline char* itoa (int n, char *s)
{
    int i = 0;
    while (n > 0)
    {
        s[i++] = n % 10 + '0';
        n /= 10;
    }
    s[i] = '\0';
    reverse(s);
}

#endif