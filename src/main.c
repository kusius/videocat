#include <stdio.h>
#include <libavformat/avformat.h>
#include <libavutil/avutil.h>
#include <libavutil/dict.h>
#include <libavutil/opt.h>
#include <libavcodec/avcodec.h>
#include "utils.h"
#include "thirdparty/libmisb/include/unpack.h"

static void setup() {
    printf("Initializing\n");
    av_register_all();
    avcodec_register_all();
    avformat_network_init();
}

static int shutdown(int status) {
    char errorMessage[50];

    av_strerror(status, errorMessage, sizeof(errorMessage));
    fprintf(stderr, "Shutting down with status %s\n", errorMessage);

    avformat_network_deinit();
    exit(status);
}

void printValue(const struct GenericValue format) {

  switch (format.type)
  {
  case UINT8:
    fprintf(stdout, "%d", format.uint8_value);
    break;
  
  case UINT16:
    fprintf(stdout, "%d", format.uint16_value);
    break;


  case UINT32:
    fprintf(stdout, "%d", format.uint32_value);
    break;
  case UINT64:
    fprintf(stdout, "%llu", format.uint64_value);
    break;


  case INT8:
    fprintf(stdout, "%d", format.int8_value);
    break;
  case INT16:
    fprintf(stdout, "%d", format.int16_value);
    break;
  case INT32:
    fprintf(stdout, "%d", format.int32_value);
    break;
  case INT64:
    fprintf(stdout, "%lld", format.int64_value);

    break;
  case FLOAT:
    fprintf(stdout, "%f", format.float_value);
    break;
  case DOUBLE:
    fprintf(stdout, "%lf", format.double_value);
    break;
  case CHAR_P:
    fprintf(stdout, "%s", format.charp_value);
    break;
  default:
    fprintf(stdout, "unknown");
    break;
  }
}

char *getTypeName(const enum Format format) {
  switch (format)
  {
  case UINT8:
    return "UINT 8";
    break;
  
  case UINT16:
    return "UINT 16";
    break;


  case UINT32:
    return "UINT 32";
    break;
  case UINT64:
    return "UINT 64";
    break;


  case INT8:
    return "INT 8";
    break;
  case INT16:
    return "INT 16";
    break;
  case INT32:
    return "INT 32";
    break;
  case INT64:
    return "INT 64";
    break;
  case FLOAT:
    return "FLOAT";
    break;
  case DOUBLE:
    return "DOUBLE";
    break;
  case CHAR_P:
    return "STRING";
    break;
  default:
    return "UNKNOWN";
    break;
  }
}

static void printMetadata(const AVFormatContext *formatContext) {
    const AVDictionaryEntry *tag = NULL;
    fprintf(stdout, "Metadata(%d):\n", av_dict_count(formatContext->metadata));
        
    while ((tag = av_dict_get(formatContext->metadata, "", tag, AV_DICT_IGNORE_SUFFIX))) {
        fprintf(stdout, "\t %s : %s\n", tag->key, tag->value);
    }
}

static void printInfo(const AVFormatContext *formatContext) {
    fprintf(stdout, "Name: %s\n", formatContext->filename);
    fprintf(stdout, "Duration: %lld\n", formatContext->duration);

    const AVCodecDescriptor *videoDescriptor = avcodec_descriptor_get(formatContext->video_codec_id);
    if(videoDescriptor != NULL) {
        fprintf(stdout, "Codec (video): %s", videoDescriptor->name);
    }

    const AVCodecDescriptor *audioDescriptor = avcodec_descriptor_get(formatContext->audio_codec_id);
    if(audioDescriptor != NULL) {
        fprintf(stdout, "Codec (audio): %s", audioDescriptor->name);
    }
}

static int openCodecContext(int *outStreamIndex, AVCodecContext **outCodecContext, AVFormatContext *formatContext, enum AVMediaType type) {
    int result, streamIndex;
    AVStream *stream;
    const AVCodec *decoder = NULL;
    
    result = av_find_best_stream(formatContext, type, -1, -1, NULL, 0);
    if(result < 0) {
        fprintf(stderr, "Could not find %s stream in input file '%s'\n", av_get_media_type_string(type), formatContext->filename);
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
        fprintf(stderr, "Could not find data stream in input file '%s'\n", formatContext->filename);
    } else {
        fprintf(stdout, "Found data stream at index %d\n", result);
        dataStreamIndex = result;
    }

    // Initialize the map that will hold the KLVs
    struct KLVMap *klvmap = malloc(sizeof(struct KLVMap));


    while(av_read_frame(formatContext, packet) >=0) {
        if (packet->stream_index == dataStreamIndex) {
            fprintf(stdout, "Got data stream packet of size %d\n", packet->size);
            // decode the metadata
            // Trying to unpack the misb, check header to see error code
            result = unpack_misb(packet->data, packet->size, klvmap);
            if (result) {
                fprintf(stderr, "Error unpacking the packet : %d\n", result);
            } else {
                // Iterating over the map to retrieve KLVs
                struct GenericValue latitude = klvmap->KLVs[SENSOR_LATITUDE]->value;
                struct GenericValue longitude = klvmap->KLVs[SENSOR_LONGITUDE]->value;
                struct GenericValue tail = klvmap->KLVs[PLATFORM_TAIL_NUMBER]->value;
                struct GenericValue callsign = klvmap->KLVs[PLATFORM_CALL_SIGN]->value;


                printValue(latitude);
                fprintf(stdout, "\n");
                printValue(longitude);
                fprintf(stdout, "\n");
                printValue(tail);
                fprintf(stdout, "\n");
                printValue(callsign);
                fprintf(stdout, "\n");

                // fprintf(stdout, "Sensor Lat Type: %s\n", getTypeName(latitude.type));
                // fprintf(stdout, "Sensor Lon Type: %s\n", getTypeName(longitude.type));
                // fprintf(stdout, "Tail number Type: %s\n", getTypeName(tail.type));
                // fprintf(stdout, "Callsign Type: %s\n", getTypeName(callsign.type));
                // print all metadata tags and length
                // for (int i = 0; i < 94; i++) {
                //     if (klvmap->KLVs[i])
                //         printf("Tag %d - Size %ld\n", klvmap->KLVs[i]->tag, klvmap->KLVs[i]->size);
                // }
            }
            
        }
    }
    // openCodecContext(&dataStreamIndex, &codecContext, formatContext, AVMEDIA_TYPE_DATA);
    // while(av_read_frame(formatContext, packet) >= 0) {
    //     fprintf(stdout, "Received packet from stream index %d\n", packet->stream_index);
    // }

    free(klvmap);
    avformat_close_input(&formatContext);

    return shutdown(0);
}