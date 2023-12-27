#include <fcntl.h>
#include <unistd.h>
#include "spl.h"

struct sourcePipeLog openSPL(char *s, char *p, char *l){
    struct sourcePipeLog spl;
    spl.source = open(s, O_RDONLY);
    spl.pipe = open(p, O_WRONLY);
    spl.log = open(l, O_WRONLY | O_TRUNC);

    return spl;
}
void closeSPL(struct sourcePipeLog spl){
    close(spl.source);
    close(spl.pipe);
    close(spl.log);
}