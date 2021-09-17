#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>

int fd;
char buf[512];
// u8 == 1byte
typedef unsigned char  u8;
// u16 = 2 byte
typedef unsigned short u16;
// u32 = 4 byte
typedef unsigned int   u32;

typedef struct partition
{
	u8 drive;             /* drive number FD=0, HD=0x80, etc. */

	u8  head;             /* starting head */
	u8  sector;           /* starting sector */
	u8  cylinder;         /* starting cylinder */

	u8  sys_type;         /* partition type: NTFS, LINUX, etc. */

	u8  end_head;         /* end head */
	u8  end_sector;       /* end sector */
	u8  end_cylinder;     /* end cylinder */

	u32 start_sector;     /* starting sector counting from 0 */
	u32 nr_sectors;       /* number of of sectors in partition */
} Partition;

void PartitionInfo(Partition * p);
void RecursiveEXTpartition(Partition * p, u32 base_sector);

int main(void)
{
	char sectorONE[512];
	// open vdisk for READ ONLY returns 3
	fd = open("vdisk", O_RDONLY);  // check fd value: -1 means open() failed
	read(fd, buf, 512);            // read sector 0 into buf[ ]
	//partition table begins at the byte offset 0x1BE of the MBR
	memcpy(sectorONE, buf, 512);
 	Partition * MBR = (Partition *)&sectorONE[0x1BE];
 	Partition * LocalMBR;
	//Partition Table consists of only 4 entries
	for(int i = 0; i < 4; i++)
	{
		PartitionInfo(MBR);
		// implies EXT partition
		if(MBR->sys_type == 5)
		{ 
			// base sector for EXT drive = MBR->start_sector
			RecursiveEXTpartition(MBR, MBR->start_sector);
		}
		MBR++;
	}
	return 0;
}


void PartitionInfo(Partition * p)
{
	printf("sys_type = '%d\n", p->sys_type);
	printf("start_sector = %u\n", p->start_sector);
	printf("end_sector %u\n", (p->start_sector + p->nr_sectors - 1));
	printf("nr_sectors = %u\n\n", p->nr_sectors);
}

void RecursiveEXTpartition(Partition * p, u32 base_sector)
{
	//just for seeking
	u32 abs_sector;
	//end of link list 
	if(p->start_sector == 0)
	{	
		return;
	}
	if(base_sector == p->start_sector)
	{
		abs_sector = p->start_sector;
	}
	else
	{
		abs_sector = base_sector + p->start_sector;
	}
	lseek(fd, (long) (abs_sector*512), SEEK_SET);
	read(fd, buf, 512);
	p = (Partition *)&buf[0x1BE];
	printf("sys_type: %u\n", p->sys_type);
	printf("start_sector %u\n", abs_sector + p->start_sector);
	printf("end_sector %u\n", abs_sector + p->start_sector + p->nr_sectors - 1);
	printf("nr_sectors %u\n\n", p->nr_sectors);
	RecursiveEXTpartition(++p, base_sector);				
}
