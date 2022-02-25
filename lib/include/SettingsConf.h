#ifndef SCREEN_RECORDER_SETTINGSCONF_H
#define SCREEN_RECORDER_SETTINGSCONF_H

#include <iostream>
#include <algorithm>

#include "ScreenRecorder.h"
/*
 * vofs = video only, fullscreen
 * avfs = audio and video, fullscreen
 * vosp = video only, screen portion
 * avsp = audio and video, screen portion
 */
enum Command{
    stop, vofs, vosp, avfs, avsp
};

class SettingsConf {
public:
    Command optionsMenu();
    void welcomeMsg();

private:
    void showAudioVideoOptions();
    void showScreenOptions();
    static int getAnswer();
    static bool validAnswer(std::string &answer);
    static bool validAnswer(std::string &answer, bool &res);
    static bool yesNoQuestion(std::string const &message);
};



#endif //SCREEN_RECORDER_SETTINGSCONF_H

