#ifndef VIDEORENDERER_H
#define VIDEORENDERER_H

#include <stdint.h>
#if defined(__APPLE__)
typedef void* HWND;
#endif


class IRenderer
{
public:
	virtual ~IRenderer() {};

	/*@desc
	*@param init with hwnd
	*/
	virtual bool init(void * hwnd) = 0;
	virtual void clean() = 0;
	virtual void loadYuv(const uint8_t* Y, const uint8_t* U, const uint8_t* V, int width, int height) = 0;
    virtual int  loadYuv(int width, int height,
		const uint8_t* Yplane, int Ypitch,
        const uint8_t* Uplane, int Upitch,
        const uint8_t* Vplane, int Vpitch) = 0;
	virtual void setMirror(bool useMirror) = 0;
};

enum RENDERER_TYPE {
	RENDERER_OPENGL,
	RENDERER_SDL
};
class RendererFactory 
{
public:
	static IRenderer* create(RENDERER_TYPE type = RENDERER_SDL);
};

#endif
