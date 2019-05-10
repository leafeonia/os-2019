#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#define ERR(s)\
	{printf("error: %s\n",s);\
	exit(1);}


struct __attribute__((packed)) boot_info{
    unsigned char dummy[11];
    unsigned short sector_size;
    unsigned char cluster;
    unsigned short reserved_sector;
    unsigned short nr_fat;
    unsigned char dummy2[14];
    unsigned int nr_sector;
    unsigned int fat_sector;
};

struct __attribute__((packed)) long_entry{
    unsigned char check;
    unsigned short filename1[5];
    unsigned char dummy1[3];
    unsigned short filename2[6];
    unsigned short dummy2;
    unsigned short filename3[2];
};

struct __attribute__((packed)) short_entry{
    unsigned char dummy[26];
    unsigned short cluster;
    unsigned int size;
};

struct __attribute__((packed)) bmp{
    unsigned short magic;
    unsigned int size;
};


int main(int argc, char *argv[]) {
    struct boot_info* p;
    int fd = open(argv[1],O_RDONLY);
    if(fd == -1){
        ERR("open file failed");
    }
    struct stat st;
    if(fstat(fd,&st) == -1){
        close(fd);
        ERR("obtain file size failed");
    }
    int length = st.st_size;
    p = mmap(NULL, length, PROT_READ, MAP_SHARED, fd, 0);
    if(!p || p == (void*)-1){
        close(fd);
        ERR("mmap failed");
    }
    void* data_base = (void*)((char*)p+((p->reserved_sector + p->nr_fat * p->fat_sector - 2)*p->sector_size));
    int cluster_size = p->sector_size*p->cluster;
    //printf("%x\n\n",data_base);
    for(char* ptr = (char*)p;ptr < (char*)p + length - 2;ptr++){
        if(*ptr == 'B' && *(ptr+1) == 'M' && *(ptr+2) == 'P' && *(ptr+3) == ' ') {
            //printf("%x\n",ptr - (char*)p);
            struct short_entry* shortEntry = (short_entry*)(ptr - 8);
            struct long_entry* longEntry = (long_entry*)(ptr - 40);
            if(!shortEntry->cluster) continue;
            char filename[50];
            memset(filename,0, sizeof(filename));
            int longEntryNum = -1;
            int flag = 0;
            while(1){
                for (int i = 0; i < 5; ++i) {
                    unsigned char ch = (unsigned char)(longEntry->filename1[i]);
                    if(!ch) break;
                    if(ch < 0x20 || ch > 0x7e) {
                        flag = 1;
                        break;
                    };
                    filename[strlen(filename)] = ch;
                }
                for (int i = 0; i < 6; ++i) {
                    unsigned char ch = (unsigned char)(longEntry->filename2[i]);
                    if(!ch) break;
                    if(ch < 0x20 || ch > 0x7e) {
                        flag = 1;
                        break;
                    };
                    filename[strlen(filename)] = ch;
                }
                for (int i = 0; i < 2; ++i) {
                    unsigned char ch = (unsigned char)(longEntry->filename3[i]);
                    if(!ch) break;
                    if(ch < 0x20 || ch > 0x7e) {
                        flag = 1;
                        break;
                    };
                    filename[strlen(filename)] = ch;
                }
                longEntryNum++;
                char checkByte = 'A'+longEntryNum;
                if(longEntry->check == checkByte) {
                    flag = 2;//successful
                    break;
                }
                if(flag == 1 || !longEntry->check) break;

                longEntry = (long_entry*)((char*)longEntry - 32;)
            };

            if(flag == 2){

                struct bmp* image = (struct bmp*)((char*)data_base + shortEntry->cluster * cluster_size);
                //printf("%X   ",shortEntry->size);
                //int nr_cluster = shortEntry->size / cluster_size;
                //printf("clusters: %d\n",nr_cluster);
                FILE* fp = fopen(filename, "w");
                if(fp){
                    for(char* fptr = (char*)image; fptr < (char*)image + shortEntry->size; fptr++){
                        fprintf(fp, "%c", *(fptr));
                    }
                }
                char command[50];
                sprintf(command,"sha1sum %s",filename);
                system(command);

            }
        }
    }
    munmap(p,length);
    close(fd);
    return 0;
}
