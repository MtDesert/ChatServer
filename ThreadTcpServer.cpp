#include "ThreadTcpServer.h"
#include <QSqlError>
#include <QDebug>
ThreadTcpServer::ThreadTcpServer(QSqlDatabase sqldb, QObject *parent) :
    QTcpServer(parent), db(sqldb)
{

}

void ThreadTcpServer::incomingConnection(int handle)
{
    ThreadTcpSocket *threadTcpSocket = new ThreadTcpSocket(handle, db, this);
    //connect(threadTcpSocket, SIGNAL(finished()), threadTcpSocket,SLOT(deleteLater()));
    connect(threadTcpSocket, SIGNAL(signalTcpConnectInfor(int,int*)),
            this, SLOT(slotRecordSocketInofor(int,int*)));
    connect(threadTcpSocket, SIGNAL(signalTcpSocketDescriptor_And_Pointer(const ThreadTcpSocket*,int*,QTcpSocket*)),
            this, SLOT(slotTcpSocketInitReady(const ThreadTcpSocket*,int*,QTcpSocket*)));
    connect(threadTcpSocket,SIGNAL(signalGetConnect(int, ThreadTcpSocket*)), SLOT(slotGetTcpConnect(int, ThreadTcpSocket*)),Qt::DirectConnection);
    threadTcpSocket->start();
}

void ThreadTcpServer::slotClientSocketClose(const ThreadTcpSocket *p_threadTcpSocket,
                                            int *p_socketHandle, QTcpSocket *p_tcpSocket, int userId)
{
	qDebug()<<QString::fromUtf8("有客户端断开~~")<<endl;
//    qDebug()<<QString::fromUtf8("信号传来的断开的客户端Tcp 描述为  ")<<*p_socketHandle<<endl;
   // qDebug()<<QString::fromUtf8("信号传来的断开的客户端TcpServer 实例地址为  ")<<p_threadTcpSocket<<endl;
	//qDebug()<<QString::fromUtf8("信号传来的断开的客户端p_tcpSocket 实例地址为  ")<<p_tcpSocket<<endl;
   // qDebug()<<QString::fromUtf8("信号传来的断开的客户端TcpServer->p_tcpSocket 实例地址为  ")<<p_threadTcpSocket->p_tcpSocket<<endl;

   // qDebug()<<QString::fromUtf8("删除记录之前的用户ID 以及Tcp描述符  ")<<hash_UserID_And_TcpDescriptor<<endl;
   // qDebug()<<QString::fromUtf8("删除记录之前的Tcp描述符 以及 TcpServer对象地址  ")<<hash_TcpDescriptor_And_ThreadTcpSocket<<endl;

    QMultiHash<int, int>::const_iterator i;
    for(i = hash_UserID_And_TcpDescriptor.begin(); i != hash_UserID_And_TcpDescriptor.end(); ++i)
    {
		//qDebug()<<*p_socketHandle <<" "<<i.value()<<endl;
        if(*p_socketHandle == i.value())
        {
            int id = i.key();
			qDebug()<<QString::fromUtf8("找到与tcp描述符相符的用户ID, 此ID的链接记录要被删除 ")<<id <<endl;
            hash_UserID_And_TcpDescriptor.remove(id);
            break;
        }
    }
    hash_TcpDescriptor_And_ThreadTcpSocket.remove(*p_socketHandle);
//    qDebug()<<QString::fromUtf8("删除记录之后的hash_TcpDescriptor_And_ThreadTcpSocket  ")<<hash_TcpDescriptor_And_ThreadTcpSocket<<endl;
}

void ThreadTcpServer::slotRecordSocketInofor(int userId, int *descriptor)
{
    hash_UserID_And_TcpDescriptor.insert(userId, *descriptor);
}

void ThreadTcpServer::slotTcpSocketInitReady(const ThreadTcpSocket *p_threadTcpSocket, int *p_socketHandle, QTcpSocket *p_tcpSocket)
{//参数是链接的指针, 链接的描述符
    hash_TcpDescriptor_And_ThreadTcpSocket.insert(*p_socketHandle, p_threadTcpSocket);
    connect(p_threadTcpSocket, SIGNAL(signalTcpConnectDisconnected(const ThreadTcpSocket*,int*,QTcpSocket*,int)),
			this, SLOT(slotClientSocketClose(const ThreadTcpSocket*,int*,QTcpSocket*,int)));
}

void ThreadTcpServer::slotGetTcpConnect(int id,  ThreadTcpSocket *p)
{
    qDebug()<<"fffffffff";
    int decriptor= hash_UserID_And_TcpDescriptor.value(id);
    if(0 == decriptor)
    {
        p->getConnectFromThreadTcpServer(NULL,false);
        return;
    }
	//ThreadTcpSocket  *getConnect = (ThreadTcpSocket *)hash_TcpDescriptor_And_ThreadTcpSocket.value(decriptor);
	ThreadTcpSocket  *getConnect = const_cast<ThreadTcpSocket *>(hash_TcpDescriptor_And_ThreadTcpSocket.value(decriptor));
	//-_-!!不要总用C风格的强转~ 用C++的....
    p->getConnectFromThreadTcpServer(getConnect,true);
}
