#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
const char* sha="d60e7d3d2b47d19418af5b0ba52406b86ec6ef83";
// Copied from the manual
struct fat32hdr {
  u8  BS_jmpBoot[3];
  u8  BS_OEMName[8];
  u16 BPB_BytsPerSec;
  u8  BPB_SecPerClus;
  u16 BPB_RsvdSecCnt;
  u8  BPB_NumFATs;
  u16 BPB_RootEntCnt;
  u16 BPB_TotSec16;
  u8  BPB_Media;
  u16 BPB_FATSz16;
  u16 BPB_SecPerTrk;
  u16 BPB_NumHeads;
  u32 BPB_HiddSec;
  u32 BPB_TotSec32;
  u32 BPB_FATSz32;
  u16 BPB_ExtFlags;
  u16 BPB_FSVer;
  u32 BPB_RootClus;
  u16 BPB_FSInfo;
  u16 BPB_BkBootSec;
  u8  BPB_Reserved[12];
  u8  BS_DrvNum;
  u8  BS_Reserved1;
  u8  BS_BootSig;
  u32 BS_VolID;
  u8  BS_VolLab[11];
  u8  BS_FilSysType[8];
  u8  __padding_1[420];
  u16 Signature_word;
} __attribute__((packed));

struct entry {
  u8  DIR_Name[11];
  u8  DIR_Attr;
  u8  DIR_NTRes;
  u8  DIR_CrtTimeTenth;
  u16 DIR_CrtTime;
  u16 DIR_CrtDate;
  u16 DIR_LastAccDate;
  u16 DIR_FstClusHI;
  u16 DIR_WrtTime;
  u16 DIR_WrtDate;
  u16 DIR_FstClusLO;
  u32 DIR_FileSize;
} __attribute__((packed));
_Static_assert(sizeof(struct entry)==0x20,"Size of entry is wrong!");

struct long_entry{
  u8  LDIR_Ord;
  u16  LDIR_Name1[5];
  u8  LDIR_Attr;
  u8  LDIR_Type;//为0
  u8  LDIR_Chksum;//check sum 校验和
  u16  LDIR_Name2[6];
  u16 LDIR_FstClusLO;//为0
  u16  LDIR_Name3[2];
}__attribute__((packed));
_Static_assert(sizeof(struct long_entry)==0x20,"Size of long entry is wrong!");

struct bmp_header{
  u8  type[2];// BM
  u32 file_size;
  u16 unused[2];
  u32 offset;
}__attribute__((packed));

struct bmp_infomation_header{
  u32 header_size;
  u32 width;
  u32 height;
  u16 nplanes;
  u16 bits_per_pixel;
  u32 compress_type;
  u32 img_size;
  u32 hers;
  u32 vres;
  u32 ncolors;
  u32 nimpcolors;
}__attribute__((packed));

void *map_disk(const char *fname);

