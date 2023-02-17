#ifndef SDL_H
#define SDL_H

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>

#include "libswresample/swresample.h"
}

#define SDL_MAIN_HANDLED

#include <SDL3/SDL.h>
#include <SDL3/SDL_thread.h>
#include <stdio.h>
#include "window.h"

#include <algorithm>
#include <atomic>
#include <queue>

const int SDL_AUDIO_BUFFER_SIZE = 1024;
const int MAX_AUDIO_FRAME_SIZE = 192000;

struct PacketQueue {
    SDL_mutex *mutex;
    SDL_cond *cond;
    std::queue<AVPacket *> q;
};

PacketQueue audioq;
SwrContext *swr;
char *filename;
std::atomic_bool new_file(false), quit(false);
Window *window;

void packet_queue_init(PacketQueue *pq) {
    pq->mutex = SDL_CreateMutex();
    pq->cond = SDL_CreateCond();
}

void packet_queue_put(PacketQueue *pq, AVPacket *pkt) {
    SDL_LockMutex(pq->mutex);
    pq->q.push(pkt);
    SDL_CondSignal(pq->cond);
    SDL_UnlockMutex(pq->mutex);
}

void packet_queue_clear(PacketQueue *pq) {
    SDL_LockMutex(pq->mutex);
    pq->q = std::queue<AVPacket*>();
    SDL_UnlockMutex(pq->mutex);
}

int packet_queue_get(PacketQueue *pq, AVPacket **packet) {
    int result = -1;
    SDL_LockMutex(pq->mutex);
    while (!quit) {
        if (pq->q.empty()) {
            SDL_CondWait(pq->cond, pq->mutex);
        } else {
            *packet = pq->q.front();
            pq->q.pop();
            result = 0;
            break;
        }
    }
    SDL_UnlockMutex(pq->mutex);
    return result;
}

struct AudioState {
    SDL_mutex *mutex;
    AVFormatContext *formatCtx;
    AVCodecContext *codecCtx;
    int audioStream;
    AVStream *av_stream;
    double audio_clock;
    double duration;
    int volume;
    int64_t seek_pos;
    std::atomic_bool seek_request;
    int audio_buf_size;
    int audio_buf_index;
    SDL_AudioDeviceID device_id;
};

AudioState audio_state;

void audio_state_init(AudioState *audio_state) {
    audio_state->mutex = SDL_CreateMutex();
}

int audio_decode_frame(AudioState *audio_state, uint8_t *audio_buf) {
    AVFrame *frame = av_frame_alloc();

    while (true) {
        while (true) {
            if (quit) {
                return -1;
            }
            if (avcodec_receive_frame(audio_state->codecCtx, frame) < 0) {
                break;
            }
            int data_size = av_samples_get_buffer_size(NULL, audio_state->codecCtx->ch_layout.nb_channels, frame->nb_samples,
                                                       AV_SAMPLE_FMT_S16, 1);
            if (data_size <= 0) {
                continue;
            }
            int len =
                swr_convert(swr, &audio_buf, frame->nb_samples, (const uint8_t **)&frame->data[0], frame->nb_samples);
            if (len < 0) {
                av_log(NULL, AV_LOG_ERROR, "swr_convert() failed\n");
                return -1;
            }
            return data_size;
        }

        AVPacket *pkt;
        if (packet_queue_get(&audioq, &pkt) < 0) {
            return -1;
        }
        audio_state->audio_clock = pkt->pts * av_q2d(audio_state->av_stream->time_base);
        if (!audio_state->seek_request) {
            window->callUpdateSlider(audio_state->audio_clock * 1000 / audio_state->duration); //emit an event to be executed in the UI thread
        }

//        fprintf(stderr, "clock: %f\n", audio_state->audio_clock);
//        fprintf(stderr, "clock: %f\n", audio_state->duration);

        if (avcodec_send_packet(audio_state->codecCtx, pkt) < 0) {
            return -1;
        }
        av_packet_free(&pkt);
    }
    av_frame_free(&frame);
    return -1;
}

void audio_callback(void *userdata, Uint8 *stream, int size) {
    static uint8_t audio_buf[MAX_AUDIO_FRAME_SIZE * 3 / 2];
    AudioState *audio_state = (AudioState *)userdata;
    SDL_LockMutex(audio_state->mutex);
    while (size > 0) {
        if (audio_state->audio_buf_index >= audio_state->audio_buf_size) {
            int audio_size = audio_decode_frame(audio_state, audio_buf);
            if (audio_size < 0) {
                /* If error, output silence */
                audio_state->audio_buf_size = 1024;
                memset(audio_buf, 0, audio_state->audio_buf_size);
            } else {
                audio_state->audio_buf_size = audio_size;
            }
            audio_state->audio_buf_index = 0;
        }
        int len = std::min(audio_state->audio_buf_size - audio_state->audio_buf_index, size);
//        memcpy(stream, (uint8_t *)audio_buf + audio_state->audio_buf_index, len);
        memset(stream, 0, len);
        SDL_MixAudioFormat(stream, (uint8_t *)audio_buf + audio_state->audio_buf_index, AUDIO_S16SYS, len, audio_state->volume);
        size -= len;
        stream += len;
        audio_state->audio_buf_index += len;
    }
    SDL_UnlockMutex(audio_state->mutex);
}

