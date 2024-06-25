#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct {
    int width;
    int height;
    AVFrame **frames;
    int count;
} FrameArray;

int set_video_context(char* path_to_video, AVFormatContext* f_ctx, int* index) {
    if (avformat_open_input(f_ctx, path_to_video, NULL, NULL) < 0) {
        fprintf(stderr, "Unable to open file: %s\n", path_to_video);
        return -1;
    }

    if (avformat_find_stream_info(f_ctx, NULL) < 0) {
        fprintf(stderr, "Unable to find stream information.\n");
        return -1;
    }

    *index = -1;
    for (int i = 0; i < f_ctx->nb_streams; i++) {
        if (f_ctx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            *index = i;
            break;
        }
    }
    if (*index == -1) {
        fprintf(stderr, "Unable to find video stream.\n");
        return -1;
    }

    return 0;
}

int set_codec_context(AVFormatContext *f_ctx, AVCodecContext *c_ctx, int *index) {
    c_ctx = avcodec_alloc_context3(NULL);
    if (!c_ctx) {
        fprintf(stderr, "Unable to allocate codec context.\n");
        return -1;
    }

    avcodec_parameters_to_context(c_ctx, f_ctx->streams[*index]->codecpar);

    return 0;
}

// TODO: I think we can replace using a codec at all with simply using `codec_context->codec`.
int get_codec(AVCodecContext *codec_context, AVCodec *codec) {
    codec = avcodec_find_decoder(codec_context->codec_id);
    if (!codec) {
        fprintf(stderr, "Video codec is unsupported.\n");
        return -1;
    }

    if (avcodec_open2(codec_context, codec, NULL) < 0) {
        fprintf(stderr, "Unable to open codec.\n");
        return -1;
    }

    return 0;
}

int decode_frames(FrameArray *frame_array, AVFormatContext *f_ctx, AVCodecContext *c_ctx, AVPacket *packet, int *index) {
    AVFrame *frame = NULL;
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Unable to allocate frame.\n");
        return -1;
    }
    
    int check;
    frame_array->width = c_ctx->width;
    frame_array->height = c_ctx->height;
    frame_array->count = 0;
    frame_array->frames = malloc(sizeof(AVFrame*) * 1800); // TODO: Find a way to get video frame count. Or do different allocation. 1800 is one minute of 30fps video.
    
    while (av_read_frame(f_ctx, &packet) >= 0) {
        if (packet->stream_index == index) {
            check = avcodec_send_packet(c_ctx, &packet);
            if (check < 0) {
                fprintf(stderr, "Unable to send packet for decoding.\n");
                break; // TODO: Should this return -1 instead? Contemplate and try some day.
            }

            while (check >= 0) {
                check = avcodec_receive_frame(c_ctx, frame);
                if (check == AVERROR(EAGAIN) || check == AVERROR_EOF) {
                    break;
                } else if (check < 0) {
                    fprintf(stderr, "An error occurred during the decoding process.\n");
                    return -1;
                }

                AVFrame *frame_copy = av_frame_clone(frame);
                frame_array->frames[frame_array->count++] = frame_copy;
            }
        }
        av_packet_unref(&packet);
    }

    return 0;
}

int reverse_frames(FrameArray *frame_array) {
    for (int i = 0; i < frame_array->count / 2; i++) {
        AVFrame *temp = frame_array->frames[i];
        frame_array->frames[i] = frame_array->frames[frame_array->count - i - 1];
        frame_array->frames[frame_array->count - i - 1] = temp;
    }
    
    return 0;
}

int encode_frames_to_video(FrameArray *frames) {
    return 0;
}

int save_video_to_file() {
    return 0;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s path/to/input/video.mp4\n", argv[0]);
    }

    char *file_path = argv[1];
    int video_stream_index;
    AVFormatContext *format_context = NULL;
    AVCodecContext *codec_context = NULL;
    AVCodec *codec = NULL;
    AVPacket *packet;
    FrameArray frames;

    // TODO: Add "if -1, fprintf>fail, goto end/cleanup" under each of these.
    set_video_context(file_path, &format_context, &video_stream_index);
    set_codec_context(&format_context, &codec_context, &video_stream_index);
    printf("Framerate: %d/%d\n", codec_context->framerate.num, codec_context->framerate.den); // TODO: use this to get frame rate count, if possible ... this multiplied by number of seconds in video file? But we'd need to find a way to get that info too.
    get_codec(&codec_context, &codec);
    decode_frames(&frames, &format_context, &codec_context, &packet, &video_stream_index);

    // Reverse the array
    reverse_frames(&frames);

    // Encode it into a video
    encode_frames_to_video(&frames);

    // Save the reversed video file
    save_video_to_file();
    
    return 0;
}
