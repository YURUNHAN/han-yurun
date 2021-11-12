/****************************************************************************
 *                   KCW testing ext2 file system                            *
 *****************************************************************************/

#include "cd_ls_pwd.h"
#include "globals.h"
#include "link_unlink.h"
#include "mkdir_create.h"
#include "rmdir.h"
#include "stat.h"
#include "type.h"
#include "util.h"
#include "miscl.h"

int init()
{
        int i, j;
        MINODE *mip;
        PROC *p;

        printf("init()\n");

        for (i = 0; i < NMINODE; i++) {
                mip = &minode[i];
                mip->dev = mip->ino = 0;
                mip->refCount = 0;
                mip->mounted = 0;
                mip->mntPtr = 0;
        }
        for (i = 0; i < NPROC; i++) {
                p = &proc[i];
                p->pid = i;
                p->uid = p->gid = 0;
                p->cwd = 0;
                p->status = FREE;
                for (j = 0; j < NFD; j++)
                        p->fd[j] = 0;
        }
        return 0;
}

int quit()
{
        int i;
        MINODE *mip;
        for (i = 0; i < NMINODE; i++) {
                mip = &minode[i];
                if (mip->refCount > 0)
                        iput(mip);
        }
        exit(0);
}

// load root INODE and set root pointer to it
int mount_root()
{
        printf("mount_root()\n");
        root = iget(dev, 2);
        return 0;
}

char *disk = "diskimage";
int main(int argc, char *argv[])
{
        char buf[BLKSIZE];
        char line[128], cmd[32], pathname[128], otherPathname[128];

        if (argc > 1)
                disk = argv[1];

        // comment@Open diskimage, fd initially points to the beginning of the file
        printf("checking EXT2 FS ....");
        if ((fd = open(disk, O_RDWR)) < 0) {
                printf("open %s failed\n", disk);
                exit(1);
        }
        dev = fd;

        /********** read super block  ****************/
        // comment@Read the first 1024 bytes of content, it is a super block
        get_block(dev, 1, buf);
        sp = (SUPER *)buf;

        /* verify it's an ext2 file system ***********/
        if (sp->s_magic != 0xEF53) {
                printf("magic = %x is not an ext2 filesystem\n", sp->s_magic);
                exit(1);
        }
        printf("EXT2 FS OK\n");
        ninodes = sp->s_inodes_count;
        nblocks = sp->s_blocks_count;

        // comment@Then read the content of 2048 bytes, which is a group descriptor
        get_block(dev, 2, buf);
        gp = (GD *)buf;

        bmap = gp->bg_block_bitmap;
        imap = gp->bg_inode_bitmap;
        inode_start = gp->bg_inode_table;
        printf("bmp=%d imap=%d inode_start = %d\n", bmap, imap, inode_start);

        init();
        mount_root();
        printf("root refCount = %d\n", root->refCount);

        // comment@Run the program on process 0, and the current directory of process 0 is the root
        printf("creating P0 as running process\n");
        running = &proc[0];
        running->status = READY;
        running->cwd = iget(dev, 2);
        printf("root refCount = %d\n", root->refCount);

        for (int i = 1; i < NPROC; i++) {
                proc[i].cwd = iget(dev, 2);
        }

        // comment@Read the command loop
        while (1) 
        {
                // printf("input command : [ls|cd|pwd|mkdir|creat|rmdir|link|unlink|symlink|utime|chmod|readlink|quit] ");
                printf("<<< enter command : \n<<< ls | cd | pwd | mkdir | creat | rmdir | link | unlink | symlink | exit\n>>> ");
                fgets(line, 128, stdin);
                line[strlen(line) - 1] = 0;

                if (line[0] == 0)
                        continue;
                pathname[0] = 0;
                otherPathname[0] = 0;

                // comment@Read in three strings, store the command, the path, and the other path respectively
                sscanf(line, "%s %s %s", cmd, pathname, otherPathname);
                printf("<<< command=[ %s ], arg1=[ %s ], arg2=[ %s ]\n", cmd, pathname, otherPathname);

                if (strcmp(cmd, "ls") == 0)
                        ls(pathname);
                if (strcmp(cmd, "cd") == 0)
                        chdir(pathname);
                if (strcmp(cmd, "pwd") == 0)
                        pwd(running->cwd);
                if (strcmp(cmd, "creat") == 0)
                        creat_local(pathname);
                if (strcmp(cmd, "mkdir") == 0)
                        mkdir_local(pathname);
                if (strcmp(cmd, "rmdir") == 0)
                        rmdir_local(pathname);
                if (strcmp(cmd, "link") == 0)
                        link(pathname, otherPathname);
                if (strcmp(cmd, "symlink") == 0)
                        symlink(pathname, otherPathname);
                if (strcmp(cmd, "unlink") == 0)
                        unlink(pathname);
                // comment@The following commands are not used
                /*
                if (strcmp(cmd, "stat") == 0)
                        local_stat(pathname);
                if (strcmp(cmd, "utime") == 0)
                        utime_local(pathname);
                if (strcmp(cmd, "chmod") == 0){
                        char *ptr = NULL;
                        chmod_local(otherPathname, strtol(pathname, &ptr, 8));
                }
                if (strcmp(cmd, "readlink") == 0) {
                        char buffer[BLKSIZE];
                        int s = readLink(pathname, buffer);
                        printf("Link: %s Link Size: %d\n", buffer, s);
                }
                */
                if (strcmp(cmd, "exit") == 0)
                        quit();
        }
}
