#include "type.h"


char *name[64]; // token string pointers
char gline[256]; // holds token strings, each pointed by a name[i]
int nname; // number of token strings

MINODE minode[NMINODE];
MTABLE mtable[NMTABLE];

// globals
MINODE minode[NMINODE];
MINODE *root;

PROC proc[NPROC], *running;

char gpath[128]; // global for tokenized components
//char *name[32];  // assume at most 32 components in pathname
int   n;         // number of component strings

int fd, dev;
int nblocks, ninodes, bmap, imap, inode_start;

MINODE *iget();


MINODE *mialloc()
// allocate a FREE minode for use
{
    int i;
    for (i=0; i<NMINODE; i++){
        MINODE *mp = &minode[i];
        if (mp->refCount == 0){
            mp->refCount = 1;
            return mp; 
        }
    }
    printf("FS panic: out of minodes\n");
    return 0;
}
int midalloc(MINODE *mip) // release a used minode
{
    mip->refCount = 0;
}



