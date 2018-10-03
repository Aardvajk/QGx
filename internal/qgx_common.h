#ifndef QGX_COMMON_H
#define QGX_COMMON_H

template<typename T> void qgx_detail_com_ptr_release(T *&ptr)
{
    if(ptr)
    {
        ptr->Release();
        ptr = nullptr;
    }
}

#endif // QGX_COMMON_H
