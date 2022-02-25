#include <ScreenRecorder.h>

int main(int argc, char **argv) {

    try {
        ScreenRecorder sr;

        sr.open_();
        sr.start_();
    } catch (std::exception &e) {
        std::cerr << "Error occurred : " <<  e.what() << std::endl;
        exit(-1);
    }

    return 0;
}