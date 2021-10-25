#pragma once

class ScaledFrame
{
public:
	ScaledFrame();
	ScaledFrame(ScaledFrame &scaledFrame_);
	~ScaledFrame();

	qglviewer::Frame frame;
	qglviewer::Vec scale;
	QMatrix4x4 getModelMatrix();

	void moveFrame(float dx_, float dy_, float dz_);
	void rotateFrame(qglviewer::Vec wolrdAxis_, float rotAngle_);
	void setScale(qglviewer::Vec scale_);

	qglviewer::Vec toLocalCoords(qglviewer::Vec world_);
	qglviewer::Vec toWorldCoords(qglviewer::Vec local_);
	qglviewer::Vec toLocalVector(qglviewer::Vec world_);
	qglviewer::Vec toWorldVector(qglviewer::Vec local_);

private:
	//현재 rodin은 serialize 사용하지 않으므로 일단 제거
	/*friend class boost::serialization::access;
	template<class Archive>
	void save(Archive & ar, const unsigned int version) const
	{
		qglviewer::Vec pos = frame.position();
		qglviewer::Quaternion q = frame.orientation();

		for (int i = 0; i < 3; i++)
			ar & scale[i];

		for (int i = 0; i < 3; i++)
			ar & pos[i];

		for (int i = 0; i < 4; i++)
			ar & q[i];
	}

	template<class Archive>
	void load(Archive & ar, const unsigned int version)
	{
		for (int i = 0; i < 3; i++)
			ar & scale[i];

		qglviewer::Vec pos;
		for (int i = 0; i < 3; i++)
			ar & pos[i];
		frame.setPosition(pos);

		qglviewer::Quaternion q;
		for (int i = 0; i < 4; i++)
			ar & q[i];
		frame.setOrientation(q);
	}
	BOOST_SERIALIZATION_SPLIT_MEMBER()*/
};