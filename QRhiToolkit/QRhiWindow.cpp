#include "QRhiWindow.h"
#include "QVulkanInstance"
#include <QtGui/private/qrhinull_p.h>
#include "private\qrhid3d11_p.h"
#include "private\qrhivulkan_p.h"
#include "private\qrhimetal_p.h"
#include "private\qrhigles2_p.h"
#include <QPlatformSurfaceEvent>

QRhiWindow::QRhiWindow(QRhi::Implementation backend)
	:mBackend(backend)
{
	switch (backend) {
	case QRhi::OpenGLES2:
		setSurfaceType(OpenGLSurface);
		setFormat(QRhiGles2InitParams::adjustedFormat());
		break;
	case QRhi::Vulkan:
		setSurfaceType(VulkanSurface);
		break;
	case QRhi::D3D11:
		setSurfaceType(Direct3DSurface);
		break;
	case QRhi::Metal:
		setSurfaceType(MetalSurface);
		break;
	default:
		break;
	}
}

void QRhiWindow::setDefaultSurfaceFormat(QSurfaceFormat format)
{
	QSurfaceFormat::setDefaultFormat(format);
}

//void QRhiWindow::exec()
//{
//	LARGE_INTEGER start_counter, end_counter, counts, frequency;
//	int fps;
//	float ms;
//
//	while (true) {
//		QueryPerformanceCounter(&start_counter);
//		QueryPerformanceFrequency(&frequency);
//
//		QApplication::eventDispatcher()->processEvents(QEventLoop::ProcessEventsFlag::ExcludeUserInputEvents);
//		renderInternal();
//		requestUpdate();
//		QueryPerformanceCounter(&end_counter);
//		counts.QuadPart = end_counter.QuadPart - start_counter.QuadPart;
//		start_counter = end_counter;
//		fps = frequency.QuadPart / counts.QuadPart;
//		ms = ((1000.0 * counts.QuadPart) / frequency.QuadPart);
//		qDebug() << "FPS:" << fps << "  Cost(ms):" << ms;
//	}
//}

void QRhiWindow::initInternal()
{
	QRhi::Flags rhiFlags = QRhi::EnableDebugMarkers | QRhi::EnableProfiling;

	if (mBackend == QRhi::Null) {
		QRhiNullInitParams params;
		mRhi.reset(QRhi::create(QRhi::Null, &params, rhiFlags));
	}
	else if (mBackend == QRhi::OpenGLES2) {
		m_fallbackSurface.reset(QRhiGles2InitParams::newFallbackSurface());
		QRhiGles2InitParams params;
		params.fallbackSurface = m_fallbackSurface.get();
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::OpenGLES2, &params, rhiFlags));
	}
	else if (mBackend == QRhi::Vulkan) {
		QRhiVulkanInitParams params;
		static QVulkanInstance vkinstance;
		if (!vkinstance.isValid()) {
			vkinstance.setLayers({ "VK_LAYER_KHRONOS_validation" });
			vkinstance.setExtensions(QRhiVulkanInitParams::preferredInstanceExtensions());
			if (!vkinstance.create())
				qFatal("Failed to create Vulkan instance");
		}
		setVulkanInstance(&vkinstance);
		params.inst = vulkanInstance();
		params.window = this;
		mRhi.reset(QRhi::create(QRhi::Vulkan, &params, rhiFlags));
	}
	else  if (mBackend == QRhi::D3D11) {
		QRhiD3D11InitParams params;
		params.enableDebugLayer = true;
		mRhi.reset(QRhi::create(QRhi::D3D11, &params, rhiFlags));
	}
	else if (mBackend == QRhi::Metal) {
		QRhiMetalInitParams params;
		mRhi.reset(QRhi::create(QRhi::Metal, &params, rhiFlags));
	}

	mSwapChain.reset(mRhi->newSwapChain());
	mRenderBuffer.reset(mRhi->newRenderBuffer(QRhiRenderBuffer::DepthStencil,
						QSize(), // no need to set the size here, due to UsedWithSwapChainOnly
						1,
						QRhiRenderBuffer::UsedWithSwapChainOnly));
	mSwapChain->setWindow(this);
	mSwapChain->setDepthStencil(mRenderBuffer.get());
	mRenderPassDesciptor.reset(mSwapChain->newCompatibleRenderPassDescriptor());
	mSwapChain->setRenderPassDescriptor(mRenderPassDesciptor.get());
	resizeSwapChain();
	init();
}

void QRhiWindow::renderInternal()
{
	if (!m_hasSwapChain)
		return;
	if (mSwapChain->currentPixelSize() != mSwapChain->surfacePixelSize()) {
		resizeSwapChain();
		if (!m_hasSwapChain)
			return;
	}
	QRhi::FrameOpResult ret = mRhi->beginFrame(mSwapChain.get());
	if (ret == QRhi::FrameOpSwapChainOutOfDate) {
		resizeSwapChain();
		if (!m_hasSwapChain)
			return;
		mRhi->beginFrame(mSwapChain.get());;
	}
	if (ret != QRhi::FrameOpSuccess) {
		qDebug("beginFrame failed with %d, retry", ret);
		return;
	}
	render();
	mRhi->endFrame(mSwapChain.get());
}

void QRhiWindow::resizeSwapChain()
{
	m_hasSwapChain = mSwapChain->createOrResize();
}

void QRhiWindow::releaseSwapChain()
{
	if (m_hasSwapChain) {
		m_hasSwapChain = false;
		mSwapChain->destroy();
	}
}

void QRhiWindow::exposeEvent(QExposeEvent*)
{
	if (isExposed() && !m_running) {
		m_running = true;
		initInternal();
		resizeSwapChain();
	}

	const QSize surfaceSize = m_hasSwapChain ? mSwapChain->surfacePixelSize() : QSize();

	// stop pushing frames when not exposed (or size is 0)
	if ((!isExposed() || (m_hasSwapChain && surfaceSize.isEmpty())) && m_running && !m_notExposed) {
		qDebug("not exposed");
		m_notExposed = true;
	}

	// Continue when exposed again and the surface has a valid size. Note that
	// surfaceSize can be (0, 0) even though size() reports a valid one, hence
	// trusting surfacePixelSize() and not QWindow.
	if (isExposed() && m_running && m_notExposed && !surfaceSize.isEmpty()) {
		qDebug("exposed again");
		m_notExposed = false;
		m_newlyExposed = true;
	}

	// always render a frame on exposeEvent() (when exposed) in order to update
	// immediately on window resize.
	if (isExposed() && !surfaceSize.isEmpty())
		renderInternal();
}

bool QRhiWindow::event(QEvent* e)
{
	switch (e->type()) {
	case QEvent::UpdateRequest:
		renderInternal();
		break;
	case QEvent::PlatformSurface:
		// this is the proper time to tear down the swapchain (while the native window and surface are still around)
		if (static_cast<QPlatformSurfaceEvent*>(e)->surfaceEventType() == QPlatformSurfaceEvent::SurfaceAboutToBeDestroyed)
			releaseSwapChain();
		break;

	default:
		break;
	}
	return QWindow::event(e);
}