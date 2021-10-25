#pragma once

template<class T>
class CommunicationInterface
{
public:
	CommunicationInterface<T>() : m_ptr(nullptr), pCount(new int(1)) {};
	CommunicationInterface<T>(T* ptr) : m_ptr(ptr), pCount(new int(1)){}

	CommunicationInterface<T>(const CommunicationInterface<T> &object)
	{
		pCount = object.pCount;
		m_ptr = object.m_ptr;
		++*pCount;
	}

	CommunicationInterface<T>& operator=(const CommunicationInterface<T> &object)
	{
		if (m_ptr != object.m_ptr){
			tryToDelete();
			pCount = object.pCount;
			m_ptr = object.m_ptr;
			++*pCount;
		}

		return *this;
	}


	T* operator->()
	{
		return m_ptr;
	}

	T* getRawPointer()
	{
		return m_ptr;
	}

	~CommunicationInterface<T>()
	{
		tryToDelete();
	}


private:
	void tryToDelete()
	{
		if (--*pCount == 0){
			delete m_ptr;
			delete pCount;
			m_ptr = NULL;
			pCount = NULL;
		}
	}
private:
	T* m_ptr;
	int *pCount;

};