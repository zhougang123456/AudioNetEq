#include <stdio.h>
#include "api/audio-neteq.h"

int main() 
{
    const int kSampleRateHz = 48000;
    const int kPlayloadType = 96;
    FILE* pcm = fopen("48.pcm", "rb");
    if (pcm == NULL) {
        printf("open pcm file failed!\n");
        return -1;
    }
    FILE* outfile = fopen("plc.pcm", "wb");
    NeteqContext* context = init_neteq(kSampleRateHz, 1, kPlayloadType);
    if (context == NULL) {
        printf("init neteq failed!\n");
        return -1;
    }
    
    neteq_set_max_delay(context, 100);
    neteq_set_max_speed(context, 1.5);
    int timestamp = 0;
    short seq_no = 0;
    const int kFframSizeMs = 10;
    const int kSamples = 480;

    short inputSample[480] = { 0 };
    short inputSample2[480] = { 0 };

    short outputSample[480 * 2] = { 0 };
    int i = 0;
    while (1) {
        
        while (!feof(pcm)) {
            int read = fread(inputSample, sizeof(short), kSamples, pcm);
            //read = fread(inputSample2, sizeof(short), kSamples, pcm);
            if (read != kSamples) {
                //printf("read: %d\n", read);
                break;
            }
            //if (seq_no % 4 != 1) { // test expand and merge
            //    if (neteq_insert_packet(seq_no, timestamp, inputSample, kSamples) == -1) {
            //        printf("neteq insert audio failed!\n");
            //        return -1;
            //    }
            //}
            /*if (neteq_insert_packet(seq_no + 1, timestamp + kSamples, inputSample2, kSamples) == -1) {
                printf("neteq insert audio failed!\n");
                return -1;
            }*/
            if (neteq_insert_packet(context, seq_no, timestamp, inputSample, kSamples) == -1) {
                //printf("neteq insert audio failed!\n");
                return -1;
            }

            seq_no += 1;
            timestamp += 1 * kSamples;
            //+seq_no;
            //timestamp += kSamples;
            printf("neteq delay %d\n", neteq_get_delay_time(context, timestamp));
            if (seq_no % 4 != 0) {
                //printf("neteq operation %d!\n", neteq_get_last_operator());
                //if (seq_no % 5 == 0) { // test accelerate
                if (!neteq_get_audio(context, outputSample, 2)) {
                    //printf("neteq get audio failed!\n");
                    return -1;
                }

                //fwrite(outputSample, sizeof(short), kSamples * 2, outfile);
                //}
            }
            
            neteq_get_nack_list(context, 100);

        }
        /*while (neteq_get_audio(outputSample, 2)) {
            printf("neteq audio buffer size %d!\n", neteq_get_buffer_size());
            printf("neteq operation %d!\n", neteq_get_last_operator());
            fwrite(outputSample, sizeof(short), kSamples * 2, outfile);
            
        }*/
        
        fseek(pcm, 0, SEEK_SET);
        printf("clear neteq!\n");
        clear_neteq(context);
        context = init_neteq(kSampleRateHz, 1, kPlayloadType);
        if (context == NULL) {
            printf("init neteq failed!\n");
            return -1;
        }
    }
    fflush(outfile);
    fclose(outfile);
    fclose(pcm);
    return 0;
}