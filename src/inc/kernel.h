#ifndef kernel_h
#define kernel_h


void main(void);
extern int cursor_x;
extern int cursor_y;

inline int abs(int x)
{
    return x < 0 ? -x : x;
}

#endif