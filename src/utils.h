#ifndef UTILS_H
#define UTILS_H

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

#endif // UTILS_H