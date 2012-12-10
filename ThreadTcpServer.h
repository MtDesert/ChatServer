#ifndef THREADTCPSERVER_H
#define THREADTCPSERVER_H

#include <QTcpServer>
#include <QSqlDatabase>
#include "ThreadTcpSocket.h"
class ThreadTcpServer : public QTcpServer
{
    Q_OBJECT
public:
	explicit ThreadTcpServer(QSqlDatabase sqldb, QObject *parent = 0);

signals:

private slots:
	void slotClientSocketClose(const ThreadTcpSocket *p_threadTcpSocket,
							   int *p_socketHandle, QTcpSocket *p_tcpSocket, int userId);
	void slotRecordSocketInofor(int userId, int *descriptor);	//这里接收到的参数是链接的ID和描述符
	void slotTcpSocketInitReady(const ThreadTcpSocket *p_threadTcpSocket, //参数是链接的指针, 链接的描述符
							int *p_socketHandle, QTcpSocket *p_tcpSocket);
    void slotGetTcpConnect(int id,ThreadTcpSocket  *p);
protected:
	void incomingConnection(int handle);//handle是Tcp的标识符
private:
private:
	QSqlDatabase db;
	//用这两个东西最终能找到一个链接,如果该链接存在的话
	QMultiHash<int, int> hash_UserID_And_TcpDescriptor;	//用户的ID还有tcp的标识符
	QMultiHash<int, const ThreadTcpSocket*> hash_TcpDescriptor_And_ThreadTcpSocket;
};

#endif // THREADTCPSERVER_H
