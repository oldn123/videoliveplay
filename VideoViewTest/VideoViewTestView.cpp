
// VideoViewTestView.cpp: CVideoViewTestView 类的实现
//

#include "pch.h"
#include "framework.h"
// SHARED_HANDLERS 可以在实现预览、缩略图和搜索筛选器句柄的
// ATL 项目中进行定义，并允许与该项目共享文档代码。
#ifndef SHARED_HANDLERS
#include "VideoViewTest.h"
#endif

#include "VideoViewTestDoc.h"
#include "VideoViewTestView.h"


#include <filesystem>
#include <thread>
#include <chrono>
#include <string>
#include <iosfwd>
#include <sstream> 


#include "..\Include\ffplayer.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CVideoViewTestView

IMPLEMENT_DYNCREATE(CVideoViewTestView, CView)

BEGIN_MESSAGE_MAP(CVideoViewTestView, CView)
	// 标准打印命令
	ON_COMMAND(ID_FILE_PRINT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_DIRECT, &CView::OnFilePrint)
	ON_COMMAND(ID_FILE_PRINT_PREVIEW, &CVideoViewTestView::OnFilePrintPreview)
	ON_WM_CONTEXTMENU()
	ON_WM_RBUTTONUP()
END_MESSAGE_MAP()

// CVideoViewTestView 构造/析构

CVideoViewTestView::CVideoViewTestView() noexcept
{
	// TODO: 在此处添加构造代码

}

CVideoViewTestView::~CVideoViewTestView()
{
}

BOOL CVideoViewTestView::PreCreateWindow(CREATESTRUCT& cs)
{
	// TODO: 在此处通过修改
	//  CREATESTRUCT cs 来修改窗口类或样式

	return CView::PreCreateWindow(cs);
}

// CVideoViewTestView 绘图

void CVideoViewTestView::OnDraw(CDC* /*pDC*/)
{
	CVideoViewTestDoc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	// TODO: 在此处为本机数据添加绘制代码
}


// CVideoViewTestView 打印


void CVideoViewTestView::OnFilePrintPreview()
{
#ifndef SHARED_HANDLERS
	AFXPrintPreview(this);
#endif
}

BOOL CVideoViewTestView::OnPreparePrinting(CPrintInfo* pInfo)
{
	// 默认准备
	return DoPreparePrinting(pInfo);
}

void CVideoViewTestView::OnBeginPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加额外的打印前进行的初始化过程
}

void CVideoViewTestView::OnEndPrinting(CDC* /*pDC*/, CPrintInfo* /*pInfo*/)
{
	// TODO: 添加打印后进行的清理过程
}

void CVideoViewTestView::OnRButtonUp(UINT /* nFlags */, CPoint point)
{
	ClientToScreen(&point);
	OnContextMenu(this, point);
}

void CVideoViewTestView::OnContextMenu(CWnd* /* pWnd */, CPoint point)
{
#ifndef SHARED_HANDLERS
	theApp.GetContextMenuManager()->ShowPopupMenu(IDR_POPUP_EDIT, point.x, point.y, this, TRUE);
#endif
}


// CVideoViewTestView 诊断

#ifdef _DEBUG
void CVideoViewTestView::AssertValid() const
{
	CView::AssertValid();
}

void CVideoViewTestView::Dump(CDumpContext& dc) const
{
	CView::Dump(dc);
}

CVideoViewTestDoc* CVideoViewTestView::GetDocument() const // 非调试版本是内联的
{
	ASSERT(m_pDocument->IsKindOf(RUNTIME_CLASS(CVideoViewTestDoc)));
	return (CVideoViewTestDoc*)m_pDocument;
}
#endif //_DEBUG


// CVideoViewTestView 消息处理程序


void _CaptureDecodeCallbackFunc_t(LPVOID pUser, BYTE* pBuffer, long lBufferSize, long lWidth, long lHeight, long lBitCnt)
{

}



typedef int32_t(WINAPI* _CreateObject)(const GUID& guid, void** ppInterface);

IRtspPlayer* pPlayer = nullptr;

void PlayThread(HWND hW)
{
    wchar_t buf[MAX_PATH] = { 0 };
    GetModuleFileName(NULL, buf, MAX_PATH);
    std::filesystem::path ph(buf);
    auto pt = ph.remove_filename();
    pt += L"ffplayer.dll";
    auto l = LoadLibraryEx(pt.wstring().c_str(), 0, LOAD_WITH_ALTERED_SEARCH_PATH);
    if (l)
    {
        _CreateObject pCreateObj = (_CreateObject)GetProcAddress(l, "CreateObject");
        if (pCreateObj)
        {
            pCreateObj(IDL_IPLAYER, (void**)&pPlayer);
            if (!pPlayer)
            {
                return;
            }
        }
    }

    if (pPlayer)
    {
        pPlayer->Init();
        //pPlayer->SetFile("rtsp://123.57.41.232/live");
        pPlayer->SetFile("D:\\Downloads\\H.264 interlaced_cut.mp4");
        pPlayer->SetBindWindow(hW);
        pPlayer->Play();
    }

    if (!hW)
    {
        MSG msg = {};
        while (GetMessage(&msg, NULL, 0, 0))//Windows消息循环。
        {
            TranslateMessage(&msg);//翻译消息，如按键消息，翻译为WM_CHAR
            DispatchMessage(&msg);//分发消息到对应窗口
        }
    }
}

void OnAppIdle() {
    if (pPlayer)
    {
        pPlayer->OnBindWindowMsgIdle();
    }
}

void CVideoViewTestView::OnInitialUpdate()
{
	CView::OnInitialUpdate();

	// TODO: 在此添加专用代码和/或调用基类



    PlayThread(GetSafeHwnd());









// 
// 	int n = CameraCatchMgr::Instance().GetCameraCount();
// 	if (n > 0)
// 	{
// 		auto pCap = CameraCatchMgr::Instance().GetCaptureByIndex(0);
// 		if (pCap)
// 		{
// 			pCap->SetBindWnd(GetSafeHwnd());
// 
//             pCap->Init(0);
// 
// 			pCap->SetCallbackFunc(_CaptureDecodeCallbackFunc_t, this);
// 
// 			BITMAPINFOHEADER bh = { 0 };
// 			pCap->GetVideoInfoHeader(bh);
// 
// 			pCap->Preview();
// 			pCap->Play();
//            
// 
// 
// 		}
// 	}
}
