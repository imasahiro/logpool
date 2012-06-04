#include "viewer.hpp"

int main(int argc, char **argv)
{
	QApplication app(argc, argv);
	Viewer *v = new Viewer();
	v->show();
	app.exec();
	return 0;
}
