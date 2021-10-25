#include "stdafx.h"
#include "ExperimentalFunctions.h"


bool ExperimentalFunctions::profileEdit_ByKey(QString _file_path, QString _remove_key, QString _original_key, QString _replaced_key)
{
	QFile input_file, output_file;
	input_file.setFileName(_file_path);
	input_file.open(QIODevice::ReadOnly);

	//
	QFileInfo input_file_path_info(_file_path);
	QString output_file_path = _file_path.section(input_file_path_info.completeBaseName(), 0, 0); //파일 이름을 제외한 full path
	output_file_path.append("output_profile/");
	Generals::checkPath(output_file_path);
	output_file_path.append(_file_path.section('/', -1)); // .section('.', 0, -2); // file name extraction
	//

	output_file.setFileName(output_file_path);
	output_file.open(QIODevice::WriteOnly);

	if (!input_file.isOpen() || !output_file.isOpen())
		return false;

	QTextStream	in_stream(&input_file);
	QTextStream out_stream(&output_file);

	QString str_line;

	while (!in_stream.atEnd())
	{
		str_line = in_stream.readLine();

		if (str_line == "")
			continue;

		if (str_line.startsWith(_remove_key))
			continue;

		if (str_line.startsWith(_original_key))
		{
			str_line.replace(_original_key, _replaced_key, Qt::CaseSensitive);
		}

		out_stream << str_line;
		out_stream << "\n";
	}

	input_file.close();
	output_file.close();

	return true;
}

bool ExperimentalFunctions::profileEdit_insertBeforeKey(QString _file_path, QString _target_key, QString _insert_string)
{
	QFile input_file, output_file;
	input_file.setFileName(_file_path);
	input_file.open(QIODevice::ReadOnly);

	//
	QFileInfo input_file_path_info(_file_path);
	QString output_file_path = _file_path.section(input_file_path_info.completeBaseName(), 0, 0); //파일 이름을 제외한 full path
	output_file_path.append("output_profile/");
	Generals::checkPath(output_file_path);
	output_file_path.append(_file_path.section('/', -1)); // .section('.', 0, -2); // file name extraction
	//

	output_file.setFileName(output_file_path);
	output_file.open(QIODevice::WriteOnly);

	if (!input_file.isOpen() || !output_file.isOpen())
		return false;

	QTextStream	in_stream(&input_file);
	QTextStream out_stream(&output_file);

	QString str_line;

	while (!in_stream.atEnd())
	{
		str_line = in_stream.readLine();

		if (str_line == "")
			continue;

		if (str_line.startsWith(_target_key))
		{
			out_stream << _insert_string;
			out_stream << "\n";
		}

		out_stream << str_line;
		out_stream << "\n";
	}

	input_file.close();
	output_file.close();

	return true;
}

