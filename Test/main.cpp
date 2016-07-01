#include <QtWidgets/QApplication>
//#define GLM_SWIZZLE
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "myWidget.h"
#include "Canvas.h"

#include <string>
#include <sstream>
#include <iostream>
#include <memory>

int main(int argc, char *argv[])
{
	QApplication app(argc, argv);

	Canvas w;
	w.show();

	return app.exec();

}
