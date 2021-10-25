#include "stdafx.h"
#include "FileCopier.h"
#include "GCodeGenerator.h"
#include "PrintingInfo.h"

#define BUF_SIZE 1024
FileCopier::FileCopier(QWidget *parent_)
	:progressHandler(nullptr)
{
	progressHandler = new ProgressHandler(parent_);
}
FileCopier::FileCopier()
	: progressHandler(nullptr)
{

}
FileCopier::~FileCopier()
{
	if (progressHandler)
		progressHandler->close();
}

void FileCopier::setProgressHandler(ProgressHandler* handler_)
{
	progressHandler = handler_;
}

bool FileCopier::copy(QString src_fname, QString dst_fname)
{
	return copy(Generals::qstringTowchar_t(src_fname), Generals::qstringTowchar_t(dst_fname));
}

bool FileCopier::copy(wchar_t* src_fname, wchar_t* dst_fname)
{
	printf("copy_file!\n");

	FILE *fp_src;
	FILE *fp_dst;

	char copy_buf[BUF_SIZE];
	int n_read;
	int n_copy = 0;
	int n_fileSize = 0;

	if ((fp_src = _wfopen(src_fname, L"r")) == NULL)
	{
		fwprintf(stderr, L"Check the source file : (%s)\n", src_fname);
		return false;
	}

	n_fileSize = Generals::getFileSize(fp_src);

	printf("file size = %d\n", n_fileSize);

	if ((fp_dst = _wfopen(dst_fname, L"w")) == NULL)
	{
		fclose(fp_src);
		fwprintf(stderr, L"Check the destination file : (%s)\n", dst_fname);
		return false;
	}
	if (progressHandler)
		progressHandler->setMaximum(n_fileSize);

	while ((n_read = fread(copy_buf, 1, BUF_SIZE, fp_src)) > 0)
	{
		fwrite(copy_buf, n_read, 1, fp_dst);
		n_copy += n_read;
		if (progressHandler)
		{
			progressHandler->setValue(n_copy);

			if (progressHandler->wasCanceled())
			{
				fclose(fp_src);
				fclose(fp_dst);

				_wremove(dst_fname);

				return false;
			}
		}
	}

	if (progressHandler)
		progressHandler->setValue(n_fileSize);

	if (fclose(fp_dst) != EOF)
	{
		fclose(fp_src);
		return true;
	}
	else
	{
		fclose(fp_src);
		return false;
	}
}

bool FileCopier::overwriteAuthentificationCode(QString src_fname, QString dst_fname)
{
	QFile inputFile(src_fname);
	inputFile.open(QIODevice::ReadOnly);
	if (!inputFile.isOpen())
		return false;

	QTextStream inStream(&inputFile);

	QString allFileContext = inStream.readAll();
	int totalLineCount = allFileContext.split("\n").count() - 1;
	inStream.seek(0);
	int countLine = 0;

	QString line;

	QByteArray outData;
	QString authentificationCode = GCodeGenerator::generateAuthentificationCode();

	if (authentificationCode == "")
	{
		inputFile.close();
		return copy(src_fname, dst_fname);
	}
	if (progressHandler)
		progressHandler->setMaximum(totalLineCount);

	QFile outputFile(dst_fname);
	outputFile.open(QIODevice::WriteOnly);
	if (!outputFile.isOpen())
	{
		inputFile.close();
		return false;
	}
	QTextStream outStream(&outputFile);

	bool hasCode = false;
	while (!inStream.atEnd())
	{
		if (progressHandler->wasCanceled())
		{
			inputFile.close();
			outputFile.close();
			progressHandler = nullptr;
			copy(src_fname, dst_fname);
			return false;
		}
		line = inStream.readLine();
		if (progressHandler)
			progressHandler->setValue(countLine++);
		if (line.startsWith(";SERVER_ID:") || line.startsWith(";SERVER_PW:") || line.startsWith(";PC_NAME:"))
			continue;
		else if (line.startsWith(";PC_IP:"))
		{
			outStream << authentificationCode << "\n";
			hasCode = true;
		}
		else if (line.startsWith(";DIMENSION:"))
		{
			if (!hasCode)
				outStream << authentificationCode << "\n";
			outStream << line << "\n";
		}
		else
		{
			outStream << line << "\n";
		}
	}

	if (progressHandler)
		progressHandler->setValue(totalLineCount);

	inputFile.close();
	outputFile.close();
	return true;
}

