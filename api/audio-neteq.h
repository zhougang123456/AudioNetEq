#ifndef _AUDIO_NETEQ_H_
#define _AUDIO_NETEQ_H_
#ifdef __cplusplus
extern "C" {
#endif
	typedef struct NeteqContext NeteqContext;
	__declspec (dllexport) NeteqContext* __cdecl init_neteq(int sample_rate, int channels, int encode_type);
	__declspec (dllexport) void __cdecl clear_neteq(NeteqContext* context);
	__declspec (dllexport) int __cdecl neteq_insert_packet(NeteqContext* context, int seq_no, int timestamp, short* data, int samples);
	__declspec (dllexport) int __cdecl neteq_get_audio(NeteqContext* context, short* data, int channels);
	__declspec (dllexport) int __cdecl neteq_get_delay_time(NeteqContext* context, int timestamp);
	__declspec (dllexport) int __cdecl neteq_get_last_operator(NeteqContext* context);
	__declspec (dllexport) void __cdecl neteq_set_max_delay(NeteqContext* context, int delay);
	__declspec (dllexport) void __cdecl neteq_set_max_speed(NeteqContext* context, double speed);
	__declspec (dllexport) void __cdecl neteq_get_nack_list(NeteqContext* context, int round_trip_time);
#ifdef __cplusplus
};
#endif
#endif // _AUDIO_NETEQ_H_
