struct sourcePipeLog{
    int source;
    int pipe;
    int log;
};
struct sourcePipeLog openSPL(char *s, char *p, char *l);
void closeSPL(struct sourcePipeLog spl);