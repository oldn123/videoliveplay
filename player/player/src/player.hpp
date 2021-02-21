
#include <string>
using namespace std;


extern void playAudio(const string& inputPath);

extern void playVideo(const string& inputPath);

extern void play(const string& inputPath);

int domain() {

    string inputPath = "rtsp://123.57.41.232:554/live";//"rtsp://123.57.41.232/live";
    play(inputPath);
//    playVideo(inputPath);
    return 0;
};



