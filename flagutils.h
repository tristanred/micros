#ifndef FLAGS_H
#define FLAGS_H

#define FLAG(x, f) ((x & f) == f)

#define FLAGSET(x, f) (x &= f)
#define FLAGUNSET(x, f) (x &= ~f)

#endif
