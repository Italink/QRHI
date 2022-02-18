#ifndef QRhiWindow_h__
#define QRhiWindow_h__

#include <QWindow>
#include "private\qrhi_p.h"
#include <QOffscreenSurface>

class QRhiWindow :public QWindow {
public:
	QRhiWindow(QRhi::Implementation backend);
	static void setDefaultSurfaceFormat(QSurfaceFormat format);
protected:
	virtual void init() {}
	virtual void render() {}
private:
	void initInternal();
	void renderInternal();
	void resizeSwapChain();
	void releaseSwapChain();
protected:
	void exposeEvent(QExposeEvent*) override;
	bool event(QEvent*) override;
protected:
	std::unique_ptr<QRhi> mRhi;
	std::unique_ptr<QRhiSwapChain> mSwapChain;
	std::unique_ptr<QRhiRenderBuffer> mRenderBuffer;
	std::unique_ptr<QRhiRenderPassDescriptor> mRenderPassDesciptor;
	std::unique_ptr<QOffscreenSurface> m_fallbackSurface;
	QRhi::Implementation mBackend;
protected:
	bool m_running = false;
	bool m_notExposed = false;
	bool m_newlyExposed = false;
	bool m_hasSwapChain = false;
};

#endif // QRhiWindow_h__
