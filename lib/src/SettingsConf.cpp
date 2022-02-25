#include "../include/SettingsConf.h"

void SettingsConf::welcomeMsg(){
    std::cout << "\033[1;34m" << R"(
$$$$$$$\  $$$$$$$\   $$$$$$\         $$$$$$\   $$$$$$\  $$$$$$$\  $$$$$$$$\ $$$$$$$$\ $$\   $$\       $$$$$$$\  $$$$$$$$\  $$$$$$\   $$$$$$\  $$$$$$$\  $$$$$$$\  $$$$$$$$\ $$$$$$$\
$$  __$$\ $$  __$$\ $$  __$$\       $$  __$$\ $$  __$$\ $$  __$$\ $$  _____|$$  _____|$$$\  $$ |      $$  __$$\ $$  _____|$$  __$$\ $$  __$$\ $$  __$$\ $$  __$$\ $$  _____|$$  __$$\
$$ |  $$ |$$ |  $$ |$$ /  \__|      $$ /  \__|$$ /  \__|$$ |  $$ |$$ |      $$ |      $$$$\ $$ |      $$ |  $$ |$$ |      $$ /  \__|$$ /  $$ |$$ |  $$ |$$ |  $$ |$$ |      $$ |  $$ |
$$$$$$$  |$$ |  $$ |\$$$$$$\        \$$$$$$\  $$ |      $$$$$$$  |$$$$$\    $$$$$\    $$ $$\$$ |      $$$$$$$  |$$$$$\    $$ |      $$ |  $$ |$$$$$$$  |$$ |  $$ |$$$$$\    $$$$$$$  |
$$  ____/ $$ |  $$ | \____$$\        \____$$\ $$ |      $$  __$$< $$  __|   $$  __|   $$ \$$$$ |      $$  __$$< $$  __|   $$ |      $$ |  $$ |$$  __$$< $$ |  $$ |$$  __|   $$  __$$<
$$ |      $$ |  $$ |$$\   $$ |      $$\   $$ |$$ |  $$\ $$ |  $$ |$$ |      $$ |      $$ |\$$$ |      $$ |  $$ |$$ |      $$ |  $$\ $$ |  $$ |$$ |  $$ |$$ |  $$ |$$ |      $$ |  $$ |
$$ |      $$$$$$$  |\$$$$$$  |      \$$$$$$  |\$$$$$$  |$$ |  $$ |$$$$$$$$\ $$$$$$$$\ $$ | \$$ |      $$ |  $$ |$$$$$$$$\ \$$$$$$  | $$$$$$  |$$ |  $$ |$$$$$$$  |$$$$$$$$\ $$ |  $$ |
\__|      \_______/  \______/        \______/  \______/ \__|  \__|\________|\________|\__|  \__|      \__|  \__|\________| \______/  \______/ \__|  \__|\_______/ \________|\__|  \__|

     ____                       ___                      __
    / __/__________ ___ ___    / _ \___ _______  _______/ /__ ____
   _\ \/ __/ __/ -_) -_) _ \  / , _/ -_) __/ _ \/ __/ _  / -_) __/
  /___/\__/_/  \__/\__/_//_/ /_/|_|\__/\__/\___/_/  \_,_/\__/_/
    )" << "\033[0m" << std::endl;
}

Command SettingsConf::optionsMenu(){
    unsigned short res;
    while(true){
        showAudioVideoOptions();
        res = getAnswer();
        switch(res){
            case 0:
                if(yesNoQuestion("Are you sure you want to quit?")){
                    std::cout << "\033[0;31m" << "\nClosing screen recorder..\n" << "\033[0m";
                    return Command::stop;
                }
                break;
            case 1:
                showScreenOptions();
                res = getAnswer();
                switch(res){
                    case 0:
                        if(yesNoQuestion("Are you sure you want to quit?")){
                            std::cout << "\033[0;31m" << "\nClosing screen recorder..\n" << "\033[0m";
                            return Command::stop;
                        }
                        break;
                    case 1:
                        return Command::vofs;
                    case 2:
                        return Command::vosp;
                    default:
                        std::cout << "Command not recognized" << std::endl;
                }
                break;
            case 2:
                showScreenOptions();
                res = getAnswer();
                switch(res){
                    case 0:
                        if(yesNoQuestion("Are you sure you want to quit?")){
                            std::cout << "\033[0;31m" << "\nClosing screen recorder..\n" << "\033[0m";
                            return Command::stop;
                        }
                        break;
                    case 1:
                        return Command::avfs;
                    case 2:
                        return Command::avsp;
                    default:
                        std::cout << "Command not recognized" << std::endl;
                }
                break;
            default:
                std::cout << "Command not recognized" << std::endl;
        }
    }
}

void SettingsConf::showAudioVideoOptions(){
    std::cout << "Which type of recording do you want to perform?\n"
              << "\t1. Record only video\n"
              << "\t2. Record both audio and video\n"
              << "\tq. Exit\n"
              << ">> ";
}

void SettingsConf::showScreenOptions(){
    std::cout << "Which portion of the screen do you want to record?\n"
              << "\t1. Record fullscreen\n"
              << "\t2. Record only a portion of the screen\n"
              << "\tq. Exit\n"
              << ">> ";
}

bool SettingsConf::validAnswer(std::string &answer){
    std::transform(answer.begin(), answer.end(), answer.begin(), [](char c){return tolower(c);});

    bool valid_ans = (answer == "q") || (answer == "quit") || (answer == "exit") ||
                     (answer == "s") || (answer == "stop");

    return valid_ans;
}

bool SettingsConf::validAnswer(std::string &answer, bool &res){
    std::transform(answer.begin(), answer.end(), answer.begin(), [](char c){return tolower(c);});

    bool valid_ans = (answer == "y") || (answer == "yes") ||
                     (answer == "n") || (answer == "no");
    res = valid_ans && (answer[0] == 'y');
    return valid_ans;
}

bool SettingsConf::yesNoQuestion(std::string const &message){
    std::string user_answer;
    bool res;

    std::cout << message << " [y/n] ";
    while(std::cin >> user_answer && !validAnswer(user_answer, res)){
        std::cout << "\033[1;31m" << "Invalid answer, retry.\n" << "\033[0m" << message << " [y/n] ";
    }

    if(!std::cin){
        //throw std::runtime_error("Failed to read user input");
    }

    return res; //true if "yes", false if "no"
}

int SettingsConf::getAnswer(){
    std::string user_answer;

    while(std::cin >> user_answer && (!validAnswer(user_answer) && (user_answer!="1" && user_answer!="2"))){
        std::cout << "\033[1;31m" << "Invalid answer: \"" << user_answer << "\". Try again.\n" << "\033[0m" << ">> ";
    }
    if(!std::cin){
        //throw std::runtime_error("Failed to read user input");
    }
    if(std::isdigit(user_answer[0])) return std::stoi(user_answer);
    return 0;
};