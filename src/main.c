#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>

typedef struct {
    int width;
    int height;
    AVFrame **frames;
    int count;
} FrameArray;

typedef struct {
    char *file;
} Video;

Video get_video_from_file(char *path_to_video) {
    Video video;

    return video;
}

FrameArray decode_video_to_frames(Video *video) {
    FrameArray frames;

    return frames;
}

void reverse_frames(FrameArray *frame_array) {
    for (int i = 0; i < frame_array->count / 2; i++) {
        AVFrame *temp = frame_array->frames[i];
        frame_array->frames[i] = frame_array->frames[frame_array->count - i - 1];
        frame_array->frames[frame_array->count - i - 1] = temp;
    }
}

Video encode_frames_to_video(FrameArray *frames) {
    Video video;

    return video;
}

void save_video_to_file(Video *video) {
    //
}

int main(int argc, char *argv[]) {
    FrameArray frames;
    Video video, reversed_video;

    // Get the video file
    video = get_video_from_file(argv[1]);

    // Decode it into an array of frames
    frames = decode_video_to_frames(&video);

    // Reverse the array
    reverse_frames(&frames);

    // Encode it into a video
    reversed_video = encode_frames_to_video(&frames);

    // Save the reversed video file
    save_video_to_file(&reversed_video);
    
    return 0;
}
