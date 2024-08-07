#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include "utils.h"
#include "thirdparty/libmisb/include/unpack.h"
#include "ui.h"

static void setup() {
    printf("Initializing\n");
    avformat_network_init();
}

static int shutdown(int status) {
    char errorMessage[50];

    av_strerror(status, errorMessage, sizeof(errorMessage));
    fprintf(stderr, "Shutting down with status %s\n", errorMessage);

    avformat_network_deinit();
    exit(status);
}

static void printMetadata(const AVFormatContext *formatContext) {
    const AVDictionaryEntry *tag = NULL;
    fprintf(stdout, "Metadata(%d):\n", av_dict_count(formatContext->metadata));
        
    while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        fprintf(stdout, "\t %s : %s\n", tag->key, tag->value);
    }
}

static int openCodecContext(int *outStreamIndex, AVCodecContext **outCodecContext, AVFormatContext *formatContext, enum AVMediaType type) {
    int result, streamIndex;
    AVStream *stream;
    const AVCodec *decoder = NULL;

    result = av_find_best_stream(formatContext, type, -1, -1, NULL, 0);
    if(result < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n", av_get_media_type_string(type), formatContext->url);
    } else {
        streamIndex = result;
        stream = formatContext->streams[streamIndex];

        // find the decoder for the stream
        decoder = avcodec_find_decoder(stream->codecpar->codec_id);
        if(!decoder) {
            fprintf(stderr, "Failed to find %s codec\n", av_get_media_type_string(type));
            return AVERROR(EINVAL);
        }

        // Allocate a codec context for the decoder
        *outCodecContext = avcodec_alloc_context3(decoder);
        if(!*outCodecContext) {
            fprintf(stderr, "Failed to allocate the %s codec context\n", av_get_media_type_string(type));
            return AVERROR(ENOMEM);
        }

        // Copy codec parameters from input stream to output codec context
        if((result = avcodec_parameters_to_context(*outCodecContext, stream->codecpar)< 0)) {
            fprintf(stderr, "Failed to open %s codec\n", av_get_media_type_string(type));
            return result;
        }
        *outStreamIndex = streamIndex;
    }

    return 0;
}

int main(int argc, char** argv) {
    setup();

    if (argc < 2) {
        fprintf(stderr, "Usage %s url/file\n", argv[0]);
        shutdown(-1);
    }

    ApplicationState applicationState = createApplication("VideoCat", 800, 600);

    runApplication(&applicationState);

    char fileName[500];
    strcpy(fileName, argv[1]);
    AVFormatContext *formatContext = NULL;
    static AVFrame *frame = NULL;
    static AVPacket *packet = NULL;
    static AVCodecContext *codecContext = NULL;
    static int dataStreamIndex = -1;
    int result;
    char errorMessage[50];

    //  /* open input file, and allocate format context */
    result = avformat_open_input(&formatContext, fileName, NULL, NULL);
    if (result) {
        av_strerror(result, errorMessage, sizeof(errorMessage));
        fprintf(stderr, "Could not open source file %s: %s\n", fileName, errorMessage);
        return shutdown(1);
    }

    /* retrieve stream information */
    result = avformat_find_stream_info(formatContext, NULL);
    if (result) {
        av_strerror(result, errorMessage, sizeof(errorMessage));
        fprintf(stderr, "Could not find stream information: %s\n", errorMessage);
        shutdown(1);
    }

    av_dump_format(formatContext, 0, fileName, 0);
    printMetadata(formatContext);

     frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate frame\n");
        result = AVERROR(ENOMEM);
        return shutdown(result);
    }

    packet = av_packet_alloc();
    if (!packet) {
        fprintf(stderr, "Could not allocate packet\n");
        result = AVERROR(ENOMEM);
        return shutdown(result);
    }

    result = av_find_best_stream(formatContext, AVMEDIA_TYPE_DATA, -1, -1, NULL, 0);
    if(result < 0) {
        fprintf(stderr, "Could not find data stream in input file '%s'\n", formatContext->url);
    } else {
        fprintf(stdout, "Found data stream at index %d\n", result);
        dataStreamIndex = result;
    }

    // Initialize the map that will hold the KLVs
    struct KLVMap klvmap;
    for (int i = 0; i < 94 ; i++) {
       klvmap.KLVs[i] = NULL;
    }

    while(av_read_frame(formatContext, packet) >=0) {
        if (packet->stream_index == dataStreamIndex) {
            fprintf(stdout, "************************************\n");
            fprintf(stdout, "Got data stream packet of size %d\n", packet->size);
            // decode the metadata
            // Trying to unpack the misb, check header to see error code
            result = unpack_misb(packet->data, packet->size, &klvmap);
            if (result) {
                fprintf(stderr, "Error unpacking the packet : %d\n", result);
            } else {
                // Iterating over the map to retrieve KLVs
                for (int i = 0; i < 94; i++) {
                  const struct KLV *klv = klvmap.KLVs[i];
                    if (klv) {
                      const char *tag = getTagName(klv->tag);
                      fprintf(stdout, "%s (%ld bytes): ", tag, klv->size);
                      printValue(klv->value);
                      fprintf(stdout, "\n");
                    }
                }
            }
            
        }
    }

    // freeMemory(klvmap);
    avformat_close_input(&formatContext);

    return shutdown(0);
}