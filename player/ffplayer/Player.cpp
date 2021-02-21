#include "stdafx.h"

#include "Audio.h"

using namespace std;

void Player::run(std::string video_addr, std::string window_name)
{
	this->video_addr = video_addr;
	this->window_name = window_name;

	this->open();
	this->malloc();
	this->create_display();
}


// »Øµ÷º¯Êý
static int interrupt_callback(void* p) {
	int32_t nt = time(0);
	int32_t ntbase = (int32_t)p;
	if (nt - ntbase > 5)
	{
		return 1;
	}
    return 0;
}

void Player::open()
{
	// open video
	int res = 0xabb6a7bb;

	while (res == 0xabb6a7bb)
	{
		if (!pFormatCtx)
		{
            pFormatCtx = avformat_alloc_context();
            pFormatCtx->interrupt_callback.callback = interrupt_callback;
			pFormatCtx->probesize = 10 * 1024 * 1024;
		}
		pFormatCtx->interrupt_callback.opaque = (void*)time(0);
		res = avformat_open_input(&pFormatCtx, this->video_addr.c_str(), NULL, NULL);
	}

	// check video
	if (res != 0)
		Utils::display_ffmpeg_exception(res);

	// get video info
	res = avformat_find_stream_info(pFormatCtx, NULL);
	if (res < 0)
		Utils::display_ffmpeg_exception(res);

	// get video stream
	m_videoStream = get_video_stream();
	if (m_videoStream == -1)
		Utils::display_exception("Error opening your video using AVCodecParameters, probably doesnt have codecpar_type type AVMEDIA_TYPE_VIDEO");

	// open
	read_audio_video_codec();
}

void Player::clear()
{
	_playStatus = 0;

	if (_threadPlay)
	{
		_threadPlay->join();
	}

	// close context info
	avformat_close_input(&pFormatCtx);
	avcodec_free_context(&pCodecCtx);

	// free buffers
	av_free(buffer);
	av_free(pFrameRGB);

	// Free the YUV frame
	av_free(pFrame);

	// Close the codecs
	avcodec_close(pCodecCtx);

	// Close the video file
	avformat_close_input(&pFormatCtx);
}

/*
Acquires video stream
*/
int Player::get_video_stream(void)
{
	for (unsigned int i = 0; i<pFormatCtx->nb_streams; i++){
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) m_videoStream = i;
		if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) m_audioStream = i;
	}

	if (m_videoStream == -1)
		Utils::display_exception("Couldnt find stream");
	pCodecParameters = pFormatCtx->streams[m_videoStream]->codecpar;

	if(m_audioStream != -1)
		pCodecAudioParameters = pFormatCtx->streams[m_audioStream]->codecpar;

	return m_videoStream;
}

/*
Reads audio and video codec
*/
int Player::read_audio_video_codec(void) 
{
	pCodec = avcodec_find_decoder(pCodecParameters->codec_id);
	pAudioCodec = avcodec_find_decoder(pCodecAudioParameters->codec_id);

	if (pCodec == NULL)
		Utils::display_exception("Video decoder not found");

	if (pAudioCodec == NULL) 
		Utils::display_exception("Audio decoder not found");

	pCodecCtx = avcodec_alloc_context3(pCodec);

	if(pCodecCtx == NULL)
		Utils::display_exception("Failed to allocate video context decoder");

	pCodecAudioCtx = avcodec_alloc_context3(pAudioCodec);

	if(pCodecAudioCtx == NULL)
		Utils::display_exception("Failed to allocate audio context decoder");

	int res = avcodec_parameters_to_context(pCodecCtx, pCodecParameters);

	if(res < 0)
		Utils::display_exception("Failed to transfer video parameters to context");

	res = avcodec_parameters_to_context(pCodecAudioCtx, pCodecAudioParameters);

	if (res < 0) 
		Utils::display_exception("Failed to transfer audio parameters to context");

	res = avcodec_open2(pCodecCtx, pCodec, NULL);

	if(res < 0)
		Utils::display_exception("Failed to open video codec");

	res = avcodec_open2(pCodecAudioCtx, pAudioCodec, NULL);

	if (res < 0)
		Utils::display_exception("Failed to open auvio codec");

	return 1;
}

