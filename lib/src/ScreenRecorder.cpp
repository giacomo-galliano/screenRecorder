#include "../include/ScreenRecorder.h"

ScreenRecorder::ScreenRecorder() : status(RecStatus::STARTED), fps(15), factor(1), cropX(0), cropY(0), vPTS(0), aPTS(0), inAIndex(-1), inVIndex(-1){
    avdevice_register_all();
//  avformat_network_init();

    av_log_set_level(AV_LOG_ERROR);

#ifdef __linux__
    memoryCheck_init(3000);
#endif
}

ScreenRecorder::ScreenRecorder(int fps_, int factor_) : status(RecStatus::STARTED), fps(fps_), factor(factor_), cropX(0), cropY(0), vPTS(0), aPTS(0), inAIndex(-1), inVIndex(-1){
    avdevice_register_all();
//  avformat_network_init();

    av_log_set_level(AV_LOG_ERROR);

    if(fps_ != 15 && fps_ != 24 && fps_ != 30 && fps_ != 60){
        throw logic_error ("Fps must be equals to one of these values: 15, 24, 30, 60");
    }

#ifdef __linux__
    memoryCheck_init(3000);
#endif
}

ScreenRecorder::~ScreenRecorder() {
    readVideoThread->join();
    switch(rec_type){
        case Command::vofs:
            videoThread->join();
            break;
        case Command::avfs:
            videoThread->join();
            audioThread->join();
            avformat_close_input(&inAFmtCtx);
            avformat_free_context(inAFmtCtx);
            break;
        case Command::vosp:
            videoThread->join();
            break;
        case Command::avsp:
            videoThread->join();
            audioThread->join();
            break;
        default:
            std::cout << "Command not recognized" << std::endl;
    }
    avformat_close_input(&inVFmtCtx);
    avformat_free_context(inVFmtCtx);

    writeTrailer();
}

void ScreenRecorder::open_(){

    SettingsConf sc;
    sc.welcomeMsg();

    rec_type = sc.optionsMenu();

    switch(rec_type){
        case Command::vofs:
            openVideoInput();
            break;
        case Command::avfs:
            openVideoInput();
            openAudioInput();
            break;
        case Command::vosp:
            //while(!setCropParameters());
            areaSelection();
            openVideoInput();
            break;
        case Command::avsp:
            //while(!setCropParameters());
            areaSelection();
            openVideoInput();
            openAudioInput();
            break;
        case Command::stop:
            break;
        default:
            std::cout << "Command not recognized" << std::endl;
    }
    if(rec_type != Command::stop)
        openOutput();
}

void ScreenRecorder::start_(){
    unique_lock<mutex> ul(statusLock);

    if(status == RecStatus::STARTED) {

        status = RecStatus::RECORDING;
        ul.unlock();

        switch (rec_type) {
            case Command::vofs:
                initVideoEncoder();
                writeHeader();
                readVideoThread = new std::thread([this]() {
                    this->readFrame();
                });

                videoThread = new std::thread([this]() {
                    this->processVideo();
                });

                break;
            case Command::avfs:
                initVideoEncoder();
                initAudioEncoder();
                writeHeader();

                readVideoThread = new std::thread([this]() {
                    this->readFrame();
                });
                videoThread = new std::thread([this]() {
                    this->processVideo();
                });
                audioThread = new std::thread([this]() {
                    this->processAudio();
                });
                break;
            case Command::vosp:
                initVideoEncoder();
                writeHeader();
                readVideoThread = new std::thread([this]() {
                    this->readFrame();
                });
                videoThread = new std::thread([this]() {
                    this->processVideo();
                });
                break;
            case Command::avsp:
                initVideoEncoder();
                initAudioEncoder();
                writeHeader();

                readVideoThread = new std::thread([this]() {
                    this->readFrame();
                });
                videoThread = new std::thread([this]() {
                    this->processVideo();
                });
                audioThread = new std::thread([this]() {
                    this->processAudio();
                });
                break;
            case Command::stop:
                break;
            default:
                std::cout << "Command not recognized" << std::endl;
        }
        if (rec_type != Command::stop)
            std::cout << "\033[1;32m" << "Recording... " << "\033[0m" << std::endl;
        else
            exit(0);
        PSRMenu();
    }else{
        throw logic_error("ACTION NOT PERMITTED. Application already started");
    }

}

