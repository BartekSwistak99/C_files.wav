#include "wav_fun.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>
enum errors_t load_WAV(const char *filename, struct WAVE_file_t **wav_file_ptr)
{
    enum errors_t error = success;
    if (!filename || !wav_file_ptr)
        return incorrect_args;
    FILE *file = fopen(filename, "rb");
    if (!file)
        return file_not_found;

    if (!(*wav_file_ptr = (struct WAVE_file_t *)malloc(sizeof(struct WAVE_file_t))))
    {
        fclose(file);
        return allocation_error;
    }
    struct WAVE_file_t *wav_file = *wav_file_ptr;

    //loading riff header from .wav file
    if ((error = load_riff(&(wav_file->RIFF_header), file)))
    {
        fclose(file);
        free(wav_file);
        return error;
    }
    //loading fmt header from .wav file
    if ((error = load_fmt(&(wav_file->WAVE_fmt), file)))
    {
        fclose(file);
        free(wav_file);
        return error;
    }
    //reading WAVE data header
    if ((error = load_data(&(wav_file->WAVE_data), file)))
    {
        fclose(file);
        free(wav_file);
        return error;
    }

    fclose(file);
    return success;
}
void destroy_WAV(struct WAVE_file_t *wav_file)
{

    free(wav_file->WAVE_data.audioData);
    free(wav_file);
}
const char *error_handle(enum errors_t errorNumber)
{
    switch (errorNumber)
    {
    case success:
        return "Success\n";
    case incorrect_args:
        return "Incorrect arguments passed to function!\n";
    case file_not_found:
        return "Incorrect filename. File not found!\n";
    case file_corrupted:
        return "File corrupted. Can not read file! \n";
    case incorrect_file_type:
        return "Incorrect file type. File contains incorrect data! \n";
    case allocation_error:
        return "Allocation error. Couldn't alocate memory! \n";
    case file_already_exists:
        return "File already exists. Rename your file!\n";
    case file_write_error:
        return "File write error. Could not write data to file!\n";
    default:
        return "Unrecognized error number\n ";
    }
    return "";
}
uint32_t littleEndian32Convert(uint32_t val)
{
    union uint32_convert_t temp = {.value = val};
    uint8_t tempByte = temp.byte[0];
    temp.byte[0] = temp.byte[3];
    temp.byte[3] = tempByte;
    tempByte = temp.byte[1];
    temp.byte[1] = temp.byte[2];
    temp.byte[2] = tempByte;

    return temp.value;
}
uint16_t littleEndian16Convert(uint16_t val)
{
    union uint16_convert_t temp = {.value = val};
    uint8_t tempByte = temp.byte[0];
    temp.byte[0] = temp.byte[1];
    temp.byte[1] = tempByte;
    return temp.value;
}
enum errors_t load_riff(struct RIFF_header_t *riff, FILE *file)
{
    if (!riff || !file)
        return incorrect_args;
    //reading RIFF header
    if (!fread(riff, sizeof(struct RIFF_header_t), 1, file))
    {
        return file_corrupted;
    }
    //converting loaded data into little endian
    riff->chunkID = littleEndian32Convert(riff->chunkID);
    riff->format = littleEndian32Convert(riff->format);
    //validating file type
    if (riff->chunkID != RIFF_ID || riff->format != WAVE_ID)
    {
        return incorrect_file_type;
    }

    return success;
}
enum errors_t load_fmt(struct WAVE_fmt_t *fmt, FILE *file)
{
    if (!fmt || !file)
        return incorrect_args;
    //loading fmt header from .wav file