/*
Alloc memory for the display
*/
int Player::malloc(void)
{
	_pAudio->malloc(pCodecAudioCtx);

	_pAudio->open();

	pFrame = av_frame_alloc();
	if (pFrame == NULL)
		Utils::display_exception("Couldnt allocate frame memory");

	pFrameRGB = av_frame_alloc();
	if (pFrameRGB == NULL)
		Utils::display_exception("Couldnt allocate rgb frame memory");

	int numBytes = av_image_get_buffer_size(VIDEO_FORMAT, pCodecCtx->width, pCodecCtx->height,1);

	buffer = (uint8_t *)av_malloc(numBytes*sizeof(uint8_t));

	int res = av_image_fill_arrays(pFrameRGB->data, pFrameRGB->linesize, buffer, VIDEO_FORMAT, pCodecCtx->width, pCodecCtx->height, 1);
	if (res < 0)
		Utils::display_ffmpeg_exception(res);
	
	return 1;
}


int Player::getAudioPacket(AudioPacket* q, AVPacket* pkt, int block){

	AVPacketList* pktl;
    int ret;

    SDL_LockMutex(q->mutex);

    while (1)
    {
        pktl = q->first;
        if (pktl)
        {
            q->first = pktl->next;
            if (!q->first)
                q->last = NULL;

            q->nb_packets--;
            q->size -= pktl->pkt.size;

            *pkt = pktl->pkt;
            av_free(pktl);
            ret = 1;
            break;
        }
        else if (!block)
        {
            ret = 0;
            break;
        }
        else
        {
            SDL_CondWait(q->cond, q->mutex);
        }
    }

    SDL_UnlockMutex(q->mutex);

    return ret;
}

ffresurlt Player::Init()
{
    SDLWrapper::init_sdl();

	if (!_pAudio)
	{
		_pAudio = new Audio(this);
	}

	return 0;
}

ffresurlt Player::UnInit()
{
    if (_pAudio)
    {
		delete _pAudio;
		_pAudio = nullptr;
    }
	return 0;
}

ffresurlt Player::SetFile(const char* sfile)
{
    run(sfile);

	return 0;
}

ffresurlt Player::Play()
{
	this->display_video();

	return 0;
}

ffresurlt Player::Stop()
{
    clear();

	return 0;
}

ffresurlt Player::SetPosition(int32_t, int32_t, int32_t, int32_t)
{
	return 0;
}

extern void ffplaythread(AVFormatContext* ic);

void Player::_PlayThread2() {
	//ffplaythread(pFormatCtx);



   

}

template <typename F>
int DecodePacket(AVCodecContext* context_, AVFrame* frame_, const AVPacket* packet, F const& f) {

    bool sent_packet = false, frames_remaining = true, decoder_error = false;
	int result = 0;
    while (!sent_packet || frames_remaining) {

        if (!sent_packet) {

            result = avcodec_send_packet(context_, packet);

            if (result < 0 && result != AVERROR(EAGAIN) && result != AVERROR_EOF) {
                return result;
            }

            sent_packet = result != AVERROR(EAGAIN);
        }

        // See if any frames are available. If we receive an EOF or EAGAIN, there
        // should be nothing left to do this pass since we've already provided the
        // only input packet that we have.

        result = avcodec_receive_frame(context_, frame_);
        if (result == AVERROR_EOF || result == AVERROR(EAGAIN)) {

            frames_remaining = false;
            if (result == AVERROR(EAGAIN)) {
				assert(sent_packet);
				sent_packet = false;
            }
         //   continue;
        }
        else if (result < 0) {

            decoder_error = true;
            continue;
        }

        const bool frame_processing_success = f(frame_);
        av_frame_unref(frame_);

        if (!frame_processing_success)
            return -110;
    }

    return decoder_error ? result : 0;

}