void ScreenRecorder::pause_(){
    lock_guard<mutex> ul(statusLock);
    if(status != RecStatus::STOP){
        status = RecStatus::PAUSE;
        std::cout << "\033[1;33m" << "Recording paused" << "\033[0m" << std::endl;
    }
}

void ScreenRecorder::restart_(){
    lock_guard<mutex> lg(statusLock);
    if(status != RecStatus::STOP) {
#ifdef __linux__
        videoPause();
        openVideoInput();
#endif
        status = RecStatus::RECORDING;
        cv.notify_all();
        std::cout << "\033[1;32m" << "Recording resumed" << "\033[0m" << std::endl;
    }
}

void ScreenRecorder::stop_(){
    lock_guard<mutex> lg(statusLock);
    status = RecStatus::STOP;
    cv.notify_all();
    std::cout << "\033[1;31m" << "Recording stopped" << "\033[0m" << std::endl;
}

void ScreenRecorder::writeHeader(){
    if(avio_open2(&outFmtCtx->pb, outFileName.c_str(), AVIO_FLAG_WRITE, nullptr, nullptr) < 0){
        throw runtime_error{"Could not open out file."};
    }
    if (avformat_write_header(outFmtCtx, nullptr) < 0) {
        throw runtime_error{"Could not write header."};
    }

}

void ScreenRecorder::writeTrailer(){
    if(av_write_trailer(outFmtCtx) < 0) {
        throw runtime_error{"Could not write trailer."};
    }

    if(avio_close(outFmtCtx->pb) < 0){
        throw runtime_error{"Could not close file."};
    }

}

void ScreenRecorder::areaSelection() {
    char  arg0[] = "areaSelection";
    char* argv[] = { &arg0[0],  NULL };
    int   argc   = (int)(sizeof(argv) / sizeof(argv[0])) - 1;

    QApplication a(argc, argv);
    Window window;
    window.show();
    a.exec();
    cropX = window.x;
    cropY = window.y;
    cropWidth = window.W;
    cropHeight = window.H;
}

void ScreenRecorder::openVideoInput() {

    sourceOptions = nullptr;
    inVFmtCtx = avformat_alloc_context();

#ifdef __linux__
    videoInFmt = "x11grab";
#endif
#ifdef _WIN32
    videoInFmt = "gdigrab";
#endif
#ifdef MACOS
    videoInFmt = "avfoundation";
#endif
    inVFmt = av_find_input_format(videoInFmt.c_str());
    if (inVFmt == nullptr) {
        throw logic_error{"av_find_input_format not found..."};
    }

    av_dict_set (&sourceOptions, "framerate", to_string(fps).c_str(), 0);
    av_dict_set (&sourceOptions, "probesize", "40M", 0);
    av_dict_set (&sourceOptions, "threads", "8", 0);

    if(rec_type == Command::vosp || rec_type == Command::avsp) {
        string ratio = to_string(cropWidth)+"x"+to_string(cropHeight);
        av_dict_set(&sourceOptions, "video_size", ratio.c_str(), 0);
    }

#ifdef __linux__
    videoDevice = ":0.0+" + std::to_string(cropX) + "," + std::to_string(cropY);
#endif
#ifdef WIN32
    av_dict_set (&sourceOptions, "offset_x", cropX.c_str(), 0);
    av_dict_set (&sourceOptions, "offset_y", cropY.c_str(), 0);

    videoDevice = "desktop";
#endif
#ifdef MACOS
    av_dict_set(&sourceOptions, "pixel_format", "0rgb", 0);
    av_dict_set(&sourceOptions, "video_device_index", "1", 0);
    av_dict_set(&sourceOptions, "preset", "ultrafast", 0);
    av_dict_set(&sourceOptions, "pixel_format", "uyvy422", 0);

    videoDevice = "Capture screen 0:none";
#endif

    if( avformat_open_input(&inVFmtCtx, videoDevice.c_str(), inVFmt, &sourceOptions) != 0) {
        throw logic_error{"Could not find input device"};
    }

    if(avformat_find_stream_info(inVFmtCtx, &sourceOptions) < 0 ){
        throw logic_error{"cannot find correct stream info..."};
    }

    for(int i=0; i < inVFmtCtx->nb_streams; i++){
        if (inVFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO){
            inVIndex = i;
            break;
        }
    }
    if (inVIndex == -1 || inVIndex >= (int)inVFmtCtx->nb_streams) {
        throw logic_error{"Didn't find a video stream."};
    }

    inVC = avcodec_find_decoder(inVFmtCtx->streams[inVIndex]->codecpar->codec_id);
    if(!inVC){
        throw logic_error{"Decoder codec not found."};
    }

    inVCCtx = avcodec_alloc_context3(inVC);
    if(!inVCCtx){
        throw runtime_error{"Could not allocate video context"};
    }
    if(avcodec_parameters_to_context(inVCCtx, inVFmtCtx->streams[inVIndex]->codecpar) < 0){
        throw runtime_error{"Video parameter to context error"};
    }

    if(avcodec_open2(inVCCtx, inVC, nullptr) < 0){
        throw runtime_error{"Could not open decoder."};
    }
}

