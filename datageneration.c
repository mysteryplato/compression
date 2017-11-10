/**
 *  @brief 数据生成函数，计算数据偏移量函数，删除文件目录函数
 *
 *  @author plato
 *  @version 1.00 2017-11-08 plato
 *                note:create it
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include "datageneration.h"

//>>>----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
//grace171018 生成数据和计算数据的偏移

float *getpData(short *pArrLength)
{
    float *pFloatData = NULL;      // 存放生成的随机数，返回指针
    int arrLengthSum = 0;          // 数组总长度

    // 1：统计数组长度
    srand(time(NULL));             // 设置随机数种子
    for ( int i = 0; i < ARR_COUNT; ++i )
    {
        pArrLength[i] = RANDOM_ARR_LENGTH;
        arrLengthSum += pArrLength[i];
    }

    // 2：申请数据空间
    pFloatData = (float *)malloc(arrLengthSum * sizeof(float));
    if( pFloatData == NULL )
    {
        printf("内存开辟失败！");
        exit(1);
    }

    // 3：数组数据生成
    arrLengthSum = 0;
    //p 这里arrCountI -> i dataCountI -> j ,其它地方也一样
    //p for循环中的循环变量名简单就好，命名太长反而影响可读性
    for ( int i = 0; i < ARR_COUNT; ++i )
    {
        for( int j = 0; j < pArrLength[i]; ++j )
        {
            // 存储产生的随机数据
            *(pFloatData + j + arrLengthSum) = RANDOM_ARR_DATA;
        }
        arrLengthSum += pArrLength[i];
    }
    return pFloatData;   // 返回存放数据的指针
}

int getDataOffset(short *pArrLength, int arrIndex)
{
    int dataOffset = 0;
    for ( int i = 0; i < arrIndex; ++i )
    {
        dataOffset += pArrLength[i];
    }
    return dataOffset;  // 返回数据的偏移量
}
//<<<----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------
