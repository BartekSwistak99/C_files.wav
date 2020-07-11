#ifndef __WAV_FUN_H__
#define __WAV_FUN_H__
/*
useful doc:
http://soundfile.sapp.org/doc/WaveFormat/
*/
#include <stdint.h>
#include <stdio.h>

enum errors_t
{
    success,
    incorrect_args,
    file_not_found,
    file_corrupted,
    incorrect_file_type,
    allocation_error,
    file_already_exists,
    file_write_error
};
union uint32_convert_t
{
    uint32_t value;
    uint8_t byte[4];
};
union uint16_convert_t
{
    uint16_t value;
    uint8_t byte[2];
};
#define RIFF_ID 0x52494646      //"RIFF" in big endian
#define WAVE_ID 0x57415645      //"WAVE" in big endian
#define FMT_ID 0x666d7420       //"FMT " in big endian
#define WAVE_DATA_ID 0x64617461 //"data" in big endian
struct RIFF_header_t
{
    uint32_t chunkID;   //in riff files its always "RIFF" or 0x52494646 in hex
    uint32_t chunkSize; //size of file -8 bytes (rest of data)
    uint32_t format;    //format of riff file- in .wav its always "WAVE" id or 0x57415645 in hex
};
struct WAVE_fmt_t
{                           //header of WAVE chunk, contains sound data's format
    uint32_t chunkID;       //id of fmt header:   "fmt " or 0x666d7420 in hex
    uint32_t chunkSize;     //size of the rest of this chunk. For wav file- 16 (10 00 00 00)
    uint16_t audioFormat;   //for PCM = 1 Other values indicates compression
    uint16_t channels;      //number of channels
    uint32_t sampleRate;    //number of samples per sec
    uint32_t byteRate;      //bytes per sec == SampleRate*channels*bitsPerSample/8
    uint16_t blockAlign;    //number of bytes for one sample including all channels ==channels*bitsPerSample/8
    uint16_t bitsPerSample; //number of bits in one sample;
};
struct WAVE_data_t
{
    uint32_t chunkID;   //id of data chunk. == "data" or 0x64617461 in hex
    uint32_t chunkSize; //number of bytes in the data. ==NumSamples*channels*bitsPerSample/8
    uint8_t *audioData; //array containing data of audio file. ArraySize == chunkSize
};
struct WAVE_file_t //structure contains wholse .WAV file data
{
    struct RIFF_header_t RIFF_header;
    struct WAVE_fmt_t WAVE_fmt;
    struct WAVE_data_t WAVE_data;
};
enum errors_t load_WAV(const char *, struct WAVE_file_t **);
void print_WAV_info(struct WAVE_file_t *);
void destroy_WAV(struct WAVE_file_t *);
const char *error_handle(enum errors_t);
uint32_t littleEndian32Convert(uint32_t);
uint16_t littleEndian16Convert(uint16_t);
enum errors_t load_riff(struct RIFF_header_t *, FILE *);
enum errors_t load_fmt(struct WAVE_fmt_t *, FILE *);
enum errors_t load_data(struct WAVE_data_t *, FILE *);
enum errors_t save_WAV(const char *, struct WAVE_file_t *);
enum errors_t generate_WAV_header(struct WAVE_file_t **, uint16_t, uint32_t, uint16_t);
enum errors_t WAV_add_data(struct WAVE_file_t *, uint8_t *, int);
enum errors_t generate_sound(struct WAVE_file_t *, int, int16_t (*)(double,double), double frequency);
int16_t generate_sinus(double,double);
int16_t generate_sinus2(double,double);
int16_t generate_white_noise(double,double);
int16_t generate_tan(double,double);
#endif