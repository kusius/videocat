#ifndef MYUTILS_H
#define MYUTILS_H

#include <libavutil/avutil.h>

const char *av_get_media_type_string(enum AVMediaType media_type) {
    switch (media_type)
    {
    case AVMEDIA_TYPE_VIDEO:
        return "video";
    case AVMEDIA_TYPE_AUDIO:
        return "audio";
    case AVMEDIA_TYPE_DATA:
        return "data";
    case AVMEDIA_TYPE_SUBTITLE:
        return "subtitle";
    case AVMEDIA_TYPE_ATTACHMENT:
        return "attachment";
    default:
        return NULL;
    }
}

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)

    void *allocateMemory(size_t bytes) {
        return CoTaskMemAlloc(bytes);
    }

    void freeMemory(void *address) {
        CoTaskMemFree(address);
    }
#else
    void *allocateMemory(size_t bytes) {
        return malloc(bytes);
    }

    void freeMemory(void *address) {
        free(address);
    }
#endif


#endif // MYUTILS_H