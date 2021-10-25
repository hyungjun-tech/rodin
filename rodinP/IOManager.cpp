#include "stdafx.h"
#include "IOManager.h"

void MeshModelImporter::operator()(std::vector<IMeshModel*>& models_, 
	QString fileName_)
{
	// Convert var type : [QString] → [wchar_t]
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*const wchar_t* wFileName = ToWideCharArray()(fileName_);

	// open the archive
	std::ifstream ifs(wFileName);
	boost::archive::text_iarchive ia(ifs, boost::archive::archive_flags::no_header);
	ia.register_type(static_cast<BasicMeshModel *>(NULL));
	ia.register_type(static_cast<JoinedMeshModel *>(NULL));

	// restore the schedule from the archive
	ia >> models_;*/
}

void MeshModelExporter::operator()(const std::vector<IMeshModel*>& models_, 
	QString fileName_)
{
	// Convert var type : [QString] → [wchar_t]
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*const wchar_t* wFileName = ToWideCharArray()(fileName_);

	// make an archive
	std::ofstream ofs(wFileName);
	boost::archive::text_oarchive oa(ofs, boost::archive::archive_flags::no_header);
	oa.register_type(static_cast<BasicMeshModel *>(NULL));
	oa.register_type(static_cast<JoinedMeshModel *>(NULL));

	oa << models_;*/
}