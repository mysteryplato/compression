/**
 *  @brief 压缩数据：  1.压缩方式分为直接压缩和拼接后压缩
 *                   2.将表头和映射后的数据存放在byte数组中
 *                   3.将byte数组保存在文件中
 *         解压缩数据：1.从文件中读取数据
 *                   2.将数据从char型映射回float型
 *                   3.计算解压后的数据与原始数据的差值并写入到文件中
 *
 *  @author plato
 *  @version 1.00 2017-11-08 plato
 *                note:create it
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <dirent.h>
#include <sys/stat.h>
#include <time.h>
#include "datageneration.h"
#include "imagecompression.h"

//>>>----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//grace171019 压缩函数集合

void compressData(float *pFloatData,
                  short *pArrLength)
{
    removeDir(COMPRESSION_DIR);             // 删除目录及文件
    mkdir(COMPRESSION_DIR,0777);            // 创建目录

    clock_t startTime, endTime;             // 压缩开始,结束时间

    short longArrLenArr[ARR_COUNT];         // 保存直接大于4096的数组
    short shortArrLenArr[ARR_COUNT];        // 保存小于4096的数组
    int longArrIdx = 0,shortArrIdx = 0;     // 保存搜索数组索引号

    short arrLenIndex = 0;                  // float数组长度索引号
    short byteDataLength;                   // 压缩后的byte数据的长度
    unsigned char *pByteData;               // 压缩后的byte数据指针
    int dataOffset;                         // 保存原始浮点数据数组在存储中的偏移

    char filename[FILENAME_MAX];            // 保存文件名的数组

    startTime = clock();                    // 获取开始时间
    //>>>-------------------------------------------------------------------------------------------------------------------------------------
    //1 搜索数组长度大于4096的数组索引更新到 longArrLenArr 和 shortArrLenArr数组
    for ( int i = 0; i < ARR_COUNT; ++i )
    {
        if( pArrLength[i] > COMPRESSION_LIMIT )
        {
            longArrLenArr[longArrIdx++] = i;
        }
        else
        {
            shortArrLenArr[shortArrIdx++] = i;
        }
    }

    //>>>-------------------------------------------------------------------------------------------------------------------------------------
    //2 将数组长度大于4096的数组 即longArrLenArr中的数组进行压缩
    for ( int i = 0; i < longArrIdx; ++i )
    {
        arrLenIndex = longArrLenArr[i];              // 获取pArrLength指针索引号
        // 申请byte数据所需大小的内存空间
        byteDataLength = FIRST_HEADER_LENGTH + SECOND_HEADER_LENGTH + pArrLength[arrLenIndex];
        pByteData = (unsigned char *)malloc(byteDataLength * sizeof(char));
        if( pByteData == NULL )
        {
            printf("内存开辟失败\n");
            exit(1);
        }

        *pByteData = 1;                              // 标记本数据指针存储的数组个数
        dataOffset = getDataOffset(pArrLength,arrLenIndex); // 获取原始数据起始偏移
        // 将浮点数压入字节数组
        calcCompressedData((pFloatData + dataOffset),
                           pArrLength[arrLenIndex],
                           (pByteData+1));
        sprintf(filename,COMPRESSION_DIR"/"COMPRESSION_FILENAME,arrLenIndex);
        writeDataToFile(filename,pByteData,byteDataLength);  // 将byte数据写入文件
        free(pByteData);
    }

    //>>>-------------------------------------------------------------------------------------------------------------------------------------
    //3 将数组长度小于4096的数组 即shortArrLenArr中的数组进行压缩
    int compressionCount = 0;                         // 一个byte数组压缩的数组个数
    int tempIndex = 0;                                // 保存临时索引号
    char jointFilename[FILENAME_MAX];                 // 保存连接的文件名
    for ( int i = 0; i < shortArrIdx; ++i )
    {
        byteDataLength = 0;
        compressionCount = 0;
        tempIndex = i;
        int offset = 1;                               // 多数组存放时的偏移

        //3.1 根据搜索到的待压缩节点数组
        for( int j = i; j < shortArrIdx; ++j )
        {
            arrLenIndex = shortArrLenArr[j];          // 获取存储的数组长度索引号
            byteDataLength += pArrLength[arrLenIndex];
            ++compressionCount;                       // 压缩的个数计数
            // 累加满足4096，或为最后一个数据
            if( byteDataLength > COMPRESSION_LIMIT || j == (shortArrIdx - 1) )
            {
                i = j;                                // 更新下一次搜索压缩包的起点
                break;
            }
        }

        //3.2 申请数据空间
        byteDataLength += FIRST_HEADER_LENGTH + SECOND_HEADER_LENGTH * compressionCount;
        pByteData = (unsigned char *)malloc(byteDataLength * sizeof(char));
        if( pByteData == NULL )
        {
            printf("内存开辟失败！");
            exit(1);
        }
        *pByteData = compressionCount;                 // 数据包存储数组个数

        //3.3 循环将相应数据压缩
        for( int j = tempIndex; j < (tempIndex + compressionCount); ++j )
        {
            arrLenIndex = shortArrLenArr[j];           // 获取pArrLength数组索引
            // 获取原始数据起始偏移
            dataOffset = getDataOffset(pArrLength,arrLenIndex);
            // 将浮点数压入字节数组
            calcCompressedData((pFloatData + dataOffset),
                               pArrLength[arrLenIndex],
                               (pByteData+offset));

            offset += pArrLength[arrLenIndex]+SECOND_HEADER_LENGTH;
            if( j == tempIndex )
                sprintf(filename,COMPRESSION_DIR"/"COMPRESSION_FILENAME,arrLenIndex);
            else
            {
                sprintf(jointFilename,"_"COMPRESSION_FILENAME,arrLenIndex);
                strcat(filename,jointFilename);
            }
        }

        //3.4 写出压缩数据到文件并释放申请的数据
        writeDataToFile(filename,pByteData,byteDataLength);  // 将byte数据写入文件
        free(pByteData);
    }

    endTime = clock();                                       // 获取结束时间
    double compressionTime;                                  // 压缩运行时间
    compressionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("压缩时间为：%fs\n",compressionTime);
}

void calcCompressedData(float *pFloatData,
                        short arrLength,
                        unsigned char *pByteData)
{
    int tempData,dataMax = 0,dataMin = DATA_MAX / 100;
    unsigned char byteData;                 // 求压缩比例的中转值
    float scale = 0.0;                      // 压缩比例

    *(pByteData + 0) = arrLength;           // 存储数组长度
    *(pByteData + 1) = arrLength >> 8;

    for( int i = 0; i < arrLength; ++i )    // 找最大最小值
    {
        tempData = (int) *(pFloatData + i);
        if(dataMax < tempData) dataMax = tempData;
        if(dataMin > tempData) dataMin = tempData;
    }

    *(pByteData + 2) = dataMin;             // 存储数组最小值
    *(pByteData + 3) = dataMin >> 8;

    *(pByteData + 4) = dataMax;             // 存储数组最大值
    *(pByteData + 5) = dataMax >> 8;

    scale = (float)(dataMax - dataMin) / 255.0;

    for( int i = 0; i < arrLength; ++i )    // 压缩存储数据
    {
        tempData = (int) *(pFloatData+i);
        byteData = (tempData - dataMin) / scale;
        *(pByteData + SECOND_HEADER_LENGTH + i) = byteData;
    }
}

void writeDataToFile(char *pFilename,
                     unsigned char *pByteData,
                     int dataLength)
{
    FILE *pFile;

    if( (pFile = fopen(pFilename, "w")) == NULL )
    {
        printf("%s open failed!",pFilename);
        exit(0);
    }
    for( int i = 0; i < dataLength; ++i )
    {
        fputc(*(pByteData + i),pFile);
    }
    fclose(pFile);
}
//<<<----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------


//>>>----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//grace171023 解压缩函数

void decompressData(float *pFloatData,
                    short *pArrLength)
{
    removeDir(DATA_DIFF_DIR);                   // 删除目录及文件
    mkdir(DATA_DIFF_DIR,0777);                  // 创建目录

    clock_t startTime, endTime;                 // 压缩开始,结束时间
    // 表头总长度
    short totalHeaderLength = FIRST_HEADER_LENGTH + SECOND_HEADER_LENGTH;

    int compressionCount;                       // 压缩的数组的个数
    short *pHeader;                             // 表头指针

    float *pNewFloatData;                       // 存放char型转float型数据
    unsigned char *pOrigByteData;               // 存放原始byte数据指针

    int filenameNumber = 0;                     // 当前文件名中的数字

    startTime = clock();                        // 获取开始时间
    //>>>-------------------------------------------------------------------------------------------------------------------------------------
    //1 打开目录
    DIR *pDir;                                  // 待打开的目录
    struct dirent *pEntry = NULL;               // 待打开的文件路径
    pDir = opendir(COMPRESSION_DIR"/");         // 找出目录中的文件
    if ( pDir == NULL )                         // 如果目录不存在，打印错误信息
    {
        perror ("Couldn't open the directory!");
    }
    //>>>-------------------------------------------------------------------------------------------------------------------------------------
    //2 遍历目录
    char filename[FILENAME_MAX];
    while ( (pEntry = readdir(pDir)) != NULL )  // while循环读取目录中的文件名
    {
        // 如果是当前目录"."或上层目录".."则继续循环，否则向下执行程序
        if( strcmp(pEntry->d_name,".") == 0 ||
                strcmp(pEntry->d_name,"..") == 0 )
        {
            continue;
        }
        // 将文件名加上路径，赋值给filename
        sprintf(filename,COMPRESSION_DIR"/%s",pEntry->d_name);
        //>>>-------------------------------------------------------------------------------------------------------------------------------------
        //2.1 打开文件
        FILE *pFile;                                     // 文件指针
        unsigned char headerArr[totalHeaderLength];      // 表头数组
        if( (pFile = fopen(filename,"rb+") ) == NULL)    // 打开文件失败退出
        {
            printf("\nCannot open file!");
            exit(1);
        }
        //>>>-------------------------------------------------------------------------------------------------------------------------------------
        //2.2 还原数据
        // 把数组个数存放到headerArr数组中
        fread(headerArr,sizeof(char),FIRST_HEADER_LENGTH,pFile);
        compressionCount = headerArr[0];     // 保存数组个数，后面headerArr[0]会改变
        pHeader = (short*)headerArr;
        for( int i = 0; i < compressionCount; ++i )
        {
            // 保存byte数据长度最大值最小值
            fread(headerArr,sizeof(char),SECOND_HEADER_LENGTH,pFile);
            // 存放原始byte数据
            pOrigByteData = malloc(pHeader[0] * sizeof(char));
            if( pOrigByteData == NULL )
            {
                printf("内存开辟失败\n");
                exit(1);
            }
            // 存放解压后新的float数据
            pNewFloatData = malloc(pHeader[0] * sizeof(float));
            if( pNewFloatData == NULL )
            {
                printf("内存开辟失败\n");
                exit(1);
            }
            // 读取byte数据
            fread(pOrigByteData,sizeof(char),pHeader[0] * sizeof(char),pFile);
            // 获取文件名数字部分，并由字符串转为数字
            filenameNumber = ( pEntry->d_name[8*i+4] - '0' ) * 100 +
                    ( pEntry->d_name[8*i+5] - '0' ) * 10  +
                    ( pEntry->d_name[8*i+6] - '0' ) * 1;
            // 将byte数据转为float数据
            calcDecompressedData(pHeader,
                                 pNewFloatData,
                                 pOrigByteData);
            // 计算原始数据与解压后数据的差值
            calcDiffAndWriteToFile(filenameNumber,
                                   (int)pHeader[0],
                                    pArrLength,
                                    pFloatData,
                                    pNewFloatData);

            // 释放指针
            free(pOrigByteData);
            pOrigByteData = NULL;
            free(pNewFloatData);
            pNewFloatData = NULL;
        }
        fclose(pFile);
    }
    // 关闭目录
    closedir(pDir);

    endTime = clock();                                  // 获取结束时间
    double decompressionTime;                           // 解压缩运行时间
    decompressionTime = (double)(endTime - startTime) / CLOCKS_PER_SEC;
    printf("解压缩运行时间：%fs\n",decompressionTime);
}

void calcDecompressedData(short *pHeader,
                          float *pNewFloatData,
                          unsigned char *pOrigByteData)
{
    float scale;
    scale = (pHeader[2] - pHeader[1])/255.0;            // 获取比例

    // 计算差值并存入文件
    for( int j = 0; j < (int)pHeader[0]; ++j )          // 遍历数组中数据
    {
        pNewFloatData[j] = pOrigByteData[j] * scale + pHeader[1];
    }
}

void calcDiffAndWriteToFile(int filenameNumber,
                            int origLength,
                            short *pArrLength,
                            float *origData,
                            float *newData)
{
    char path[FILENAME_MAX];

    float floatDataDiff;                     // float数据的差值
    int origDataOffset;                      // 原始数据偏移

    sprintf(path,DATA_DIFF_DIR"/"DIFF_VALUE_FILENAME,filenameNumber);
    FILE *pFile = fopen(path, "a+");         // 以追加形式的打开文件，没有文件则创建

    for( int j = 0; j < origLength; ++j )    // 遍历数组中数据
    {
        // 获取当前数据的偏移
        origDataOffset =  getDataOffset(pArrLength,filenameNumber);
        floatDataDiff = origData[origDataOffset+j] - newData[j];      // 获取差值
        fprintf(pFile,"%.2f\n",floatDataDiff);
    }
    // 关闭文件
    fclose(pFile);
}
//<<<----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