void Player::_LoopDecoder()
{
	SDL_Event evt;
	AVPacket packet;

    do {

        auto result = av_read_frame(pFormatCtx, &packet);

        if (result < 0)
            break;

        if (packet.stream_index == m_audioStream) {
            _pAudio->put_audio_packet(&packet);
        }
        else
		if (packet.stream_index == m_videoStream)
		{
			result = DecodePacket(pCodecCtx, pFrame, &packet, [this](auto & pFrame) {

                SDL_UpdateYUVTexture(bmp, NULL, pFrame->data[0], pFrame->linesize[0],
                    pFrame->data[1], pFrame->linesize[1],
                    pFrame->data[2], pFrame->linesize[2]);
                SDL_RenderCopy(renderer, bmp, NULL, NULL);
                SDL_RenderPresent(renderer);
                SDL_UpdateWindowSurface(screen);
                if (_playStatus)
                {
                    SDL_Delay(1000 / 30);
                }
                else
                {
					return 0;
                }
				return 1;
			});
		}

		SDL_PollEvent(&evt);
        av_packet_unref(&packet);

		if (result == -110)
		{
			break;
		}
		else
		{
			if (result < 0)
			{
				Utils::display_ffmpeg_exception(result);
			}
		}

    } while (true);

}


void Player::_PlayThread()
{
    AVPacket packet;

    SDL_Event evt;
	int nframecnt = 0;
    while (av_read_frame(pFormatCtx, &packet) >= 0) {

        if (packet.stream_index == m_audioStream) {
            _pAudio->put_audio_packet(&packet);
        }
		else
        if (packet.stream_index == m_videoStream)
        {
			nframecnt++;
			int res = 0;

            while (1) {
				res = avcodec_send_packet(pCodecCtx, &packet);
                if (res == 0) {
                    break;
                }
                else if (res == AVERROR(EAGAIN)) {
                    break;
                }
                else {
					Utils::display_ffmpeg_exception(res);
                    av_packet_unref(&packet);
                   // goto quit;
                }
            }
              
            //res = avcodec_receive_frame(pCodecCtx, pFrame);
			res = avcodec_receive_frame(pCodecCtx, pFrame);
            if (res == AVERROR(EAGAIN)) {
                continue;
				//goto read_another_frame;
                /* If the end of the input file is reached, stop decoding. */
            }
            else if (res == AVERROR_EOF) {
                break;
            }
            else if (res < 0) {
				Utils::display_ffmpeg_exception(res);
                break;
            }
            // Default case: encode data
            else {

            }


            SDL_UpdateYUVTexture(bmp, NULL, pFrame->data[0], pFrame->linesize[0],
                pFrame->data[1], pFrame->linesize[1],
                pFrame->data[2], pFrame->linesize[2]);
            SDL_RenderCopy(renderer, bmp, NULL, NULL);
            SDL_RenderPresent(renderer);
            SDL_UpdateWindowSurface(screen);
			if (_playStatus)
			{
				SDL_Delay(1000 / 30);
			}
			else
			{
				break;
			}
        }
		else
		{
			int n = 0;
			n++;
		}

        SDL_PollEvent(&evt);

        av_packet_unref(&packet);
    }
}

/*
Read frames and display
*/
int Player::display_video(void) {
	//video context
	sws_ctx = sws_getContext(pCodecCtx->width,
		pCodecCtx->height,
		pCodecCtx->pix_fmt,
		pCodecCtx->width,
		pCodecCtx->height,
		VIDEO_FORMAT,
		SWS_BILINEAR,
		NULL,
		NULL,
		NULL
		);

	_playStatus = 1;

	if (!_threadPlay)
	{
		//_threadPlay = std::make_unique<std::thread>(&Player::_LoopDecoder, this);
		_threadPlay = std::make_unique<std::thread>(&Player::_PlayThread, this);
	}

	return 1;

}

/*
Create the display for the received video
*/
int Player::create_display(void) 
{
	screen = SDL_CreateWindow(window_name.c_str(),
			SDL_WINDOWPOS_CENTERED,
			SDL_WINDOWPOS_CENTERED,
			pCodecCtx->width, pCodecCtx->height,
			SDL_WINDOW_OPENGL);
	
	if (!screen)
		Utils::display_exception("Couldn't show display window");

	renderer = SDL_CreateRenderer(screen, -1, 0);

	bmp = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_YV12, SDL_TEXTUREACCESS_STATIC, pCodecCtx->width, pCodecCtx->height);

	return 1;
}