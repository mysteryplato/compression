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

// 删除文件目录
int removeDir(const char *dir)
{
    char currentDir[] = ".";
    char upDir[] = "..";
    char dirName[FILENAME_MAX];
    DIR *pDir;
    struct dirent *pEntry;
    struct stat dirStat;

    // 如果文件存在
    if ( 0 != access(dir, F_OK) )
    {
        return 0;
    }
    // 如果获取信息失败
    if ( 0 > stat(dir, &dirStat) )
    {
        perror("get directory stat error");
        return -1;
    }
    // 如果是一般文件，直接删除
    if ( S_ISREG(dirStat.st_mode) )
    {
        remove(dir);
    }
    // 如果是目录文件，递归删除目录中内容
    else if ( S_ISDIR(dirStat.st_mode) )
    {
        // 打开目录
        pDir = opendir(dir);
        while ( (pEntry=readdir(pDir)) != NULL )
        {
            // 忽略"."和".."隐藏目录
            if ( (0 == strcmp(currentDir, pEntry->d_name)) || (0 == strcmp(upDir, pEntry->d_name)) )
            {
                continue;
            }
            sprintf(dirName, "%s/%s", dir, pEntry->d_name);
            removeDir(dirName);   // 递归调用
        }
        // 关闭目录
        closedir(pDir);
        rmdir(dir);               // 删除空目录
    }
    else
    {
        perror("unknow file type!");
    }
    return 0;
}
