#ifndef PIPE_H
#define PIPE_H
#define BUFFER_SIZE 256
#define MAX_NUM_DESCRIPTORS 64

class Descriptor {
public:
    char* buff;
    char* desType;
    char* persimission;
    int currSeekPos;
    int* dataSize;
    bool ownsSharedState;

    Descriptor(char* desType,char* permission,char* buff, int* dataSize,
               bool ownsSharedState);
    ~Descriptor();
};  

class PipeDescriptors{
public:
    PipeDescriptors();
    Descriptor* descriptors[MAX_NUM_DESCRIPTORS];

    int createDes(int* x,int* y,char *desType);
    int readDes(int id, char* buff, int size);
    int writeDes(int id, char* buff, int size);
    void test_method();
};

#endif