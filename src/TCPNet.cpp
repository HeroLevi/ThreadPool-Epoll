#include "TCPNet.h"

extern MySql m_sql;

bool TCPNet::m_flag = true;
IKernel *TCPNet::m_pKernel = NULL;

TCPNet::TCPNet(IKernel *pKernel)
{
	s_socket = -1;
	m_pKernel = pKernel;
}

TCPNet::~TCPNet()
{

}

void TCPNet::setnonblocking(int fd)
{
	int old_option = fcntl(fd,F_GETFL);
	int new_option = old_option | O_NONBLOCK;
	fcntl(fd,F_SETFL,new_option);
}

void TCPNet::addfd(int epollfd,int fd)
{
	epoll_event event;
	event.data.fd = fd;
	event.events = EPOLLIN | EPOLLET;
	epoll_ctl(epollfd,EPOLL_CTL_ADD,fd,&event);
	setnonblocking(fd);
}

void TCPNet::removefd(int epollfd,int fd)
{
	epoll_ctl(epollfd,EPOLL_CTL_DEL,fd,0);
	close(fd);
}

bool TCPNet::InitNetWork()
{
	cout<<"server start..."<<endl;
	struct sockaddr_in saddr;
	bzero(&saddr,sizeof(saddr));
	saddr.sin_family = AF_INET;
	saddr.sin_addr.s_addr = inet_addr(_DEFIP);
	saddr.sin_port = htons(_DEFPORT);

	s_socket = socket(AF_INET,SOCK_STREAM,0);
	assert(s_socket != -1);

	int ret = bind(s_socket,(struct sockaddr*)&saddr,sizeof(saddr));
	if(ret == -1) 
	{   
		UnInitNetWork();
		return false;
	}   
	listen(s_socket,128);

	epoll_event events[MAX_EVENTS];
	epollfd = epoll_create(5);
	assert(epollfd != -1);
	addfd(epollfd,s_socket);

	int num;
	while(m_flag)
	{
		num = epoll_wait(epollfd,events,EPOLLSIZE,-1);
	
		while(num--)
		{
			if(!((TCPKernel*)m_pKernel)->m_pool->Addtask(Jobs,(void*)this))
			{
				cout<<"Addtask failed"<<endl;
				break;
			}
		}
	}
	return true;
}

//accept recv ->TCPKernel->DealData
void* TCPNet::Jobs(void* arg)
{
	TCPNet* pthis = (TCPNet*)arg;

	int PackSize;
	int RealReadNum;
	char* buf = NULL;
	epoll_event events[MAX_EVENTS];

	struct sockaddr_in caddr;
	socklen_t len = sizeof(caddr);
	while(pthis->m_flag){

	int cfd = accept(pthis->s_socket,(struct sockaddr*)&caddr,&len);
	//assert(cfd != -1);
	if(cfd == -1)
		continue;

	pthis->addfd(pthis->epollfd,cfd);

	RealReadNum = recv(cfd,(char*)&PackSize,sizeof(int),0);
	if(RealReadNum <= 0)
	{
		cout<<"client:"<<cfd<<"disconnect"<<endl;
		close(cfd);
	}
	buf = new char[PackSize];
	cout<<"packsize:"<<PackSize<<endl;
	int offset = 0;
	while(PackSize)
	{
		RealReadNum = recv(cfd,buf+offset,PackSize,0);
		offset += RealReadNum;
		PackSize -= RealReadNum;
	}
	pthis->SendData(cfd,buf,strlen(buf));

	cout<<buf<<endl;
	//m_pKernel->DealData(buf);


	char sql[300] = {0};
	list<string> lststr;

	sprintf(sql,"SELECT * FROM tmp ");
	m_sql.SelectMySql(sql,1,lststr);

	char szsql[300] = {0};
	sprintf(szsql,"insert into tmp(name) values('123')");
	//cout<<szsql<<endl;
	if(!m_sql.UpdateMySql(szsql))
	{
		cout<<"sql update failed"<<endl;
	}

	free(buf);
	buf = NULL;
	}
	return NULL;
}


void TCPNet::UnInitNetWork()
{
	m_flag = false;

	removefd(epollfd,s_socket);
	close(s_socket);
	
}

bool TCPNet::SendData(int sock,char* szbuf,int nlen)
{
	if(!sock || !szbuf || !nlen)
		return false;

	if(send(sock,(const char*)&nlen,sizeof(int),0)<0)
		return false;
	if(send(sock,szbuf,nlen,0)<0)
		return false;
	return true;
}
