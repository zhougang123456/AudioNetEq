#include "api/audio/audio_frame.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "api/neteq/neteq.h"
#include "modules/audio_coding/codecs/pcm16b/pcm16b.h"
#include "modules/audio_coding/neteq/default_neteq_factory.h"
#include "system_wrappers/include/clock.h"
using namespace webrtc;
using webrtc::NetEq;
static void PopulateRtpInfo(int frame_index, int timestamp, RTPHeader* rtp_info)
{
    rtp_info->sequenceNumber = frame_index;
    rtp_info->timestamp = timestamp;
    rtp_info->ssrc = 0x1234;
    rtp_info->payloadType = 96;
    rtp_info->markerBit = 0;
}

int maintest() {
    const int kSampleRateHz = 48000;
    const int kPlayloadType = 96;

    NetEq::Config config;
    config.sample_rate_hz = kSampleRateHz;
    webrtc::Clock* clock = webrtc::Clock::GetRealTimeClock();
    auto audio_decoder_factory = CreateBuiltinAudioDecoderFactory();
    auto neteq =
        DefaultNetEqFactory().CreateNetEq(config, audio_decoder_factory, clock);
    if (!neteq->RegisterPayloadType(kPlayloadType, SdpAudioFormat("l16", kSampleRateHz, 1))) {
        printf("register playload type failed!\n");
        return -1;
    }
    AudioFrame out_frame;
    uint32_t timestamp = 0;
    uint16_t seq_no = 0;
    const int kFframSizeMs = 10;
    const size_t kSamples = kFframSizeMs * 48;
    const size_t kPayloadBytes = kSamples * 2;
    double next_input_time_ms = 0.0;
    double t_ms;
    bool muted;
    const int kSpeechDurationMs = 5000;
    const double driftFactor = 1.0;
    FILE* pcm = fopen("48.pcm", "rb");
    if (pcm == NULL) {
        printf("open pcm file failed!\n");
        return 0;
    }
    FILE* outfile = fopen("plc.pcm", "wb");
    int16_t inputSample[kSamples] = { 0 };
    uint8_t playload[kPayloadBytes] = { 0 };
    for (t_ms = 0; t_ms < kSpeechDurationMs && !feof(pcm); t_ms += 10) {
        int read = fread(inputSample, sizeof(int16_t), kSamples, pcm);
        if (read != kSamples) {
            printf("read: %d\n", read);
            break;
        }
        size_t payload_len = WebRtcPcm16b_Encode(inputSample, kSamples, playload);
        //while (next_input_time_ms <= t_ms) {
        //if (seq_no % 4 != 1) {
            RTPHeader rtp_info;
            PopulateRtpInfo(seq_no, timestamp, &rtp_info);
            if (neteq->InsertPacket(rtp_info, playload) == -1) {
                printf("neteq insert packet failed!\n");
                return -1;
            }
            printf("neteq insert packet %u success!\n", seq_no);
         
        //}
        ++seq_no;
        timestamp += kSamples;
        next_input_time_ms += static_cast<double>(kFframSizeMs) * driftFactor;

        //}
        if (seq_no % 100 == 0) {
            if (neteq->GetAudio(&out_frame, &muted) == -1) {
                printf("neteq get audio failed!\n");
                return -1;
            }
          
            fwrite(out_frame.data(), sizeof(int16_t), out_frame.samples_per_channel_, outfile);
            printf("audio frame size is %ld\n", out_frame.samples_per_channel_);
        }

    }
    while (neteq->GetAudio(&out_frame, &muted) == 0 && out_frame.samples_per_channel_ > 0) {
        fwrite(out_frame.data(), sizeof(int16_t), out_frame.samples_per_channel_, outfile);
        printf("audio frame size is %ld\n", out_frame.samples_per_channel_);
    }

    fflush(outfile);
    fclose(outfile);
    fclose(pcm);
    return 0;
}