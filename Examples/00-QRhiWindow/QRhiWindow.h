#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>
#include <QOffscreenSurface>
#include "private\qrhi_p.h"

class QRhiWindow;
class QRhiRenderer : public QObject {
	friend class QRhiWindow;
public:
	struct FrameContext {
		QSize viewport;
		QRect overlayRect;
	};
public:
	virtual void setWindow(QRhiWindow* window) { window_ = window; }
protected:
	virtual void initResources() {}
	virtual void releaseResources() {}
	virtual void initSwapChainResources() {}
	virtual void releaseSwapChainResources() {}
	virtual void startNextFrame(FrameContext frameCtx) = 0;
protected:
	QRhiWindow* window_ = nullptr;
};

class QRhiWindow :public QWindow {
public:
	QRhiWindow(QRhi::Implementation backend);
	static void setDefaultSurfaceFormat(QSurfaceFormat format);
private:
	void initInternal();
	void renderInternal();
	void resizeSwapChain();
	void releaseSwapChain();
protected:
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
public:
	QRhi::Implementation mBackend;
	std::unique_ptr<QRhi> mRhi;
	std::unique_ptr<QRhiSwapChain> mSwapChain;
	std::unique_ptr<QRhiRenderBuffer> mDepthStencilFrameBuffer;
	std::unique_ptr<QRhiRenderPassDescriptor> mRenderPassDesciptor;
	std::unique_ptr<QOffscreenSurface> mFallbackSurface;
	std::shared_ptr<QRhiRenderer>  mRootRenderer;
protected:
	bool m_running = false;
	bool m_notExposed = false;
	bool m_newlyExposed = false;
	bool m_hasSwapChain = false;
};

#endif // QRhiWindow_h__