void ScreenRecorder::videoPause() {
    avformat_close_input(&inVFmtCtx);
    if (inVFmtCtx != nullptr) {
        throw runtime_error("Unable to close the inVFmtCtx (before pause)");
    }
    avformat_free_context(inVFmtCtx);
//    avcodec_close(inVCCtx);
//    if (inVCCtx != nullptr) {
//        throw runtime_error("Unable to close the inVCCtx (before pause)");
//    }
//    avcodec_free_context(&inVCCtx);

}

void ScreenRecorder::initVideoEncoder(){

    outVC = avcodec_find_encoder(AV_CODEC_ID_H264);
    if(!outVC){
        throw logic_error{"Encoder codec not found"};
    }

    outVStream = avformat_new_stream(outFmtCtx, outVC);
//    outVStream->index = OUT_VIDEO_INDEX;
    if(!outVStream){
        throw runtime_error{"Could not create out video stream."};
    }

    outVCCtx = avcodec_alloc_context3(outVC);
    if(!outVCCtx){
        throw runtime_error{"Could not allocate video encoder contex"};
    }

    outVCCtx->width = inVCCtx->width*factor;
    outVCCtx->height = inVCCtx->height*factor;
    outVCCtx->pix_fmt = PIXELFMT;
    outVCCtx->time_base = (AVRational){1,2*fps};
    outVCCtx->framerate = (AVRational){fps,1};//av_inv_q(cCtx->time_base);
    if ( outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        outVCCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if (outVCCtx->codec_id == AV_CODEC_ID_H264) {
        av_opt_set(outVCCtx, "preset", "ultrafast", 0);
    }


    if(avcodec_open2(outVCCtx, outVC, nullptr) < 0){
        throw runtime_error{"Failed to open video encoder."};
    }

    if(avcodec_parameters_from_context(outFmtCtx->streams[OUT_VIDEO_INDEX]->codecpar, outVCCtx) < 0){
        throw runtime_error{"Video parameter from context error"};
    }

    swsCtx = sws_getContext(
            inVCCtx->width,
            inVCCtx->height,
            inVCCtx->pix_fmt,
            outVCCtx->width,
            outVCCtx->height,
            outVCCtx->pix_fmt,
            SWS_FAST_BILINEAR, nullptr, nullptr, nullptr);
}

void ScreenRecorder::openAudioInput() {
    audioOptions = nullptr;
    inAFmtCtx = avformat_alloc_context();
    if (!inAFmtCtx) {
        throw runtime_error{"Cannot alloc audio format context"};
    }

    av_dict_set(&audioOptions, "sample_rate", "44100", 0);
    av_dict_set(&audioOptions, "async", "25", 0);
#ifdef __linux__
    audioInFmt = "alsa";
    vector<string> devices = getAudioDevices();
    if(devices.empty())
        throw runtime_error("No audio device available");
    audioDevice =devices[0];
#endif
#ifdef _WIN32
    audioInFmt = "dshow";
    audioDevice = "audio="+deviceName;
#endif

    inAFmt = av_find_input_format(audioInFmt.c_str());
    if (inAFmt == nullptr) {
        throw runtime_error{"Cannot open " + audioInFmt + " driver"};
    }
    if (avformat_open_input(&inAFmtCtx, audioDevice.c_str(), inAFmt, nullptr) < 0) {
        throw runtime_error("cannot open video device");
    }
//    inAFmt = av_find_input_format("pulse");
//    if (inAFmt == NULL) {
//        throw runtime_error{"Cannot open PULSE driver"};
//    }
//    if (avformat_open_input(&inAFmtCtx, "default", inAFmt, nullptr) < 0) {
//        throw runtime_error("cannot open video device");
//    }


    if(avformat_find_stream_info(inAFmtCtx, &audioOptions) < 0 ){
        throw logic_error{"cannot find correct stream info..."};
    }

    for(int i=0; i < inAFmtCtx->nb_streams; i++){
        if (inAFmtCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO){
            inAIndex = i;
            break;
        }
    }
    if (inAIndex == -1 || inAIndex >= (int)inAFmtCtx->nb_streams) {
        throw logic_error{"Didn't find a video stream."};
    }

    inAC = avcodec_find_decoder(inAFmtCtx->streams[inAIndex]->codecpar->codec_id);
    if(!inAC){
        throw logic_error{"Decoder codec not found."};
    }

    inACCtx = avcodec_alloc_context3(inAC);
    if(!inACCtx){
        throw runtime_error{"Could not allocate video contex"};
    }
    if(avcodec_parameters_to_context(inACCtx, inAFmtCtx->streams[inAIndex]->codecpar) < 0){
        throw runtime_error{"Video parameter to context error"};
    }

    if(avcodec_open2(inACCtx, inAC, nullptr) < 0){
        throw runtime_error{"Could not open decoder."};
    }

}

void ScreenRecorder::initAudioEncoder() {
    outAC = avcodec_find_encoder(AV_CODEC_ID_AAC);
    if(!outAC){
        throw logic_error{"Encoder codec not found"};
    }

    outAStream = avformat_new_stream(outFmtCtx, outAC);
    if(!outAStream){
        throw runtime_error{"Could not create out video stream."};
    }

    outACCtx = avcodec_alloc_context3(outAC);
    if(!outACCtx){
        throw runtime_error{"Could not allocate video encoder contex"};
    }

    if ((outAC)->supported_samplerates) {
        outACCtx->sample_rate = (outAC)->supported_samplerates[0];
        for (int i = 0; (outAC)->supported_samplerates[i]; i++) {
            if ((outAC)->supported_samplerates[i] == inACCtx->sample_rate)
                outACCtx->sample_rate = inACCtx->sample_rate;
        }
    }



    outACCtx->codec_id = AV_CODEC_ID_AAC;
    outACCtx->bit_rate = 128000;
    outACCtx->channels = inACCtx->channels;
    outACCtx->channel_layout = av_get_default_channel_layout(outACCtx->channels);
    outACCtx->sample_fmt = outAC->sample_fmts ? outAC->sample_fmts[0] : SAMPLEFMT;
    outACCtx->time_base = {1, inACCtx->sample_rate};
    outACCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    if ( outFmtCtx->oformat->flags & AVFMT_GLOBALHEADER)
        outVCCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

    if(avcodec_open2(outACCtx, outAC, nullptr) < 0){
        throw runtime_error{"Failed to open video encoder."};
    }

    if(avcodec_parameters_from_context(outFmtCtx->streams[OUT_AUDIO_INDEX]->codecpar, outACCtx) < 0){
        throw runtime_error{"Video parameter from context error"};
    }

}

void ScreenRecorder::openOutput() {
    getFilenameOut(outFileName);
    outFmtCtx = avformat_alloc_context();
    outFmt = av_guess_format(nullptr, outFileName.c_str(), nullptr);
    if (!outFmt) {
        throw runtime_error{"Cannot guess format"};
    }
    avformat_alloc_output_context2(&outFmtCtx, outFmt, outFmt->name, outFileName.c_str());
}

void ScreenRecorder::readFrame(){
    AVPacket* pkt;

    while(true) {
        unique_lock<mutex> ul(statusLock);
        if (status == RecStatus::STOP){
            break;
        }
        cv.wait(ul, [this]() { return status != RecStatus::PAUSE; });
        ul.unlock();

#ifdef __linux__
        if(memoryCheck_limitSurpassed()){
            pause_();
            cout << "\033[1;33m" << "Overload! Needed to pause to avoid memory saturation. Try to reduce screen resolution to improve performances" << "\033[0m \n>> " << endl;
        }
#endif

        pkt = av_packet_alloc();
        if (av_read_frame(inVFmtCtx, pkt) < 0) {
            throw std::runtime_error("Error in getting RawPacket");
        }

        unique_lock<mutex> video_queue_ul{videoQueueMutex};
        video_queue.push(pkt);
        video_queue_ul.unlock();

    }
}

void ScreenRecorder::processVideo() {

    AVPacket *inPkt;

    AVPacket outPkt;
    av_init_packet(&outPkt);

    AVFrame* frame = av_frame_alloc();;

    AVFrame *convFrame = av_frame_alloc();;
    if(!convFrame){
        throw runtime_error{"Could not allocate convFrame."};
    }

    uint8_t *buffer = (uint8_t *) av_malloc(
            av_image_get_buffer_size( outVCCtx->pix_fmt,outVCCtx->width,outVCCtx->height,1));
    if (buffer == NULL) {
        throw runtime_error{"Could not allocate image buffer."};
    }

    if((av_image_fill_arrays(convFrame->data, convFrame->linesize,
                             buffer,
                             outVCCtx->pix_fmt,
                             outVCCtx->width,
                             outVCCtx->height, 1)) <0){
        throw runtime_error{"Could not fill arrays."};
    }

    int res = 0;

    while(true){

        unique_lock<mutex> video_queue_ul{videoQueueMutex};
        if(!video_queue.empty()) {
            inPkt = video_queue.front();
            video_queue.pop();

            video_queue_ul.unlock();

            if(inPkt->stream_index == inVIndex){

                inPkt->pts =  vPTS++ * outFmtCtx->streams[OUT_VIDEO_INDEX]->time_base.den / outVCCtx->framerate.num;

                res = avcodec_send_packet(inVCCtx, inPkt);
                av_packet_unref(inPkt);
                av_packet_free(&inPkt);
                if (res < 0) {
                    throw runtime_error("Decoding Error: sending packet");
                }
                if( avcodec_receive_frame(inVCCtx, frame) == 0){

                    convFrame->width = outVCCtx->width;
                    convFrame->height = outVCCtx->height;
                    convFrame->format = outVCCtx->pix_fmt;
                    av_frame_copy_props(convFrame, frame);

                    sws_scale(swsCtx,frame->data, frame->linesize, 0,
                              frame->height, convFrame->data, convFrame->linesize);

                    if(avcodec_send_frame(outVCCtx, convFrame) >= 0){
                        if(avcodec_receive_packet(outVCCtx, &outPkt) >= 0){
                            unique_lock<mutex> writeLock_ul(writeLock);
                            //todo: interleaved_write or write?
                            if (av_write_frame(outFmtCtx, &outPkt) < 0) {
                                throw runtime_error("Error in writing file");
                            }
                            writeLock_ul.unlock();
                        }
                    }
                }else{
                    throw runtime_error("Decoding Error: receiving frame");
                }
                av_frame_unref(frame);
                av_packet_unref(&outPkt);
            }
        }else{

            video_queue_ul.unlock();
            unique_lock<mutex> ul(statusLock);

            if (status == RecStatus::STOP)
                break;

            cv.wait(ul, [this]() { return status != RecStatus::PAUSE; });
            ul.unlock();
        }
    }
    av_packet_unref(&outPkt);
    av_frame_free(&frame);
    av_frame_free(&convFrame);

}

void ScreenRecorder::processAudio() {
    int ret;
    AVPacket *inPacket, *outPacket;
    AVFrame *rawFrame, *scaledFrame;
    uint8_t **resampledData;

    /* Create the FIFO buffer based on the specified output sample format. */
    if (!(audioFifo = av_audio_fifo_alloc(outACCtx->sample_fmt, outACCtx->channels, 1))) {
        throw runtime_error("Could not allocate FIFO");
    }
    //allocate space for a packet
    inPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (!inPacket) {
        throw runtime_error("Cannot allocate an AVPacket for encoded video");
    }
    av_init_packet(inPacket);

    //allocate space for a packet
    rawFrame = av_frame_alloc();
    if (!rawFrame) {
        throw runtime_error("Cannot allocate an AVPacket for encoded video");
    }

    scaledFrame = av_frame_alloc();
    if (!scaledFrame) {
        throw runtime_error("Cannot allocate an AVPacket for encoded video");
    }

    outPacket = (AVPacket *)av_malloc(sizeof(AVPacket));
    if (!outPacket) {
        throw runtime_error("Cannot allocate an AVPacket for encoded video");
    }
    //init the resampler
    swrCtx = swr_alloc_set_opts(nullptr,
                                 av_get_default_channel_layout(outACCtx->channels),
                                 outACCtx->sample_fmt,
                                 outACCtx->sample_rate,
                                 av_get_default_channel_layout(inACCtx->channels),
                                 inACCtx->sample_fmt,
                                 inACCtx->sample_rate,
                                 0,
                                 nullptr);
    if (!swrCtx) {
        throw runtime_error("Cannot allocate the resample context");
    }
    if (swr_init(swrCtx) < 0) {
        throw runtime_error("Could not open resample context");
        swr_free(&swrCtx);
    }


    while (true) {
        unique_lock<mutex> ul(statusLock);
        cv.wait(ul, [this]() { return status != RecStatus::PAUSE; });
        if (status == RecStatus::STOP) {
            break;
        }
        ul.unlock();

        if (av_read_frame(inAFmtCtx, inPacket) >= 0 && inPacket->stream_index == inAIndex) {
            //decode audio routing
            av_packet_rescale_ts(outPacket, inAFmtCtx->streams[inAIndex]->time_base, inACCtx->time_base);

            if ((ret = avcodec_send_packet(inACCtx, inPacket)) < 0) {
                throw runtime_error("Cannot decode current audio packet ");
                continue;
            }
            while (ret >= 0) {
                ret = avcodec_receive_frame(inACCtx, rawFrame);

                if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                    break;
                else if (ret < 0) {
                    throw runtime_error("Error during decoding");
                }
//                if (avFmtCtxOut->streams[audioIndexOut]->start_time <= 0) {
//                    avFmtCtxOut->streams[audioIndexOut]->start_time = rawFrame->pts;
//                }
                if (!(resampledData = (uint8_t **)calloc(outACCtx->channels, sizeof(*resampledData)))) {
                    throw runtime_error("Could not allocate converted input sample pointers");
                }
                /* Allocate memory for the samples of all channels in one consecutive
              * block for convenience. */
                if (av_samples_alloc(resampledData, nullptr, outACCtx->channels, rawFrame->nb_samples, outACCtx->sample_fmt, 0) < 0) {
                    throw runtime_error("could not allocate memory for samples in all channels (audio)");
                }
                swr_convert(swrCtx,
                            resampledData, rawFrame->nb_samples,
                            (const uint8_t **)rawFrame->extended_data, rawFrame->nb_samples);

                /* Make the FIFO as large as it needs to be to hold both,
                * the old and the new samples. */
                if ((av_audio_fifo_realloc(audioFifo, av_audio_fifo_size(audioFifo) + rawFrame->nb_samples)) < 0) {
                    throw runtime_error("Could not reallocate FIFO");
                }
                /* Store the new samples in the FIFO buffer. */
                if (av_audio_fifo_write(audioFifo, (void **)resampledData, rawFrame->nb_samples) < rawFrame->nb_samples) {
                    throw runtime_error("Could not write data to FIFO");
                }

                //raw frame ready
                av_init_packet(outPacket);
                outPacket->data = nullptr;
                outPacket->size = 0;

                scaledFrame = av_frame_alloc();
                if (!scaledFrame) {
                    throw runtime_error("Cannot allocate an AVPacket for encoded audio");
                }

                scaledFrame->nb_samples = outACCtx->frame_size;
                scaledFrame->channel_layout = outACCtx->channel_layout;
                scaledFrame->format = outACCtx->sample_fmt;
                scaledFrame->sample_rate = outACCtx->sample_rate;
                av_frame_get_buffer(scaledFrame, 0);
                while (av_audio_fifo_size(audioFifo) >= outACCtx->frame_size) {
                    ret = av_audio_fifo_read(audioFifo, (void **)(scaledFrame->data), outACCtx->frame_size);
                    scaledFrame->pts = aPTS;
                    aPTS += scaledFrame->nb_samples;

                    if (avcodec_send_frame(outACCtx, scaledFrame) < 0) {
                        throw runtime_error("Cannot encode current audio packet ");
                    }

                    while (ret >= 0) {
                        ret = avcodec_receive_packet(outACCtx, outPacket);
                        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
                            break;
                        else if (ret < 0) {
                            throw runtime_error("Error during encoding");
                        }
                        av_packet_rescale_ts(outPacket, outACCtx->time_base, outFmtCtx->streams[OUT_AUDIO_INDEX]->time_base);
                        outPacket->stream_index = OUT_AUDIO_INDEX;

                        unique_lock<mutex> writeLock_ul{writeLock};

                        if (av_write_frame(outFmtCtx, outPacket) != 0) {
                            throw runtime_error("Error in writing audio frame");
                        }

                        writeLock_ul.unlock();
                        av_packet_unref(outPacket);
                    }
                    ret = 0;
                }

                av_frame_free(&scaledFrame);
                av_packet_unref(outPacket);
            }
        }
    }
}

void ScreenRecorder::PSRMenu() {
    unsigned short res;
    int cnt = 0;
    while(true) {
        unique_lock<mutex> ul(statusLock);
        if(status == RecStatus::STOP)
            break;

        if(cnt == 0)
            showPSROptions();

        ul.unlock();

        switch (getPSRAnswer()) {
            case 0:
                if (status != RecStatus::STOP){
                    cnt = 0;
                    stop_();
                }
                cnt++;
                break;
            case 1:
                if(status != RecStatus::PAUSE){
                    cnt = 0;
                    pause_();
                }else{
                    cnt++;
                    std::cout << "\033[0;33m" << "Already paused.\n" << "\033[0m" << ">> ";
                }
                break;
            case 2:
                if(status == RecStatus::PAUSE){
                    cnt = 0;
                    restart_();
                }
                else{
                    cnt++;
                    std::cout << "\033[0;33m" << "You must pause before resume.\n" << "\033[0m" << ">> ";
                }
                break;
            default:
                std::cout << "Command not recognized" << std::endl;
        }
    }
}

void ScreenRecorder::showPSROptions(){
    if(status != RecStatus::PAUSE) {
        std::cout << "Digit \"p\" or \"pause\" to pause the recording, \"s\" or \"stop\" to terminate\n"
                  << ">> ";
    }else{
        std::cout << "Digit \"r\" or \"restart\" to resume the recording, \"s\" or \"stop\" to terminate\n"
                  << ">> ";
    }
}

bool ScreenRecorder::validPSRAnswer(std::string &answer, int &res){
    std::transform(answer.begin(), answer.end(), answer.begin(), [](char c){return tolower(c);});

    bool valid_ans = false;
    if(answer == "s" || answer == "stop"){
        valid_ans = true;
        res = 0;
    }else if (answer == "p" || answer == "pause"){
        valid_ans = true;
        res = 1;
    }else if (answer == "r" || answer == "restart"){
        valid_ans = true;
        res = 2;
    }
    return valid_ans;
}

int ScreenRecorder::getPSRAnswer(){
    std::string user_answer;
    int res = -1;

    while(std::cin >> user_answer && (!validPSRAnswer(user_answer, res))){
        std::cout << "\033[1;31m" << "Invalid answer: \"" << user_answer << "\". Try again.\n" << "\033[0m" << ">> ";
    }
    if(!std::cin){
        throw std::runtime_error("Failed to read user input");
    }

    return res;
}

void ScreenRecorder::getFilenameOut(std::string& str){
    std::string filename;

    std::cout << "Insert output filename:\n>>  ";
    cin.ignore();
    std::getline(std::cin, filename);
    std::replace(filename.begin(), filename.end(), ' ', '_');
    str = "../../media/" + filename + ".mp4";
}

string ScreenRecorder::getRecState(){
    string state;
    if(this->status == RecStatus::STARTED){
        state = "STARTED";
    }else if(this->status == RecStatus::RECORDING){
        state = "RECORDING";
    }else if(this->status == RecStatus::PAUSE){
        state = "PAUSE";
    }else if(this->status == RecStatus::STOP){
        state = "STOP";
    }
    return state;
}
string ScreenRecorder::getVideoInfo(){
    string videoInfo;
    lock_guard<mutex> ul(statusLock);
    if(this->status != RecStatus::STARTED){
        videoInfo = "Video info:\n- width: " +
                    to_string(this->outVCCtx->width) + "\n- height: " +
                    to_string(this->outVCCtx->height) + "\n- fps: " +
                    to_string(this->outVCCtx->framerate.num);
    }else{
        videoInfo = "No information available. Recording not started yet.";
    }
    return videoInfo;
}
string ScreenRecorder::getAudioInfo(){
    string audioInfo;
    lock_guard<mutex> ul(statusLock);
    if(rec_type == Command::avsp || rec_type == Command::avfs) {
        if (this->status != RecStatus::STARTED) {
            audioInfo = "Audio info:\n- sample rate: " +
                        to_string(this->outACCtx->sample_rate) + "\n- bit rate: " +
                        to_string(this->outACCtx->bit_rate) + "\n- channels: " +
                        to_string(this->outACCtx->channels);
        } else {
            audioInfo = "No information available. Recording not started yet.";
        }
    }else{
        audioInfo = "No information available. This recording contains only video part.";
    }
    return audioInfo;
}

/*
static inline void ltrim(std::string &s) {
    s.erase(s.begin(), std::find_if(s.begin(), s.end(), [](unsigned char ch) {
        return !std::isspace(ch);
    }));
}

static inline void rtrim(std::string &s) {
    s.erase(std::find_if(s.rbegin(), s.rend(), [](unsigned char ch) {
        return !std::isspace(ch);
    }).base(), s.end());
}

static inline void trim(std::string &s) {
    ltrim(s);
    rtrim(s);
}

bool ScreenRecorder::setCropParameters() {
    Display* display = XOpenDisplay(NULL);
    Screen* screen = DefaultScreenOfDisplay(display);
    string line, s;
    vector<string> strings;

    cout << "Your screen dimensions are " + to_string(screen->width) + "x" + to_string(screen->height) << endl;
    cout << "Insert video crop values following the pattern: x, y, width, height\t>> ";
    cin.ignore();
    getline(cin, line);

    istringstream f(line);
    while (getline(f, s, ',')) {
        trim(s);
        strings.push_back(s);
    }

    cropX = stoi(strings[0]);
    cropY = stoi(strings[1]);
    cropWidth = stoi(strings[2]);
    cropHeight = stoi(strings[3]);

    if((cropX+cropWidth) > screen->width || (cropY+cropHeight) > screen->height){
        std::cout << "\033[0;31m" << "Error: you exceeded screen dimensions. Retry." << "\033[0m" << std::endl;
        return false;
    }else {
        return true;
    }
}
*/