int open_file(const char* filename) {
    packet_queue_clear(&audioq);
    AVFormatContext *formatCtx = audio_state.formatCtx;
    if (formatCtx) {
        avformat_close_input(&formatCtx);
    }
    int ret = avformat_open_input(&formatCtx, filename, NULL, NULL);
    if (ret) {
        fprintf(stderr, "Error opening file %s: error code %d\n", filename, ret);
        return -1;
    }

    if (avformat_find_stream_info(formatCtx, NULL) < 0)
        return -1;

    // Dump information about file onto standard error
    av_dump_format(formatCtx, 0, filename, 0);

    int audioStream = av_find_best_stream(formatCtx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (audioStream < 0)
        return -1;

    AVCodecParameters *codecParams = formatCtx->streams[audioStream]->codecpar;
    const AVCodec *aCodec = avcodec_find_decoder(codecParams->codec_id);
    if (!aCodec) {
        fprintf(stderr, "Unsupported codec!\n");
        return -1;
    }

    if (swr) {
        swr_free(&swr);
        swr = nullptr;
    }

    if (swr_alloc_set_opts2(&swr, &codecParams->ch_layout, AV_SAMPLE_FMT_S16, codecParams->sample_rate,
                            &codecParams->ch_layout, (AVSampleFormat)codecParams->format, codecParams->sample_rate, 0,
                            NULL) < 0) {
        fprintf(stderr, "swr_alloc_set_opts2() error\n");
        return -1;
    }

    if (swr_init(swr) < 0) {
        fprintf(stderr, "swr_init() error\n");
        return -1;
    }

    AVCodecContext *dec_ctx = audio_state.codecCtx;
    if (dec_ctx) {
        avcodec_free_context(&dec_ctx);
    }
    dec_ctx = avcodec_alloc_context3(aCodec);
    if (!dec_ctx) {
        return -1;
    }

    // Copy codec parameters from input stream to output codec context
    if (avcodec_parameters_to_context(dec_ctx, codecParams) < 0) {
        return -1;
    }

    AVDictionary *audioOptionsDict = NULL;
    avcodec_open2(dec_ctx, aCodec, &audioOptionsDict);

    audio_state.formatCtx = formatCtx;
    audio_state.codecCtx = dec_ctx;
    audio_state.audioStream = audioStream;
    audio_state.av_stream = formatCtx->streams[audioStream];
    audio_state.audio_clock = 0.0;
    audio_state.volume = 50;
    audio_state.seek_pos = 0;
    audio_state.seek_request = false;
    audio_state.audio_buf_size = 0;
    audio_state.audio_buf_index = 0;
    audio_state.duration = audio_state.av_stream->duration * av_q2d(audio_state.av_stream->time_base);

    // Set audio settings from codec info
    SDL_AudioSpec wanted_spec;
    wanted_spec.freq = codecParams->sample_rate;
    wanted_spec.format = AUDIO_S16SYS;
    wanted_spec.channels = codecParams->ch_layout.nb_channels;
    wanted_spec.silence = 0;
    wanted_spec.samples = SDL_AUDIO_BUFFER_SIZE;
    wanted_spec.callback = audio_callback;
    wanted_spec.userdata = &audio_state;

    SDL_AudioSpec spec;
    SDL_AudioDeviceID device_id = audio_state.device_id;
    if(device_id) {
        SDL_CloseAudioDevice(device_id);
    }
    device_id = SDL_OpenAudioDevice(NULL, 0, &wanted_spec, &spec, 0);
    if (!device_id) {
        fprintf(stderr, "SDL_OpenAudioDevice: %s\n", SDL_GetError());
        return -1;
    }
    audio_state.device_id = device_id;

    if (spec.format != AUDIO_S16SYS) {
        av_log(NULL, AV_LOG_ERROR, "SDL advised audio format %d is not supported!\n", spec.format);
        return -1;
    }

    SDL_PauseAudioDevice(device_id, 0);

    return 0;
}

int decode_thread(void *arg) {
    while (true) {
        if (new_file) {
            SDL_LockMutex(audio_state.mutex);
            open_file(filename);
            SDL_UnlockMutex(audio_state.mutex);
            new_file = false;
        }
        if (audio_state.seek_request) {
            AVRational a{1, AV_TIME_BASE};
            AVRational b{1, 1};
            int64_t seek_time = av_rescale_q(audio_state.duration * audio_state.seek_pos / 1000 , b, audio_state.av_stream->time_base);
            av_seek_frame(audio_state.formatCtx, audio_state.audioStream, seek_time, 0);
            audio_state.seek_request = false;
            SDL_LockMutex(audioq.mutex);
            audioq.q = std::queue<AVPacket*>();
            SDL_UnlockMutex(audioq.mutex);
        }
        SDL_LockMutex(audio_state.mutex);
        if (audio_state.formatCtx) {
            AVPacket *packet = av_packet_alloc();
            if (av_read_frame(audio_state.formatCtx, packet) < 0) {
                SDL_UnlockMutex(audio_state.mutex);
                SDL_Delay(100);
                continue;
            }
            if (packet->stream_index == audio_state.audioStream) {
                packet_queue_put(&audioq, packet);
            } else {
                av_packet_free(&packet);
            }
        }
        SDL_UnlockMutex(audio_state.mutex);
    }

    return 0;
}

int init_audio() {
    if (SDL_Init(SDL_INIT_EVERYTHING)) {
        fprintf(stderr, "Could not initialize SDL - %s\n", SDL_GetError());
        return -1;
    }
    packet_queue_init(&audioq);
    SDL_CreateThread(decode_thread, "decode_thread", nullptr);
    return 0;
}

#endif // SDL_H
