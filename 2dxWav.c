#include "sox/sox.h"
#include "2dxWav.h"
#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// the assert in assert.h produces a crash, this just returns
#define assert(cond) if(!(cond)) {printf("Something went wrong: " #cond "\n"); return 1;}

/* Private data for .wav file, taken from wav.c so we can change
   certain attributes to the 2dx specific format */
typedef struct {
    uint64_t  numSamples;
    size_t    dataLength;
    unsigned short formatTag;
    unsigned short samplesPerBlock;
    unsigned short blockAlign;
    size_t dataStart;
    char           * comment;
    int ignoreSize;

    unsigned short nCoefs;          /* ADPCM: number of coef sets */
    short         *lsx_ms_adpcm_i_coefs; /* ADPCM: coef sets           */
    unsigned char *packet;          /* Temporary buffer for packets */
    short         *samples;         /* interleaved samples buffer */
    short         *samplePtr;       /* Pointer to current sample  */
    short         *sampleTop;       /* End of samples-buffer      */
    unsigned short blockSamplesRemaining;/* Samples remaining per channel */
    int            state[16];       /* step-size info for *ADPCM writes */
    
    /* there are more things in here but they're related to GSM encoding */
} priv_t;

/* For debugging */
void printwav(priv_t* wav) {
    printf("\t numSamples: %d\n"              ,  wav->numSamples            );
    printf("\t dataLength: %zu\n"             ,  wav->dataLength            );
    printf("\t formatTag: %hu\n"              ,  wav->formatTag             );
    printf("\t samplesPerBlock: %hu\n"        ,  wav->samplesPerBlock       );
    printf("\t blockAlign: %hu\n"             ,  wav->blockAlign            );
    printf("\t dataStart: %zu\n"              ,  wav->dataStart             );
    printf("\t ignoreSize: %d\n"              ,  wav->ignoreSize            );
    printf("\t nCoefs: %hu\n"                 ,  wav->nCoefs                );
    printf("\t lsx_ms_adpcm_i_coefs: %d\n"    ,  wav->lsx_ms_adpcm_i_coefs  );
    printf("\t packet: %d\n:"                 ,  wav->packet                );
    printf("\t samples: %d\n"                 ,  wav->samples               );
    printf("\t samplePtr: %d\n"               ,  wav->samplePtr             );
    printf("\t sampleTop: %d\n"               ,  wav->sampleTop             );
    printf("\t blockSamplesRemaining: %d\n"   ,  wav->blockSamplesRemaining );
}

int convert_wav(char* inFile, char* outWav, int trimPreview) {
    static sox_format_t *in, *out;
    sox_effects_chain_t *chain;
    sox_effect_t *e;
    sox_signalinfo_t interm_signal;
    char *args[10];
    
    assert(sox_init() == SOX_SUCCESS);
    assert(in = sox_open_read(inFile, NULL, NULL, NULL));

    // 2dx specific format
    sox_signalinfo_t out_signal = {
        .rate = 44100,
        .channels = 2, // have to be 2 for later hacks to work
        .precision = 0,
        .length = 0,
        .mult = NULL
    };
    
    sox_encodinginfo_t out_encoding;
    memset(&out_encoding, 0, sizeof(out_encoding));
    out_encoding.encoding = SOX_ENCODING_MS_ADPCM;
    out_encoding.bits_per_sample = 4;
    
    assert(out = sox_open_write(outWav, &out_signal, &out_encoding, "wav", NULL, NULL));
    // Forcibly set the values we want
    priv_t *wav = (priv_t *) out->priv;
    wav->samplesPerBlock = 244;
    wav->blockAlign = 256;
    // taken from wav.c, with locked 2 channels
    // The buffers become the wrong size, but we use smaller buffers so it's ok
    size_t sbsize = 2 * wav->samplesPerBlock;
    wav->sampleTop = wav->samples + sbsize;

    // we'll need these later to fix the headers
    uint16_t blockAlign = wav->blockAlign;
    uint16_t samplesPerBlock = wav->samplesPerBlock;
    uint32_t avgBytes = (double)blockAlign*out_signal.rate / (double)samplesPerBlock + 0.5;

    // Debugging
    /*printf("\n\nOutput data:\n");
    printwav(wav);

    wav = (priv_t *) in->priv;
    printf("\n\nInput data:\n");
    printwav(wav);*/

    chain = sox_create_effects_chain(&in->encoding, &out->encoding);
    interm_signal = in->signal; /* NB: deep copy */
    e = sox_create_effect(sox_find_effect("input"));
    args[0] = (char *)in;
    assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &in->signal, &in->signal) == SOX_SUCCESS);
    free(e);

    if (in->signal.rate != out->signal.rate) {
        e = sox_create_effect(sox_find_effect("rate"));
        assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
        assert(sox_add_effect(chain, e, &interm_signal, &out->signal) == SOX_SUCCESS);
        free(e);
    }
    
    // Only use 10 seconds of audio for previews
    if(trimPreview) {
        e = sox_create_effect(sox_find_effect("trim"));
        args[0] = "0";
        args[1] = "10";
        assert(sox_effect_options(e, 2, args) == SOX_SUCCESS);
        assert(sox_add_effect(chain, e, &interm_signal, &in->signal) == SOX_SUCCESS);
        free(e);
    }

    if (in->signal.channels != out->signal.channels) {
        e = sox_create_effect(sox_find_effect("channels"));
        assert(sox_effect_options(e, 0, NULL) == SOX_SUCCESS);
        assert(sox_add_effect(chain, e, &interm_signal, &out->signal) == SOX_SUCCESS);
        free(e);
    }

    e = sox_create_effect(sox_find_effect("output"));
    args[0] = (char *)out;
    assert(sox_effect_options(e, 1, args) == SOX_SUCCESS);
    assert(sox_add_effect(chain, e, &in->signal, &in->signal) == SOX_SUCCESS);
    free(e);

    /* Flow samples through the effects processing chain until EOF is reached */
    sox_flow_effects(chain, NULL, NULL);
    /* All done; tidy up: */
    sox_delete_effects_chain(chain);
    sox_close(out);
    sox_close(in);
    sox_quit();

    // Fix the header, since sox wrote it before we changed our block stuff
    FILE* outFile = fopen(outWav, "r+b");
    // AvgBytesPerSec
    fseek(outFile, 28, SEEK_SET);
    fwrite(&avgBytes, sizeof(uint32_t), 1, outFile);
    fwrite(&blockAlign, sizeof(uint16_t), 1, outFile);
    // samples per block
    fseek(outFile, 38, SEEK_SET);
    fwrite(&samplesPerBlock, sizeof(uint16_t), 1, outFile);

    // RIFF size
    // minus 4 byte "RIFF" and 4 bytes for this number
    size_t wavSize = file_size(outWav) - 8;
    fseek(outFile, 4, SEEK_SET);
    fwrite(&wavSize, sizeof(uint32_t), 1, outFile);

    //// data size
    // minus the RIFF header size
    wavSize -= 82;
    fseek(outFile, 86, SEEK_SET);
    fwrite(&wavSize, sizeof(uint32_t), 1, outFile);

    fclose(outFile);
    
    return 0;
}