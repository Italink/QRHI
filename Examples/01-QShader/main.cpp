#include <QGuiApplication>
#include <QFile>
#include "private\qshader_p.h"

QShader createShaderFromQSBFile(QString path) {
	QFile f(path);
	if (f.open(QIODevice::ReadOnly))
		return QShader::fromSerialized(f.readAll());
	return QShader();
}

QShader createShaderFromCode(QString code) {
}

int main(int argc, char** argv)
{
	QGuiApplication app(argc, argv);

	return app.exec();
}