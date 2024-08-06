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
    // av_register_all();
    // avcodec_register_all();
    avformat_network_init();
}

static int shutdown(int status) {
    char errorMessage[50];

    av_strerror(status, errorMessage, sizeof(errorMessage));
    fprintf(stderr, "Shutting down with status %s\n", errorMessage);

    avformat_network_deinit();
    exit(status);
}

char *getTagName(const enum Tags tag) {
  switch (tag)
  {
  case CHECKSUM:
    return "CHECKSUM"; 
    break;
  case UNIX_TIME_STAMP:
    return "UNIX_TIME_STAMP";
    break;
  case MISSION_ID:
    return "MISSION_ID";
    break;
  case PLATFORM_TAIL_NUMBER:
    return "PLATFORM_TAIL_NUMBER";
    break;
  case PLATFORM_HEADING_ANGLE:
    return "PLATFORM_HEADING_ANGLE";
    break;
  case PLATFORM_PITCH_ANGLE:
    return "PLATFORM_PITCH_ANGLE";
    break;
  case PLATFORM_ROLL_ANGLE:
    return "PLATFORM_ROLL_ANGLE";
    break;
  case PLATFORM_TRUE_AIRSPEED:
    return "PLATFORM_TRUE_AIRSPEED";
    break;
  case PLATFORM_INDICATED_AIRSPEED:
    return "PLATFORM_INDICATED_AIRSPEED";
    break;
  case PLATFORM_DESIGNATION:
    return "PLATFORM_DESIGNATION";
    break;
  case IMAGE_SOURCE_SENSOR: 
    return "IMAGE_SOURCE_SENSOR";
    break;
  case IMAGE_COORDINATE_SYSTEM:
    return "IMAGE_COORDINATE_SYSTEM";
    break;
  case SENSOR_LATITUDE:
    return "SENSOR_LATITUDE";
    break;
  case SENSOR_LONGITUDE: 
    return "SENSOR_LONGITUDE";
    break;
  case SENSOR_TRUE_ALTITUDE:
    return "SENSOR_TRUE_ALTITUDE";
    break;
  case SENSOR_HORIZONTAL_FOV:
    return "SENSOR_HORIZONTAL_FOV";
    break;
  case SENSOR_VERTICAL_FOV: 
    return "SENSOR_VERTICAL_FOV";
    break;
  case SENSOR_RELATIVE_AZIMUTH_ANGLE:
   return "SENSOR_RELATIVE_AZIMUTH_ANGLE";
   break;
  case SENSOR_RELATIVE_ELEVATION_ANGLE:
    return "";
    break;
  case  SENSOR_RELATIVE_ROLL_ANGLE:
    return "SENSOR_RELATIVE_ROLL_ANGLE";
    break;
  case  SLANT_RANGE:
    return "SLANT_RANGE";
    break;
  case  TARGET_WIDTH:
    return "TARGET_WIDTH";
    break;
  case  FRAME_CENTER_LATITUDE:
    return "FRAME_CENTER_LATITUDE";
    break;
  case  FRAME_CENTER_LONGITUDE:
    return "FRAME_CENTER_LONGITUDE";
    break;
  case  FRAME_CENTER_ELEVATION:
    return "FRAME_CENTER_ELEVATION";
    break;
  case  OFFSET_CORNER_LATITUDE_POINT_1:
    return "OFFSET_CORNER_LATITUDE_POINT_1";
    break;
  case  OFFSET_CORNER_LONGITUDE_POINT_1:
    return "OFFSET_CORNER_LONGITUDE_POINT_1";
    break;
  case  OFFSET_CORNER_LATITUDE_POINT_2:
    return "OFFSET_CORNER_LATITUDE_POINT_2";
    break;
  case  OFFSET_CORNER_LONGITUDE_POINT_2:
    return "OFFSET_CORNER_LONGITUDE_POINT_2";
    break;
  case  OFFSET_CORNER_LATITUDE_POINT_3:
    return "OFFSET_CORNER_LATITUDE_POINT_3";
    break;
  case  OFFSET_CORNER_LONGITUDE_POINT_3:
    return "OFFSET_CORNER_LONGITUDE_POINT_3";
    break;
  case  OFFSET_CORNER_LATITUDE_POINT_4:
    return "OFFSET_CORNER_LATITUDE_POINT_4";
    break;
  case  OFFSET_CORNER_LONGITUDE_POINT_4:
    return "OFFSET_CORNER_LONGITUDE_POINT_4";
    break;
  case  ICING_DETECTED:
    return "ICING_DETECTED";
    break;
  case  WIND_DIRECTION:
    return "WIND_DIRECTION";
    break;
  case  WIND_SPEED:
    return "WIND_SPEED";
    break;
  case  STATIC_PRESSURE:
    return "STATIC_PRESSURE";
    break;
  case  DENSITY_ALTITUDE:
    return "DENSITY_ALTITUDE";
    break;
  case  OUTSIDE_AIR_TEMPERATURE:
    return "OUTSIDE_AIR_TEMPERATURE";
    break;
  case  TARGET_LOCATION_LATITUDE:
    return "TARGET_LOCATION_LATITUDE";
    break;
  case  TARGET_LOCATION_LONGITUDE:
    return "TARGET_LOCATION_LONGITUDE";
    break;
  case  TARGET_LOCATION_ELEVATION:
    return "TARGET_LOCATION_ELEVATION";
    break;
case  TARGET_TRACK_GATE_WIDTH :
  return "TARGET_TRACK_GATE_WIDTH";
  break;
case  TARGET_TRACK_GATE_HEIGHT :
  return "TARGET_TRACK_GATE_HEIGHT";
  break;
case  TARGET_ERROR_ESTIMATE_CE90 :
  return "TARGET_ERROR_ESTIMATE_CE90";
  break;
case  TARGET_ERROR_ESTIMATE_LE90 :
  return "TARGET_ERROR_ESTIMATE_LE90";
  break;
case  GENERIC_FLAG_DATA1 :
  return "GENERIC_FLAG_DATA1";
  break;
case  SECURITY_LOCAL_METADATA_SET :
  return "SECURITY_LOCAL_METADATA_SET";
  break;
case  DIFFERENTIAL_PRESSURE :
  return "DIFFERENTIAL_PRESSURE";
  break;
case  PLATFORM_ANGLE_OF_ATTACK :
  return "PLATFORM_ANGLE_OF_ATTACK";
  break;
case  PLATFORM_VERTICAL_SPEED :
  return "PLATFORM_VERTICAL_SPEED";
  break;
case  PLATFORM_SIDESLIP_ANGLE :
  return "PLATFORM_SIDESLIP_ANGLE";
  break;
case  AIRFIELD_BAROMETRIC_PRESSURE :
  return "AIRFIELD_BAROMETRIC_PRESSURE";
  break;
case  AIRFIELD_ELEVATION :
  return "AIRFIELD_ELEVATION";
  break;
case  RELATIVE_HUMIDITY :
  return "RELATIVE_HUMIDITY";
  break;
case  PLATFORM_GROUND_SPEED :
  return "PLATFORM_GROUND_SPEED";
  break;
case  GROUND_RANGE :
  return "GROUND_RANGE";
  break;
case  PLATFORM_FUEL_REMAINING :
  return "PLATFORM_FUEL_REMAINING";
  break;
case  PLATFORM_CALL_SIGN :
  return "PLATFORM_CALL_SIGN";
  break;
case  WEAPON_LOAD :
  return "WEAPON_LOAD";
  break;
case  WEAPON_FIRED :
  return "WEAPON_FIRED";
  break;
case  LASER_PRF_CODE :
  return "LASER_PRF_CODE";
  break;
case  SENSOR_FOV_NAME :
  return "SENSOR_FOV_NAME";
  break;
case  PLATFORM_MAGNETIC_HEADING :
  return "PLATFORM_MAGNETIC_HEADING";
  break;
case  UAS_LDS_VERSION_NUMBER :
  return "UAS_LDS_VERSION_NUMBER";
  break;
case  TARGET_LOCATION_COVARIANCE_MATRIX :
  return "TARGET_LOCATION_COVARIANCE_MATRIX";
  break;
case  ALTERNATE_PLATFORM_LATITUDE :
  return "ALTERNATE_PLATFORM_LATITUDE";
  break;
case  ALTERNATE_PLATFORM_LONGITUDE :
  return "ALTERNATE_PLATFORM_LONGITUDE";
  break;
case  ALTERNATE_PLATFORM_ALTITUDE :
  return "ALTERNATE_PLATFORM_ALTITUDE";
  break;
case  ALTERNATE_PLATFORM_NAME :
  return "ALTERNATE_PLATFORM_NAME";
  break;
case  ALTERNATE_PLATFORM_HEADING :
  return "ALTERNATE_PLATFORM_HEADING";
  break;
case  EVENT_START_TIME_UTC :
  return "EVENT_START_TIME_UTC";
  break;
case  RVT_LOCAL_DATA_SET :
  return "RVT_LOCAL_DATA_SET";
  break;
case  VMTI_LOCAL_DATA_SET :
  return "VMTI_LOCAL_DATA_SET";
  break;
case  SENSOR_ELLIPSOID_HEIGHT :
  return "SENSOR_ELLIPSOID_HEIGHT";
  break;
case  ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT :
  return "ALTERNATE_PLATFORM_ELLIPSOID_HEIGHT";
  break;
case  OPERATIONAL_MODE :
  return "OPERATIONAL_MODE";
  break;
case  FRAME_CENTER_HEIGHT_ABOVE_ELLIPSOID :
  return "FRAME_CENTER_HEIGHT_ABOVE_ELLIPSOID";
  break;
case  SENSOR_NORTH_VELOCITY :
  return "SENSOR_NORTH_VELOCITY";
  break;
case  SENSOR_EAST_VELOCITY :
  return "SENSOR_EAST_VELOCITY";
  break;
case  IMAGE_HORIZON_PIXEL_PACK :
  return "IMAGE_HORIZON_PIXEL_PACK";
  break;
case  CORNER_LATITUDE_POINT_1 :
  return "CORNER_LATITUDE_POINT_1";
  break;
case  CORNER_LONGITUDE_POINT_1 :
  return "CORNER_LONGITUDE_POINT_1";
  break;
case  CORNER_LATITUDE_POINT_2 :
  return "CORNER_LATITUDE_POINT_2";
  break;
case  CORNER_LONGITUDE_POINT_2 :
  return "CORNER_LONGITUDE_POINT_2";
  break;
case  CORNER_LATITUDE_POINT_3 :
  return "CORNER_LATITUDE_POINT_3";
  break;
case  CORNER_LONGITUDE_POINT_3 :
  return "CORNER_LONGITUDE_POINT_3";
  break;
case  CORNER_LATITUDE_POINT_4 :
  return "CORNER_LATITUDE_POINT_4";
  break;
case  CORNER_LONGITUDE_POINT_4 :
  return "CORNER_LONGITUDE_POINT_4";
  break;
case  PLATFORM_PITCH_ANGLE_F :
  return "PLATFORM_PITCH_ANGLE_F";
  break;
case  PLATFORM_ROLL_ANGLE_F :
  return "PLATFORM_ROLL_ANGLE_F";
  break;
case  PLATFORM_ANGLE_OF_ATTACK_F :
  return "PLATFORM_ANGLE_OF_ATTACK_F";
  break;
case  PLATFORM_SIDESLIP_ANGLE_F :
  return "PLATFORM_SIDESLIP_ANGLE_F";
  break;
  
  default:
    return "UNKNOWN_FIELD";
    break;
  }
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
    fprintf(stdout, "Name: %s\n", formatContext->url);
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