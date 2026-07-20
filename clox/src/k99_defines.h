#ifndef K99_DEFINES_H
#define K99_DEFINES_H

#define internal static
#define local_persist static
#define global static

#define Kilobytes(Value) ((Value) * 1024LL)
#define Megabytes(Value) (Kilobytes(Value) * 1024LL)
#define Gigabytes(Value) (Megabytes(Value) * 1024LL)
#define Terabytes(Value) (Gigabytes(Value) * 1024LL)
#define Petabytes(Value) (Terabytes(Value) * 1024LL)

#endif
