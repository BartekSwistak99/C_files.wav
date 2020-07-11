#include <SDL2/SDL.h>

#include "wav_fun.h"

int main(int argc, char **argv)
{
    for (double i = 110; i<120; i+=110)
    {
        struct WAVE_file_t *wave_file = NULL;
        enum errors_t error = generate_WAV_header(&wave_file, 1, 44100, 16);
        if (error)
        {
            fprintf(stderr, "%s", error_handle(error));
            return error;
        }
        if ((error = generate_sound(wave_file, 3, generate_saw, i,50)))
        {
            fprintf(stderr, "%s", error_handle(error));
            return error;
        }
        char filename[50];
        sprintf(filename, "5Saw%dTest.wav", (int)i);
        if ((error = save_WAV(filename, wave_file)))
        {
            fprintf(stderr, "%s", error_handle(error));
            return error;
        }
        destroy_WAV(wave_file);
    }
    return success;
}
