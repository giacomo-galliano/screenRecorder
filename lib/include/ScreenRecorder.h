#ifndef SCREEN_RECORDER_SCREENRECORDER_H
#define SCREEN_RECORDER_SCREENRECORDER_H

#include <atomic>
#include <iostream>
#include <sstream>
#include <memory>
#include <functional>
#include <map>
#include <malloc.h>
#include <thread>
#include <mutex>
#include <cinttypes>
#include <queue>
#include <list>
#include <condition_variable>

#include "AreaSelector.h"
#include "MemoryCheck.h"
#include "GetAudioDevices.h"
#include "Common.h"
#include "SettingsConf.h"

#define OUT_VIDEO_INDEX 0
#define OUT_AUDIO_INDEX 1

/*
 * todo: getVideo info?
 * todo: check if destroy everything needed
 */

using namespace  std;

inline const AVSampleFormat SAMPLEFMT = AV_SAMPLE_FMT_FLTP;
inline const AVPixelFormat PIXELFMT = AV_PIX_FMT_YUV420P;

enum RecStatus{STARTED, RECORDING, PAUSE, STOP};

class ScreenRecorder {
public:

    ScreenRecorder();
    ~ScreenRecorder();

    void open_();
    void start_();
    void pause_();
    void restart_();
    void stop_();

    void openAudioInput();
    void initAudioEncoder();
    void processAudio();

private:
    string outFileName;
    RecStatus status;
    int rec_type{};

    //VIDEO VARIABLES
    string videoInFmt;
    string videoDevice;
    long vPTS;
    int inVIndex;
    AVDictionary* sourceOptions{};
    AVInputFormat* inVFmt{};
    AVFormatContext* inVFmtCtx{};
    AVCodec* inVC{};
    AVCodecContext* inVCCtx{};
    SwsContext* swsCtx{};
    int cropX, cropY, cropWidth{}, cropHeight{};

    //AUDIO VARIABLES
    string audioInFmt;
    string audioDevice;
    long aPTS;
    int inAIndex;
    AVDictionary * audioOptions{};
    AVFormatContext* inAFmtCtx{};
    AVInputFormat *inAFmt{};
    AVCodec* inAC{};
    AVCodecContext* inACCtx{};
    AVAudioFifo* audioFifo{};
    SwrContext* swrCtx{};

    //OUTPUT VARIABLES
    AVFormatContext *outFmtCtx{};
    AVOutputFormat* outFmt{};
    AVCodec* outVC{};
    AVCodec* outAC{};
    AVCodecContext* outVCCtx{};
    AVCodecContext* outACCtx{};
    AVStream* outVStream{};
    AVStream* outAStream{};

    //lock variables
    mutex statusLock;
    mutex writeLock;
    condition_variable cv;
    mutex videoQueueMutex;

    queue<AVPacket *> video_queue;

    //thread variables
    thread* audioThread{};
    thread* videoThread{};
    thread* readVideoThread{};

    //functions
    void writeHeader();
    void writeTrailer();


    void openVideoInput();
    void initVideoEncoder();
    void readFrame();
    void processVideo();
    void videoPause();
    void areaSelection();
    //bool setCropParameters();

    void openOutput();

    string getRecState();
    string getVideoInfo();
    string getAudioInfo();

    void PSRMenu(); //pause-stop-restart menu
    void showPSROptions();
    static int getPSRAnswer();
    static bool validPSRAnswer(std::string &answer, int &res);
    void getFilenameOut(std::string& str);

};

#endif //SCREEN_RECORDER_SCREENRECORDER_H