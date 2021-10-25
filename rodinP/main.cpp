#include "stdafx.h"
#include "rodinP.h"

int main(int argc, char *argv[])
{
	//#ifdef _WIN32
	//  {
	//	  typedef BOOL(*SetProcessDpiAwarenessT)(int value);
	//	  QLibrary user32("user32.dll", NULL);
	//	  SetProcessDpiAwarenessT SetProcessDpiAwarenessD =
	//		  (SetProcessDpiAwarenessT)user32.resolve("SetProcessDpiAwarenessInternal");
	//	  if (SetProcessDpiAwarenessD)
	//	  {
	//		  Logger::write("SetProcessDpiAwarenessD - true");
	//		  SetProcessDpiAwarenessD(1); //PROCESS_SYSTEM_DPI_AWARE
	//	  }
	//  }
	//#endif

		//for not using OpenGL ES : 20191023 by swyang//
	QApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
	//QApplication::setAttribute(Qt::AA_EnableHighDpiScaling); // DPI support
	//QCoreApplication::setAttribute(Qt::AA_UseHighDpiPixmaps); //HiDPI pixmaps
	//QGuiApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	QApplication a(argc, argv);

	//QGraphicsScene scene;
	//QGraphicsView view(&scene);
	//QGraphicsPixmapItem item(QPixmap("c:\\Thumbnail.png"));
	//
	//scene.addItem(&item);
	//view.show();
	//
	//Sleep(3000);

	//view.close();


	rodinP w;
	w.show();

	QString openGLver((const char*)glGetString(GL_VERSION));

	qDebug() << "OpenGL ver : " << openGLver;

	openGLver.truncate(1);


	if (openGLver == "1")
	{
		QString message = MessageAlert::tr("opengl_version_not_compatible").arg(AppInfo::getAppName());
		QMessageBox::warning(&w, CustomTranslate::tr("Warning"), message, QMessageBox::Ok);
	}

	if (argc > 1)
	{
		/*argv++;

		QTextCodec * codec = QTextCodec::codecForName("eucKR");
		QString scanFile = codec->toUnicode(*argv);

		SetCurrentDirectory((LPCWSTR)Generals::qstringTowchar_t(Generals::getAppPath()));
		w.loadModelFile(scanFile);*/

		SetCurrentDirectory((LPCWSTR)Generals::qstringTowchar_t(Generals::getAppPath()));
		QStringList inputFileList;
		for (int i = 1; i < argc; i++)
		{
			argv++;
			inputFileList.push_back(QString::fromLocal8Bit(*argv));
		}
		w.loadFiles(inputFileList);
	}

	return a.exec();
}
