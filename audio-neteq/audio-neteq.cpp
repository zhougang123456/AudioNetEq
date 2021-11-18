#include "api/audio/audio_frame.h"
#include "api/audio_codecs/builtin_audio_decoder_factory.h"
#include "modules/audio_coding/codecs/pcm16b/pcm16b.h"
#include "modules/audio_coding/neteq/default_neteq_factory.h"
#include "system_wrappers/include/clock.h"
#include "api/neteq/neteq.h"
#include "audio-neteq.h"
using namespace webrtc;
using webrtc::NetEq;
static std::unique_ptr<NetEq> neteq_ = nullptr;
#ifdef __cplusplus
extern "C" {
#endif
    namespace webrtc {

    }
    int init_neteq(int sample_rate, int channels, int encode_type)
    {
        if (neteq_ != nullptr) {
            return 0;
        }
        NetEq::Config config;
        config.sample_rate_hz = sample_rate;
        webrtc::Clock* clock = webrtc::Clock::GetRealTimeClock();
        auto audio_decoder_factory = CreateBuiltinAudioDecoderFactory();
        neteq_ =
            DefaultNetEqFactory().CreateNetEq(config, audio_decoder_factory, clock);
        if (neteq_ == nullptr) {
            return -1;
        }
        if (!neteq_->RegisterPayloadType(encode_type, SdpAudioFormat("l16", sample_rate, channels))) {
            return -1;
        }
        return 0;
    }

    void clear_neteq(void) {
        if (neteq_ != nullptr) {
            neteq_->FlushBuffers();
        }
        //neteq_ = nullptr;
    }

    int insert_packet(int seq_no, int timestamp, short* data, int samples)
    {   
        if (neteq_) {
            size_t payload_len = WebRtcPcm16b_Encode(data, samples, (uint8_t*)data);
            RTPHeader rtp_info;
            rtp_info.sequenceNumber = seq_no;
            rtp_info.timestamp = timestamp;
            rtp_info.ssrc = 0x1234;
            rtp_info.payloadType = 96;
            rtp_info.markerBit = 0;
            if (neteq_->InsertPacket(rtp_info, rtc::ArrayView<const uint8_t>((uint8_t*)data, payload_len)) == -1) {
                return -1;
            }
            return 0;
        }
        return -1;
    }

    int get_audio(short* data)
    {
        if (neteq_) {
            AudioFrame out_frame;
            bool muted;
            if (neteq_->GetAudio(&out_frame, &muted) == -1) {
                return 0;
            }
            memcpy(data, out_frame.data(), out_frame.samples_per_channel_ * 2);
            return out_frame.samples_per_channel_;
        }
        return 0;
    }

#ifdef __cplusplus
};
#endif