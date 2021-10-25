#pragma once
#include "ProgressHandler.h"

class PrintingInfo;
class FileCopier
{
public:
	FileCopier(QWidget *parent_);
	FileCopier();
	~FileCopier();
	void setProgressHandler(ProgressHandler* handler_);
	bool copy(QString src_fname, QString dst_fname);
	bool copy(wchar_t* src_fname, wchar_t* dst_fname);
	bool overwriteAuthentificationCode(QString src_fname, QString dst_fname);
	bool insertImageToGCode(PrintingInfo* printingInfo_, QString imagePath_);
	bool deleteImageInGCode(PrintingInfo* printingInfo_);
private:
	ProgressHandler *progressHandler;
};
