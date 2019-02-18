#ifndef _TCPKERNEL_H
#define _TCPKERNEL_H

#include "IKernel.h"
#include "TCPNet.h"
#include "PackDef.h"
#include "ThreadPool.h"

class TCPKernel:public IKernel
{
public:
	TCPKernel();
	virtual ~TCPKernel();
public:
	bool Open();
	void Close();
	void DealData(int sock,char* szbuf);
public:
	
public:
	INet* m_pTcpNet;
	ThreadPool* m_pool;
};

#endif
