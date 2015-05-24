///////////////////////////////////////////////////////////////////////////////
///
/// \code main
/// \brief Main file that runs QTunes.
///
/// This code contains the main function that runs QTunes.
///
///////////////////////////////////////////////////////////////////////////////
#include <QApplication>
#include "MainWindow.h"

//! The main function just runs QTunes
int main(int argc, char **argv) {
	// init variables and application font
	QString	      program = argv[0];
	QApplication  app(argc, argv);

	// invoke  MainWindow constructor
	MainWindow window(program);

	// display MainWindow
	window.show();

	return app.exec();
}
