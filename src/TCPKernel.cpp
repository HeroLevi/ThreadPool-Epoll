#include "TCPKernel.h"

//ThreadPool* m_pool;
MySql m_sql;

TCPKernel::TCPKernel()
{
	m_pTcpNet = new TCPNet(this);
	m_pool = new ThreadPool;
}

TCPKernel::~TCPKernel()
{
	if(m_pTcpNet)
	{
		delete m_pTcpNet;
		m_pTcpNet = NULL;
	}
}

bool TCPKernel::Open()
{
	//1.初始化线程池
	if(!m_pool->InitThreadPool(500,10,100))
		return false;
	//2.初始化网络
	if(!(m_pTcpNet->InitNetWork()))
		return false;
	//3.初始化数据库
	if(!m_sql.ConnectMySql((char*)"localhost",(char*)"root",(char*)"colin123",(char*)"test"))
		return false;
	return true;
}

void TCPKernel::Close()
{
	m_pTcpNet->UnInitNetWork();
	m_pool->DestroyThreadPool();
}

void TCPKernel::DealData(int sock,char* szbuf)
{
	
}
