#include <stdio.h>
#include <string.h>
#include "cp936.h"
#include "UTF-8toGB2312.h"

int GetUtf8ByteNumForWord(u8 firstCh)
{
    u8 temp = 0x80;
    int num = 0;

    while (temp & firstCh)
    {
        num++;
        temp = (temp >> 1);
    }
    //printf("the num is: %d\n", num);
    return num;
}

u16 SearchCodeTable(u16 unicodeKey)
{
    int first = 0;
    int end = CODE_TABLE_SIZE - 1;
    int mid = 0;
    while (first <= end)
    {
        mid = (first + end) / 2;
        if (pgm_read_word(&(code_table[mid][0])) == unicodeKey)
        {
            return pgm_read_word(&(code_table[mid][1]));
        }
        else if (pgm_read_word(&(code_table[mid][0])) > unicodeKey)
        {
            end = mid - 1;
        }
        else
        {
            first = mid + 1;
        }
    }
    return 0;
}

void Utf8ToGb2312(const char *utf8, int len, u16 *gbArray)
{
    int k = 0;

    int byteCount = 0;
    int i = 0;
    int j = 0;
    char temp[500];
    u16 unicodeKey = 0;
    u16 gbKey = 0;

    //循环解析
    while (i < len)
    { //delay(0);

        switch (GetUtf8ByteNumForWord((u8)utf8[i]))
        {
        case 0:
            temp[j] = utf8[i];
            gbArray[k++] = gbKey;
            byteCount = 1;
            break;

        case 2:
            temp[j] = utf8[i];
            temp[j + 1] = utf8[i + 1];
            gbKey = ((utf8[i] & 0x80) << 8) + utf8[i + 1];
            gbArray[k++] = gbKey;
            byteCount = 2;
            break;

        case 3:
            //这里就开始进行UTF8->Unicode
            temp[j + 1] = ((utf8[i] & 0x0F) << 4) | ((utf8[i + 1] >> 2) & 0x0F); //高字节
            temp[j] = ((utf8[i + 1] & 0x03) << 6) + (utf8[i + 2] & 0x3F);        //低字节

            //取得Unicode的值
            memcpy(&unicodeKey, (temp + j), 2);

            //根据这个值查表取得对应的GB2312的值
            gbKey = SearchCodeTable(unicodeKey);
            //printf("gbKey=0x%X\n",gbKey);
            gbArray[k++] = gbKey;
            byteCount = 3;
            break;

        case 4:
            byteCount = 4;
            break;
        case 5:
            byteCount = 5;
            break;
        case 6:
            byteCount = 6;
            break;

        default:

            break;
        }
        i += byteCount;
        if (byteCount == 1)
        {
            j++;
        }
        else
        {
            j += 2;
        }
    }
    gbArray[k] = 0xffff;
    //return gbKey;
}