    //reading WAVE fmt header
    if (!fread(fmt, sizeof(struct WAVE_fmt_t), 1, file))
    {
        return file_corrupted;
    }
    //convert fmt header from big_endian to little_endian
    fmt->chunkID = littleEndian32Convert(fmt->chunkID);
    //validating WAVE fmt header
    if (fmt->chunkID != FMT_ID || fmt->chunkSize != 16 || fmt->audioFormat != 1)
    {
        printf("FMT");
        // printf(" 0x%x 0x%x 0x%x", fmt->chunkID, fmt->chunkSize, fmt->audioFormat);
        return incorrect_file_type;
    }
    return success;
}
enum errors_t load_data(struct WAVE_data_t *waveData, FILE *file)
{
    if (!waveData || !file)
        return incorrect_args;
    //reading WAVE data header
    if (!fread(waveData, sizeof(uint32_t), 2, file))
    {
        return file_corrupted;
    }
    //convert data from big endian to little endian
    waveData->chunkID = littleEndian32Convert(waveData->chunkID);
    //validating WAVE data header
    if (waveData->chunkID != WAVE_DATA_ID || waveData->chunkSize == 0)
    {
        return incorrect_file_type;
    }
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * waveData->chunkSize);
    if (!data)
    {
        return allocation_error;
    }
    waveData->audioData = data;
    if (!fread(waveData->audioData, sizeof(uint8_t), waveData->chunkSize, file))
    {
        free(data);
        printf("DATA");

        return file_corrupted;
    }

    return success;
}
enum errors_t save_WAV(const char *filename, struct WAVE_file_t *wav_file)
{
    if (!filename || !wav_file)
    {
        return incorrect_args;
    }
    //check if file already exists
    FILE *f = fopen(filename, "rb");
    if (f)
    {
        fclose(f);
        return file_already_exists;
    }
    FILE *file = fopen(filename, "wb");
    //writing RIFF header
    uint32_t convert_temp = littleEndian32Convert(wav_file->RIFF_header.chunkID);
    if (!fwrite(&convert_temp, sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    if (!fwrite(&(wav_file->RIFF_header.chunkSize), sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    convert_temp = littleEndian32Convert(wav_file->RIFF_header.format);
    if (!fwrite(&convert_temp, sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    //writing fmt header
    convert_temp = littleEndian32Convert(wav_file->WAVE_fmt.chunkID);
    if (!fwrite(&convert_temp, sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    if (!fwrite((void *)((intptr_t) & (wav_file->WAVE_fmt) + sizeof(uint32_t)), sizeof(struct WAVE_fmt_t) - sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    //writing wave data
    convert_temp = littleEndian32Convert(wav_file->WAVE_data.chunkID);
    if (!fwrite(&convert_temp, sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }

    if (!fwrite(&(wav_file->WAVE_data.chunkSize), sizeof(uint32_t), 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    if (!fwrite(wav_file->WAVE_data.audioData, wav_file->WAVE_data.chunkSize, 1, file))
    {
        fclose(file);
        return file_write_error;
    }
    fclose(file);
    return success;
}
void print_WAV_info(struct WAVE_file_t *wav_file)
{
    if (!wav_file)
        return;
    printf("WAVE file info:\n");
    printf("\t[RIFF header]\n");
    printf("### ChunkID: 0x%x\n", wav_file->RIFF_header.chunkID);
    printf("### ChunkSize: %d\n", wav_file->RIFF_header.chunkSize);
    printf("### Format: 0x%x\n", wav_file->RIFF_header.format);
    printf("\t[fmt header]\n");
    printf("### ChunkID: 0x%x\n", wav_file->WAVE_fmt.chunkID);
    printf("### ChunkSize: %d\n", wav_file->WAVE_fmt.chunkSize);
    printf("### AudioFormat: %d\n", wav_file->WAVE_fmt.audioFormat);
    printf("### Chunnels: %d\n", wav_file->WAVE_fmt.channels);
    printf("### SampleRate: %d\n", wav_file->WAVE_fmt.sampleRate);
    printf("### ByteRate: %d\n", wav_file->WAVE_fmt.byteRate);
    printf("### BlockAlign: %d\n", wav_file->WAVE_fmt.blockAlign);
    printf("### BitsPerSample: %d\n", wav_file->WAVE_fmt.bitsPerSample);
    printf("\tTotal sound data size: %d\n\n", wav_file->WAVE_data.chunkSize);
}
enum errors_t generate_WAV_header(struct WAVE_file_t **wav_file_ptr, uint16_t channels, uint32_t SampleRate, uint16_t BitsPerSample)
{

    if (!wav_file_ptr)
        return incorrect_args;

    if (!(*wav_file_ptr = (struct WAVE_file_t *)malloc(sizeof(struct WAVE_file_t))))
    {
        return allocation_error;
    }
    struct WAVE_file_t *wav_file = *wav_file_ptr;
    //RIFF header
    wav_file->RIFF_header.chunkID = RIFF_ID;
    wav_file->RIFF_header.chunkSize = 36;
    wav_file->RIFF_header.format = WAVE_ID;
    //FMT header
    wav_file->WAVE_fmt.chunkID = FMT_ID;
    wav_file->WAVE_fmt.chunkSize = 16;
    wav_file->WAVE_fmt.audioFormat = 1;
    wav_file->WAVE_fmt.channels = channels;
    wav_file->WAVE_fmt.sampleRate = SampleRate;
    wav_file->WAVE_fmt.bitsPerSample = BitsPerSample;
    wav_file->WAVE_fmt.byteRate = SampleRate * channels * BitsPerSample / 8;
    wav_file->WAVE_fmt.blockAlign = channels * BitsPerSample / 8;
    //wave data header
    wav_file->WAVE_data.chunkID = WAVE_DATA_ID;
    wav_file->WAVE_data.chunkSize = 0;
    return success;
}
enum errors_t WAV_add_data(struct WAVE_file_t *wav_file, uint8_t *data, int buffer_size)
{
    if (!wav_file || !data || buffer_size <= 0)
    {
        return incorrect_args;
    }
    wav_file->WAVE_data.chunkSize = buffer_size;
    wav_file->RIFF_header.chunkSize += buffer_size;
    wav_file->WAVE_data.audioData = data;
    return success;
}
enum errors_t generate_sound(struct WAVE_file_t *wav_file, int time, int16_t (*generate_signal)(double, double), double frequency)
{
    if (!wav_file || time <= 0 || !generate_signal)
        return incorrect_args;
    uint8_t *data = (uint8_t *)malloc(sizeof(uint8_t) * time * wav_file->WAVE_fmt.sampleRate * wav_file->WAVE_fmt.blockAlign);
    if (!data)
        return allocation_error;
    int16_t *temp_array = (int16_t *)data;
    double timeStep = 0.0;
    for (int i = 0; i < time * wav_file->WAVE_fmt.sampleRate; i++)
    {
        double temp = generate_signal(frequency, timeStep);
        *(temp_array + i) = temp;
        timeStep += 1.0 / (wav_file->WAVE_fmt.sampleRate);
    }
    enum errors_t error = WAV_add_data(wav_file, data, time * wav_file->WAVE_fmt.sampleRate * wav_file->WAVE_fmt.blockAlign);
    if (error)
        return error;
    return success;
}
int16_t generate_sinus(double frequency, double time)
{
    double result = sin(time * frequency * 2 * 3.1415);
    return result * 1000;
}
int16_t generate_white_noise(double frequency, double time)
{
    srand(clock());
    double result = rand() % 1000;
    return result;
}
int16_t generate_tan(double frequency, double time)
{
    double result = tan(time * frequency * 2 * 3.1415);
    return result * 100;
}
int16_t generate_sinus2(double frequency, double time)
{
    double result = sin(time * frequency * 2 * 3.1415) + tan(time * 3 * frequency * 2 * 3.1415);
    return result * 100;
}
double statFun(void)
{
    static int counter = 0;
    if (counter < 22050)
        counter++;
    else if (counter > 22050)
        counter--;
}