int main(int argc, char *argv[]) {
  if (argc < 2) {
    fprintf(stderr, "Usage: %s fs-image\n", argv[0]);
    exit(1);
  }

  setbuf(stdout, NULL);

  assert(sizeof(struct fat32hdr) == 512); // defensive

  // map disk image to memory
  struct fat32hdr *hdr = map_disk(argv[1]);

  // TODO: frecov
  u32 tot_sec=hdr->BPB_TotSec32;//扇区总数
  u32 offset_sec=hdr->BPB_RsvdSecCnt+hdr->BPB_NumFATs*hdr->BPB_FATSz32;//data开始扇区数
  u32 data_sec=tot_sec-offset_sec;//data总数
  u32 tot_clus=data_sec/hdr->BPB_SecPerClus;//clus总个数
  u32 clus_sz=hdr->BPB_BytsPerSec*hdr->BPB_SecPerClus;//一个clus大小
  u32 short_entry_cnt=clus_sz/sizeof(struct entry);//每个clus最多短目录数
  uintptr_t data_start=(uintptr_t)hdr+offset_sec*hdr->BPB_BytsPerSec;
  uintptr_t fat1=(uintptr_t)hdr+hdr->BPB_RsvdSecCnt*hdr->BPB_BytsPerSec;
  uintptr_t end=(uintptr_t)hdr+tot_sec*hdr->BPB_BytsPerSec;
  u8 *used=malloc(tot_clus+2);
  memset(used,0,tot_clus+2);
  char result[1024][128];int num=0;
  memset(result,'\0',sizeof(result));
  u32 first_clus[1024];
  memset(first_clus,0,sizeof(first_clus));
  for(int i=0;i<tot_clus;i++){
    uintptr_t addr=data_start+i*clus_sz;
    for(int j=0;j<short_entry_cnt;j++){
      struct entry *short_entry=(struct entry *)(addr+j*sizeof(struct entry));
      if(strncmp((char*)short_entry->DIR_Name+8,"BMP",3)==0){//BMP文件
        if(short_entry->DIR_Name[0]==0xE5||short_entry->DIR_FileSize==0)continue;
        //this if all short entry;
        //printf("%s\n",short_entry->DIR_Name);输出所有文件名。97 right

        first_clus[num]=((((uint)short_entry->DIR_FstClusHI)<<16)|((uint)short_entry->DIR_FstClusLO))-2;
        struct bmp_header *bmp_fp=(struct bmp_header*)(data_start+first_clus[num]*clus_sz);
        if((uintptr_t)bmp_fp>=end||strncmp((char*)bmp_fp->type,"BM",2)!=0)continue;
        used[first_clus[num]]=1;





        int flag=1;int index=0;char filename[128];
        memset(filename,'\0',sizeof(filename));
        for(int k=j-1;k>=0;k--){
          struct long_entry *long_entry=(struct long_entry *)(addr+k*sizeof(struct entry));
          if(long_entry->LDIR_Attr==15&&long_entry->LDIR_Type==0&&long_entry->LDIR_FstClusLO==0){
            flag=0;//
            for(int l=0;l<5;l++){
              if(long_entry->LDIR_Name1[l]==0xFFFF)continue;
              filename[index++]=(char)long_entry->LDIR_Name1[l];
            }
            for(int l=0;l<6;l++){
              if(long_entry->LDIR_Name2[l]==0xFFFF)continue;
              filename[index++]=(char)long_entry->LDIR_Name2[l];
            }
            for(int l=0;l<2;l++){
              if(long_entry->LDIR_Name3[l]==0xFFFF)continue;
              filename[index++]=(char)long_entry->LDIR_Name3[l];
            }
          }
          else break;
        }
        if(flag==1){
            for(int l=0;l<11;l++)filename[index++]=(char)short_entry->DIR_Name[l];
          }
        strcpy(result[num++],filename);
      }
    }
  }
  //todo:recover
  for(int i=0;i<num;i++){
    //todo:write to tmp
    char tmp_path[128]="tmp/DICM";
    strcat(tmp_path,result[i]);
    struct bmp_header *bmp_fp=(struct bmp_header*)(data_start+first_clus[i]*clus_sz);
    FILE *bmp_tmp_file=fopen(tmp_path,"a");
    fwrite(bmp_fp,sizeof(struct bmp_header),1,bmp_tmp_file);
    struct bmp_infomation_header *bmp_ip=(struct bmp_infomation_header*)(bmp_fp+1);
    fwrite(bmp_ip,sizeof(struct bmp_infomation_header),1,bmp_tmp_file);
    uintptr_t img_start=(uintptr_t)(bmp_fp+bmp_fp->offset);
    if(bmp_ip->img_size>clus_sz-sizeof(struct bmp_header)-sizeof(struct bmp_infomation_header)){
      //多个簇
      continue;
    }
    else{
      fwrite(img_start,bmp_ip->img_size,1,bmp_tmp_file);
    }
    fclose(bmp_tmp_file);

    char buf[64];
    char file_path[128]="sha1sum /tmp/DICM";
    strcat(file_path,result[i]);
    FILE *fp=popen(file_path,"r");
    fscanf(fp,"%s",buf);
    pclose(fp);
    printf("%s %s\n",buf,result[i]);
  }


  // file system traversal
  munmap(hdr, hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec);
}

void *map_disk(const char *fname) {
  int fd = open(fname, O_RDWR);

  if (fd < 0) {
    perror(fname);
    goto release;
  }

  off_t size = lseek(fd, 0, SEEK_END);
  if (size == -1) {
    perror(fname);
    goto release;
  }

  struct fat32hdr *hdr = mmap(NULL, size, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);
  if (hdr == (void *)-1) {
    goto release;
  }

  close(fd);

  if (hdr->Signature_word != 0xaa55 ||
      hdr->BPB_TotSec32 * hdr->BPB_BytsPerSec != size) {
    fprintf(stderr, "%s: Not a FAT file image\n", fname);
    goto release;
  }
  return hdr;

release:
  if (fd > 0) {
    close(fd);
  }
  exit(1);
}
