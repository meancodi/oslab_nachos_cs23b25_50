#include "pipe.h"
#include "debug.h"
#include "main.h"
using namespace std;

Descriptor::Descriptor(char* desType,char* permission,char* buffPtr,
                       int* sharedDataSize, bool ownsSharedState){
    buff = buffPtr;
    this->desType = desType;
    this->persimission = permission;
    this->currSeekPos = 0;
    this->dataSize = sharedDataSize;
    this->ownsSharedState = ownsSharedState;
}

Descriptor::~Descriptor(){
    if (ownsSharedState) {
        delete[] buff;
        delete dataSize;
    }
}

PipeDescriptors::PipeDescriptors(){
    for(int i=0;i<MAX_NUM_DESCRIPTORS;i++){
        descriptors[i] = NULL;
    }
}

int PipeDescriptors::createDes(int* x,int* y,char* desType){
    char* buf = new char[BUFFER_SIZE];
    int* sharedDataSize = new int(0);
    int parentPd = -1,childPd = -1;
    int freeDes_cnt = 0;

    for(int i=0;i<MAX_NUM_DESCRIPTORS;i++){
        if(descriptors[i] == NULL) freeDes_cnt++;
    }

    if(freeDes_cnt < 2) return -1;

    Descriptor* parentDes = new Descriptor(desType,"w",buf, sharedDataSize, true);
    Descriptor* childDes = new Descriptor(desType,"r",buf, sharedDataSize, false);

    for(int i=0;i<MAX_NUM_DESCRIPTORS;i++){
        if(descriptors[i] == NULL){ parentPd = i; break; }
    }

    for(int i=0;i<MAX_NUM_DESCRIPTORS;i++){
        if(descriptors[i] == NULL && i != parentPd){ childPd = i; break; }
    }

    descriptors[parentPd] = parentDes;
    descriptors[childPd] = childDes;

    *x = parentPd;
    *y = childPd;

    return 1;
}

int PipeDescriptors::readDes(int des, char* buf, int numBytes) {
    if (des < 0 || des >= MAX_NUM_DESCRIPTORS) return -1;
    Descriptor* pDes = descriptors[des];
    if (pDes == NULL) return -1;
    char* pipeBuf = pDes->buff;
    while (*pDes->dataSize <= pDes->currSeekPos) {
        kernel->currentThread->Yield();
    }

    int available = *pDes->dataSize - pDes->currSeekPos;
    int numRead = (numBytes < available) ? numBytes : available;

    for (int i = 0; i < numRead; i++) {
        buf[i] = pipeBuf[pDes->currSeekPos + i];
    }
    pDes->currSeekPos += numRead;
    return numRead;
}

int PipeDescriptors::writeDes(int id,char* buff,int size){
    if(id < 0 || id>=MAX_NUM_DESCRIPTORS) return -1;
    Descriptor* pDes = descriptors[id];
    if (pDes == NULL) return -1;

    int numWrite = 0;
    int startPos = *pDes->dataSize;
    for(int i=0;i<size && startPos + i < BUFFER_SIZE;i++){
        pDes->buff[startPos + i] = buff[i];
        numWrite++;
    }
    *pDes->dataSize += numWrite;
    pDes->currSeekPos = *pDes->dataSize;
    return numWrite;
}

void PipeDescriptors::test_method(){
    int des1,des2;
    char* buf = new char[BUFFER_SIZE];
    createDes(&des1,&des2,"pipe");
    writeDes(des1,"Hello world",11);
    readDes(des2,buf,11);
    delete[] buf;
}