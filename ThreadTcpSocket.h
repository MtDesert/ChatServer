#ifndef THREADTCPSOCKET_H
#define THREADTCPSOCKET_H

#include <QTcpSocket>
#include <QUdpSocket>
#include <QThread>
#include <QStringList>
#include <QSqlDatabase>
#include <QSqlQuery>

const int USER_ID_START = 100000000;

class ThreadTcpSocket : public QThread
{
	Q_OBJECT
public:
	explicit ThreadTcpSocket(int handle, QSqlDatabase &sqldb, QObject *parent);
	void getConnectFromThreadTcpServer(ThreadTcpSocket *p, bool flag);//用这个参数来判断是否获取到连接，得到连接放在指针。
	QTcpSocket *p_tcpSocket;
	int socketHandle;
	int *p_socketHandle;
signals:
	//void signalTcpConnectInfor(int userId, const ThreadTcpSocket *threadTcpSocket);
	void signalTcpConnectInfor(int userId, int *descriptor);
	void signalTcpSocketDescriptor_And_Pointer(const ThreadTcpSocket *p_threadTcpSocket,
											 int *p_socketHandle, QTcpSocket *p_tcpSocket);
    void signalTcpConnectDisconnected(const ThreadTcpSocket *p_threadTcpSocket,
									  int *p_socketHandle, QTcpSocket *p_tcpSocket, int userId);
	void signalGetConnect(int id, ThreadTcpSocket  *p);//告诉ThreadTcpServer获取相应tcp连接。
public slots:
    void slotReadDatagramFromClient();
	void slotTcpConnectDisconnected();

protected:
	void run();
private:
		//bool connectToSqlDatabase();
	void clientRegisterMessage(QDataStream &in);
	void clientPersonalMessage(QDataStream &in,qint32 sourceID);//传回个人信息请求(大漠定义的)
	void clientRequireUpdateFriendInfo(QDataStream &in,qint32 sourceID);//这个是用来把新的好友信息回发的
	void clientInitFriendsInfomationMessage(QDataStream &in, qint32 sourceID);
	void clientEditRemarksMessage(QDataStream &in);//修改备注的请求
	void clientDeleteFriendMessage(QDataStream &in);//删除好友的请求
	void clientModifyPersonalInfoMessage(QDataStream &in,qint32 sourceID);
	void clientLoginMessage(QDataStream &in);
	void registerResultToClient(const QString &str);    ///这个函数是返回注册信息给客户端的
    void clientFindPasswdMessage(QDataStream &in);
	void clientNewPasswdMessage(QDataStream &in);
	void clientSeekFriend(QDataStream &ins);//查找好友函数。
	void readAddFriendFromClient(QDataStream &in, int sourceId,QTcpSocket *p_tcpSocket);//接收到添加好友的请求。
	void sendAddFriendToClient(QString strTime, int sourceId ,  int destination, int  friendId);//发送添加好友信息给服务端。
    void insertInforToUserchat(QDateTime time, int infor, int senderId, int acceptId);
	void readAgreeAddFriendInfor(QDataStream &in, int friendId);
	void addFriendInDatabase(int requesterID, int friendId);
	void clientSendChatMessage(qint32 desID, QDataStream &in);
	void sendChatMessageToClient(QDateTime time, const QString &chat);
	void saveChatMessageToDatabase(QDateTime time, const QString &chat);
    void clientPasswdProtectAnswers(QDataStream &in);
	void sendOffLineMessage(int id);
    void sendOfLineAddFriend(int id);//用户上线后，发送添加好友的消息。
    void sendAgreeAddfriendToClient(QDateTime time,int requesterId, int friendId);
	void clientTransferFile(QDataStream &in, qint32 id_self);	//客户端发送文件的请求
	void sendTransferFileRequest(qint32 id_self, qint64  size, QString name, QByteArray iconByte);		//发送传输文件请求给目的id
    void transFileDesClientOffline(qint32 id_self, qint32 id_friend);   //这里用于通知发送方,文件的接收方不在线
private:
	QSqlDatabase db;
	qint32 blockSize;
	QStringList strList;

	QUdpSocket *udpSocket;



	QString q1;//问题
	QString q2;
	QString q3;

    QString a1;//收到客户端的答案。
	QString a2;
	QString a3;



	QString passwd;
//	QSqlDatabase db;
	QSqlQuery *query;
	QSqlQuery *queryPasswd;
	QString userName;
	int userNameInt;

	ThreadTcpSocket const*tcpSocketConnect;
	QTcpSocket *tcpSocket;
	bool userLine;

	QHostAddress clientAddress;
	quint16  clientPort;

	////////////////////////////////////////////////////
	//聊天信息相关变量
	qint32 id_src;
	qint32 id_des;
	qint32 imgCount;
	QByteArray imgTempDataByte;
	QString html;
	/////////////////////////////////////////////////////
};

#endif // THREADTCPSOCKET_H