bool FileCopier::insertImageToGCode(PrintingInfo* printingInfo_, QString imagePath_)
{
	printingInfo_->setGcodeTempFileName();
	QString src_fname = printingInfo_->getGcodeTempFileName();
	QString dst_fname = printingInfo_->getGcodeFileName();

	//gcode 파일을 gcodeTemp파일로 복사
	QFile outputFile(dst_fname); //gcodeFile
	QFile inputFile(src_fname); //gcodeTempFile
	if (inputFile.exists()) inputFile.remove();
	if (!outputFile.copy(inputFile.fileName()))
		return false;

	inputFile.open(QIODevice::ReadOnly);
	if (!inputFile.isOpen())
		return false;

	QTextStream inStream(&inputFile);

	QString allFileContext = inStream.readAll();
	int totalLineCount = allFileContext.split("\n").count() - 1;
	inStream.seek(0);
	int countLine = 0;

	QString line;

	QByteArray outData;

	if (progressHandler)
		progressHandler->setMaximum(totalLineCount);

	outputFile.open(QIODevice::WriteOnly);
	if (!outputFile.isOpen())
	{
		inputFile.close();
		return false;
	}
	QTextStream outStream(&outputFile);
	QString encodedImage = GCodeGenerator::generateImageTextCode(imagePath_);

	if (printingInfo_->getSlicerType() == SINDOH)
	{
		bool imageInserted = false;
		while (!inStream.atEnd())
		{
			if (progressHandler->wasCanceled())
			{
				inputFile.close();
				outputFile.close();
				progressHandler = nullptr;
				copy(src_fname, dst_fname);
				return false;
			}
			line = inStream.readLine();
			if (progressHandler)
				progressHandler->setValue(countLine++);

			if (line.startsWith(";IMAGE"))
				continue;
			else if (line.startsWith(";FILENAME") && !imageInserted)
			{
				outStream << encodedImage << "\n";
				outStream << line << "\n";
			}
			else
				outStream << line << "\n";
		}
	}
	else
	{
		outStream << encodedImage;
		QString code = GCodeGenerator::generateCodeForOtherSlicer(printingInfo_);
		outStream << code;

		while (!inStream.atEnd())
		{
			if (progressHandler->wasCanceled())
			{
				inputFile.close();
				outputFile.close();
				progressHandler = nullptr;
				copy(src_fname, dst_fname);
				return false;
			}
			line = inStream.readLine();
			if (progressHandler)
				progressHandler->setValue(countLine++);

			if (line.startsWith(";IMAGE") ||
				line.startsWith(";FILENAME") ||
				line.startsWith(";ESTIMATION_TIME") ||
				line.startsWith(";OPERATINGZONE") ||
				line.startsWith(";ESTIMATION_FILAMENT_CARTRIDGE"))
			{
				continue;
			}
			else
				outStream << line << "\n";
		}
	}

	if (progressHandler)
		progressHandler->setValue(totalLineCount);

	inputFile.close();
	outputFile.close();
	if (!printingInfo_->setEncodedImage(imagePath_))
		return false;
	return true;
}

bool FileCopier::deleteImageInGCode(PrintingInfo* printingInfo_)
{
	printingInfo_->setGcodeTempFileName();
	QString src_fname = printingInfo_->getGcodeTempFileName();
	QString dst_fname = printingInfo_->getGcodeFileName();

	//gcode 파일을 gcodeTemp파일로 복사
	QFile outputFile(dst_fname); //gcodeFile
	QFile inputFile(src_fname); //gcodeTempFile
	if (inputFile.exists()) inputFile.remove();
	if (!outputFile.copy(inputFile.fileName()))
		return false;

	inputFile.open(QIODevice::ReadOnly);
	if (!inputFile.isOpen())
		return false;

	QTextStream inStream(&inputFile);

	QString allFileContext = inStream.readAll();
	int totalLineCount = allFileContext.split("\n").count() - 1;
	inStream.seek(0);
	int countLine = 0;

	QString line;

	QByteArray outData;

	if (progressHandler)
		progressHandler->setMaximum(totalLineCount);

	outputFile.open(QIODevice::WriteOnly);
	if (!outputFile.isOpen())
	{
		inputFile.close();
		return false;
	}
	QTextStream outStream(&outputFile);

	if (printingInfo_->getSlicerType() == SINDOH)
	{
		while (!inStream.atEnd())
		{
			if (progressHandler->wasCanceled())
			{
				inputFile.close();
				outputFile.close();
				progressHandler = nullptr;
				copy(src_fname, dst_fname);
				return false;
			}
			line = inStream.readLine();
			if (progressHandler)
				progressHandler->setValue(countLine++);

			if (line.startsWith(";IMAGE"))
				continue;
			else
				outStream << line << "\n";
		}
	}
	else
	{
		while (!inStream.atEnd())
		{
			if (progressHandler->wasCanceled())
			{
				inputFile.close();
				outputFile.close();
				progressHandler = nullptr;
				copy(src_fname, dst_fname);
				return false;
			}
			line = inStream.readLine();
			if (progressHandler)
				progressHandler->setValue(countLine++);

			if (line.startsWith(";IMAGE") ||
				line.startsWith(";FILENAME") ||
				line.startsWith(";ESTIMATION_TIME") ||
				line.startsWith(";OPERATINGZONE") ||
				line.startsWith(";ESTIMATION_FILAMENT_CARTRIDGE"))
			{
				continue;
			}
			else
				outStream << line << "\n";
		}
	}

	if (progressHandler)
		progressHandler->setValue(totalLineCount);

	inputFile.close();
	outputFile.close();
	return true;
}