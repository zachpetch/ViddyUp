#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct {
    int width;
    int height;
    AVFrame **frames;
    int count;
} FrameArray;

int set_video_context(char *path_to_video, AVFormatContext **f_ctx, int *index) {
    if (avformat_open_input(f_ctx, path_to_video, NULL, NULL) < 0) {
        fprintf(stderr, "Unable to open file: %s\n", path_to_video);
        return -1;
    }

    if (avformat_find_stream_info(*f_ctx, NULL) < 0) {
        fprintf(stderr, "Unable to find stream information.\n");
        return -1;
    }

    *index = -1;
    for (int i = 0; i < (*f_ctx)->nb_streams; i++) {
        if ((*f_ctx)->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
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

int set_codec_context(AVFormatContext **f_ctx, AVCodecContext **c_ctx, const int *index) {
    const AVCodec *codec = avcodec_find_decoder((*f_ctx)->streams[*index]->codecpar->codec_id);
    if (!codec) {
        fprintf(stderr, "Unable to find codec.\n");
        return -1;
    }

    *c_ctx = avcodec_alloc_context3(codec);
    if (!*c_ctx) {
        fprintf(stderr, "Unable to allocate codec context.\n");
        return -1;
    }

    if (avcodec_parameters_to_context(*c_ctx, (*f_ctx)->streams[*index]->codecpar) > 0) {
        fprintf(stderr, "Unable to copy codec parameters to context.\n");
        return -1;
    }

    if (avcodec_open2(*c_ctx, codec, NULL) > 0) {
        fprintf(stderr, "Unable to open codec.\n");
        return -1;
    }

    return 0;
}

// TODO: I think we can replace this, either by simply using `codec_context->codec`,
// or by initializing the codec in `main()` and passing it along to set_codec_context().
int get_codec(AVCodecContext **codec_context, const AVCodec **codec) {
    *codec = avcodec_find_decoder((*codec_context)->codec_id);
    if (!*codec) {
        fprintf(stderr, "Video codec is unsupported.\n");
        return -1;
    }

    if (avcodec_open2(*codec_context, *codec, NULL) < 0) {
        fprintf(stderr, "Unable to open codec.\n");
        return -1;
    }

    return 0;
}

int decode_frames(FrameArray **frame_array, AVFormatContext **f_ctx, AVCodecContext **c_ctx, AVPacket *packet, const int index) {
    AVFrame *frame = NULL;
    frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Unable to allocate frame.\n");
        return -1;
    }

    (*frame_array)->width = (*c_ctx)->width;
    (*frame_array)->height = (*c_ctx)->height;
    (*frame_array)->count = 0;
    (*frame_array)->frames = malloc(sizeof(AVFrame*) * 1800); // TODO: Find a way to get video frame count. Or do different allocation. 1800 is one minute of 30fps video.

    // TODO: DELETE THIS
    fprintf(stdout, "FrameArray configured\n");

    while (av_read_frame(*f_ctx, packet) >= 0) {
        // TODO: DELETE THIS
        fprintf(stdout, "av_read_frame succeeded\n");

        if (packet->stream_index == index) {
            int check = avcodec_send_packet(*c_ctx, packet);
            if (check < 0) {
                fprintf(stderr, "Unable to send packet for decoding.\n");
                break; // TODO: Should this return -1 instead? Contemplate and try some day.
            }

            // TODO: DELETE THIS
            fprintf(stdout, "av_read_frame(f_ctx, packet) is indeed > 0\n");

            while (check >= 0) {
                // TODO: DELETE THIS
                fprintf(stdout, "They told me this would always be true\n");

                check = avcodec_receive_frame(*c_ctx, frame);
                if (check == AVERROR(EAGAIN) || check == AVERROR_EOF) {
                    break;
                }

                if (check < 0) {
                    fprintf(stderr, "An error occurred during the decoding process.\n");
                    return -1;
                }

                // TODO: DELETE THIS
                fprintf(stdout, "Frame decoded by avcodec\n");

                AVFrame *frame_copy = av_frame_clone(frame);
                (*frame_array)->frames[(*frame_array)->count++] = frame_copy;

                // TODO: DELETE THIS
                fprintf(stdout, "Frame copied to FrameArray\n");
            }
        }
        av_packet_unref(packet);
    }

    // TODO: DELETE THIS
    fprintf(stdout, "Frames decoded\n");
    return 0;
}

int reverse_frames(FrameArray **frame_array) {
    for (int i = 0; i < (*frame_array)->count / 2; i++) {
        AVFrame *temp = (*frame_array)->frames[i];
        (*frame_array)->frames[i] = (*frame_array)->frames[(*frame_array)->count - i - 1];
        (*frame_array)->frames[(*frame_array)->count - i - 1] = temp;
    }

    // TODO: DELETE THIS
    fprintf(stdout, "Frames reversed\n");
    return 0;
}

int encode_frames_to_video(FrameArray **frame_array) {
    if ((*frame_array)->count > 0) {
        fprintf(stdout, "test\n");
    }

    // TODO: DELETE THIS
    fprintf(stdout, "Frames encoded\n");
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
    const AVCodec *codec = NULL;
    AVPacket *packet = NULL;
    FrameArray *frame_array;

    set_video_context(file_path, &format_context, &video_stream_index);
    if (format_context == NULL) {
        fprintf(stderr, "Unable to set format context.\n");
        return 1;
    }
    fprintf(stdout, "Set Format Context: %s\n", format_context->iformat->long_name);

    set_codec_context(&format_context, &codec_context, &video_stream_index);
    if (codec_context == NULL) {
        fprintf(stderr, "Unable to set codec context.\n");
        return 1;
    }
    fprintf(stdout, "Codec Type: %s\n", codec_context->codec->long_name);

    get_codec(&codec_context, &codec);
    if (codec == NULL) {
        fprintf(stderr, "Unable to get codec.\n");
        return 1;
    }
    fprintf(stdout, "Codec: %s\n", codec->long_name);

    decode_frames(&frame_array, &format_context, &codec_context, packet, video_stream_index);
    fprintf(stdout, "Decoded %d frames\n", frame_array->count);

    // Reverse the array
    reverse_frames(&frame_array);

    // Encode it into a video
    encode_frames_to_video(&frame_array);

    // Save the reversed video file
    save_video_to_file();

    for (int i = 0; i < frame_array->count; i++) {
        av_frame_free(&frame_array->frames[i]);
    }
    free(frame_array->frames);
    avcodec_free_context(&codec_context);
    avformat_close_input(&format_context);
    // TODO: Check for memory leaks

    return 0;
}

