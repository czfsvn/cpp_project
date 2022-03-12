#ifndef __MEMORY_HELPER_H__
#define __MEMORY_HELPER_H__

#include <assert.h>
#include <iostream>

#define INIT_PAGE_SIZE 100
#define MAX_BYTES      2048

namespace cncpp
{
    inline static uint16_t _Index(uint16_t size, uint16_t align)
    {
        uint16_t alignnum = 1 << align;  //����ʵ�ֵķ���
        return ((size + alignnum - 1) >> align) - 1;
    }

    inline static uint16_t _Roundup(uint16_t size, uint16_t align)
    {
        uint16_t alignnum = 1 << align;
        return (size + alignnum - 1) & ~(alignnum - 1);
    }

    // ������12%���ҵ�����Ƭ�˷�
    // [1,128]				8byte���� freelist[0,16)
    // [129,1024]			16byte���� freelist[16,72)
    // [1025,8*1024]		128byte���� freelist[72,128)
    // [8*1024+1,64*1024]	1024byte���� freelist[128,184)

    inline static uint16_t Index(uint16_t size)
    {
        assert(size <= MAX_BYTES);

        // ÿ�������ж��ٸ���
        static int group_array[4] = { 16, 56, 56, 56 };
        if (size <= 128)
        {
            return _Index(size, 3);
        }
        else if (size <= 1024)
        {
            return _Index(size - 128, 4) + group_array[0];
        }
        else if (size <= 8192)
        {
            return _Index(size - 1024, 7) + group_array[0] + group_array[1];
        }
        else  // if (size <= 65536)
        {
            return _Index(size - 8 * 1024, 10) + group_array[0] + group_array[1] + group_array[2];
        }
    }

    // �����С���㣬����ȡ��
    static inline uint16_t Roundup(uint16_t bytes)
    {
        assert(bytes <= MAX_BYTES);

        if (bytes <= 128)
        {
            return _Roundup(bytes, 3);
        }
        else if (bytes <= 2048)
        {
            return _Roundup(bytes, 4);
        }
        else if (bytes <= 8192)
        {
            return _Roundup(bytes, 7);
        }
        else
        {  // if (bytes <= 65536){
            return _Roundup(bytes, 10);
        }
    }

}  // namespace cncpp
#endif