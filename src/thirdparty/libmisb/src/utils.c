#include "../include/utils.h"

uint64_t get_timestamp();

#if defined(WIN32) || defined(_WIN32) || defined(__WIN32__) || defined(__NT__)
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64 

// MSVC defines this in winsock2.h!?
typedef struct timeval {
    long tv_sec;
    long tv_usec;
} timeval;

int gettimeofday(struct timeval * tp, struct timezone * tzp)
{
    // Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
    // This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
    // until 00:00:00 January 1, 1970 
    static const uint64_t EPOCH = ((uint64_t) 116444736000000000ULL);

    SYSTEMTIME  system_time;
    FILETIME    file_time;
    uint64_t    time;

    GetSystemTime( &system_time );
    SystemTimeToFileTime( &system_time, &file_time );
    time =  ((uint64_t)file_time.dwLowDateTime )      ;
    time += ((uint64_t)file_time.dwHighDateTime) << 32;

    tp->tv_sec  = (long) ((time - EPOCH) / 10000000L);
    tp->tv_usec = (long) (system_time.wMilliseconds * 1000);
    return 0;
}


uint64_t get_timestamp()
{
    struct timeval tv;
    gettimeofday(&tv,NULL);
    return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
}


#else 
  #include <sys/time.h>
  uint64_t get_timestamp()
  {
      struct timeval tv;
      gettimeofday(&tv,NULL);
      return tv.tv_sec * (uint64_t)1000000 + tv.tv_usec;
  }
#endif

#include <math.h>
#include <stddef.h>

// INT16

unsigned short signed_dec_to_int16(float value, float range)
{
  char negative = 0;
  unsigned short result = 0;

  // pack using two's complement if needed
  if (value < 0)
  {
    value = -value;
    negative = 1;
  }

  // range of an uint16 is 65534
  result = (unsigned short) round((value * 65534) / range);

  if (negative)
    result = (~result) + 1;

  return result;
}

float int16_to_signed_dec(unsigned short value, float range)
{
  char negative = 0;
  float result = 0;

  // unpack two's complement value
  if ((value >> 15) & 1)
  {
    negative = 1;
    value = (~value) + 1;
  }

  // range of an uint16 is 65534
  result = ((float)range / 65534.0) * (float)value;
  if (negative)
    result *= -1;

  return result;
}

unsigned short unsigned_dec_to_int16(float value, float range, float offset)
{
  unsigned short result = 0;
  result = (unsigned short) round(((value + offset)* 65534) / range);
  return result;
}


float int16_to_unsigned_dec(unsigned short value, float range, float offset)
{
  float result = 0;
  result = ((float)range / 65534.0) * (float) value;
  return result - offset;
}

// INT32

int signed_dec_to_int32(double value, float range)
{
  char negative = 0;
  int result = 0;

  // pack using two's complement if needed
  if (value < 0)
  {
    value = -value;
    negative = 1;
  }

  // range of an uint32 is 4294967294
  result = round((value * 4294967294.0) / range);

  if (negative)
    result = (~result) + 1;

  return result;
}

double int32_to_signed_dec(int value, float range)
{
  char negative = 0;
  double result = 0;

  // unpack two's complement value
  if ((value >> 31) & 1)
  {
    negative = 1;
    value = (~value) + 1;
  }

  // range of an uint16 is 4294967294
  result = ((double)range / 4294967294.0) * (double)value;
  if (negative)
    result *= -1;

  return result;
}

int unsigned_dec_to_int32(double value, float range, double offset)
{
  int result = 0;
  result = round(((value + offset)* 4294967294.0) / range);
  return result;
}

double int32_to_unsigned_dec(int value, float range, double offset)
{
  double result = 0;
  result = ((double)range / 4294967294.0) * (double)value;
  return result - offset;
}


// `buff` is a pointer to the first byte in the 16-byte UAS LDS key.
// `len` is the length from 16-byte UDS key up to 1-byte checksum length.
unsigned short bcc_16(uint8_t *buff, unsigned short len)
{
  // Initialize Checksum and counter variables.
  unsigned short bcc = 0, i;

  // Sum each 16-bit chunk whitin the buffer into a checksum
  for (i = 0; i < len; i++)
    bcc += buff[i] << (8 * ((i + 1) % 2));
  return bcc;
} // end of bcc_16()


