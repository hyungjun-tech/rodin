#pragma once

#include <QString>


class ExperimentalFunctions
{
public:
	static bool profileEdit_ByKey(QString _file_path, QString _remove_key, QString _original_key, QString _replaced_key);
	//static bool profileEdit_replaceByKey(QString _file_path, QString _original_key, QString _replaced_key);

	static bool profileEdit_insertBeforeKey(QString _file_path, QString _target_key, QString _insert_key);
};

