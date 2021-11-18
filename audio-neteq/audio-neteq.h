#ifndef _AUDIO_NETEQ_H_
#define _AUDIO_NETEQ_H_
#ifdef __cplusplus
extern "C" {
#endif

	extern int init_neteq(int sample_rate, int channels, int encode_type);
	extern void clear_neteq(void);
	extern int insert_packet(int seq_no, int timestamp, short* data, int samples);
	extern int get_audio(short* data);

#ifdef __cplusplus
};
#endif
#endif // _AUDIO_NETEQ_H_
