#include <QGuiApplication>
#include "QSurfaceFormat"
#include "QRhiWindow.h"

int main(int argc, char** argv)
{
	QGuiApplication app(argc, argv);
	QSurfaceFormat fmt;
	fmt.setDepthBufferSize(24);
	fmt.setStencilBufferSize(8);
	QRhiWindow::setDefaultSurfaceFormat(fmt);
	QRhiWindow w(QRhi::Vulkan);
	w.show();
	return app.exec();
}