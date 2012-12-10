#include "ThreadTcpSocket.h"
#include <QLabel>
#include <QCryptographicHash>
#include <QBuffer>
#include <QPixmap>
#include <QDebug>
#include <QSqlError>
#include <QDate>

#define ADDFRIENDREQUEST 8
#define AGREEADDFRIEND 9

ThreadTcpSocket::ThreadTcpSocket(int handle,  QSqlDatabase &sqldb, QObject *parent) :
	QThread(parent), socketHandle(handle), db(sqldb), blockSize(0)
{

}

void ThreadTcpSocket::run()
{
	udpSocket = new QUdpSocket(this);
	QTcpSocket *tcpSocket = new QTcpSocket(this);
	if(!tcpSocket->setSocketDescriptor(socketHandle))
	{
		qDebug()<<tcpSocket->errorString()<<endl;
		return;
	}
	p_socketHandle = new int;
	*p_socketHandle = socketHandle;

	this->p_tcpSocket = tcpSocket;

	emit signalTcpSocketDescriptor_And_Pointer(this, p_socketHandle, p_tcpSocket);

	connect(tcpSocket, SIGNAL(readyRead()), this, SLOT(slotReadDatagramFromClient()),Qt::DirectConnection);
	connect(tcpSocket, SIGNAL(disconnected()), this, SLOT(slotTcpConnectDisconnected()));
	exec();//一定要启用事件循环这样线程才能响应信号槽
}

void ThreadTcpSocket::slotReadDatagramFromClient()
{//读取客户端发来的数据报

	qDebug()<<QString::fromUtf8("读取客户端发来的数据") <<p_tcpSocket->socketDescriptor();
	QDataStream in(p_tcpSocket);
	in.setVersion(QDataStream::Qt_4_7);
	if(0 == blockSize)
	{
		if(p_tcpSocket->bytesAvailable() < (qint32)sizeof(qint32))
		{
			return;
		}
		in >> blockSize;
	}

	if(p_tcpSocket->bytesAvailable() < blockSize)
	{
		return;
	}
	qDebug()<<"服务端接收到的数据大小是"<<blockSize;

	in>> id_des >> id_src;
	switch (id_des)
	{
		case 1:
		{
			qDebug()<<QString::fromUtf8("客户端注册新用户的消息");
			clientRegisterMessage(in);
			break;
		}
		case 2:
		{
			qDebug()<<QString::fromUtf8("客户端获取个人信息");
			clientPersonalMessage(in,id_src);
			break;
		}
		case 3:
		{
			qDebug()<<QString::fromUtf8("客户端登录");
			clientLoginMessage(in);
			break;
		}
		case 4:
		{
			qDebug()<<QString::fromUtf8("客户端获取好友列表信息");
			clientInitFriendsInfomationMessage(in, id_src);
			break;
		}
		case 5:
		{
			qDebug()<<QString::fromUtf8("客户端修改备注");
			clientEditRemarksMessage(in);
			break;
		}
		case 6:
		{
			qDebug()<<QString::fromUtf8("客户端删除好友");
			clientDeleteFriendMessage(in);
			break;
		}
		case 7:
		{
			qDebug()<<QString::fromUtf8("客户端查找请求");
			clientSeekFriend(in);
			break;
		}
		case 8:
		{
			qDebug()<<"this datagram is client requet add friend";
			readAddFriendFromClient(in,id_src,p_tcpSocket);
			break;
		}
		case 9:
		{
			qDebug()<<"this datagram is client agree add friend ";
			readAgreeAddFriendInfor(in, id_src);
			break;
		}
		case 10:
		{
			qDebug()<<QString::fromUtf8("客户端登录");
			readAgreeAddFriendInfor(in, id_src);
			break;
		}
		case 20:
		{
			qDebug()<<"this datagram is client find passwd message";
			clientFindPasswdMessage(in);
			break;
		}
		case 21:
		{
			qDebug()<<"this datagram is client set new passwd message";
			clientNewPasswdMessage(in);
			break;
		}
		case 22:
		{
			qDebug()<<"this datagram is client passwd protect answers";//这里是收到密码保护的答案。
			clientPasswdProtectAnswers(in);
			break;
		}
		case 30:
		{
			qDebug()<<"this datagram is client modify personal info message";
			clientModifyPersonalInfoMessage(in,id_src);
			break;
		}
		case 31:
		{
			qDebug()<<"this datagram is client require update friend info message";
			clientRequireUpdateFriendInfo(in,id_src);
			break;
		}
		case 32:
		{
			qDebug()<<"客户端获取离线消息";
			sendOffLineMessage(id_src);
			sendOfLineAddFriend(id_src);
			break;
		}
		case 33:
		{
			qDebug()<<"客户端发送文件请求";
			clientTransferFile(in, id_src);
			break;
		}
		default:
		{
			if(id_des >= USER_ID_START)	//这个USER_ID_START常数被定义为100000000 也就是用户id字段的起始id
			{
				qDebug()<<"客户机要求转发聊天信息";
				clientSendChatMessage(id_des, in);
			}
			else
			{
				qDebug()<< QString::fromUtf8("未知消息! 出现未定义的目的id = ")<<id_des<<endl;
			}
			break;
		}
	}
	blockSize=0;
}

void ThreadTcpSocket::clientRegisterMessage(QDataStream &in)
{//收到客户端注册的信息
	QString registerName;
	QString registerNickName;
	QString registerPasswd;
	QString registerSex;
	QDate   registerBirthday;
	QString registerMail;
	QString question1;
	QString question2;
	QString question3;
	QString answer1;
	QString answer2;
	QString answer3;
	QPixmap img;
	//////////////////////////
	//后来添加的
	QString animalYear;
	QString constellation;
	QString blood;
	QString province;
	QString city;
	QString phone;
	///////////////////////////
	//我操一大堆数据.......
	in >> registerName >> registerNickName >> registerPasswd >> registerSex >> registerBirthday >> registerMail
	   >> question1 >> question2 >> question3 >> answer1 >>answer2 >>answer3 ;
	QByteArray byte;
	in >> byte;
	img.loadFromData(byte, "JPEG");
	if(img.isNull())
	{
		qDebug()<<QString::fromUtf8("警告!客户端注册时 服务器这边检测到头像没有能加载")<<endl;
	}
	in  >> animalYear >> constellation >> blood >> province >> city >> phone;
	//qDebug()<< animalYear << constellation << blood << province << city << phone;
	//	qDebug()<< registerName << registerNickName << registerPasswd  << registerSex << registerBirthday << registerMail
	//			<< question1 << question2 << question3 << answer1 << answer2 << answer3;
	//记录一下客户端的Ip地址还有端口号
	clientAddress = p_tcpSocket->peerAddress();
	clientPort = p_tcpSocket->peerPort();
	//qDebug()<<QString::fromUtf8("客户端的ip地址还有端口号是: ")<<clientAddress<<clientPort<<endl;
	QSqlQuery query(db);
	int numRecord;  //记录插入之前的记录数
	query.exec("select count(*) from user_infor"); //聚集函数统计记录数
	if(query.next())
	{
		numRecord = query.value(0).toInt();
	}
	//	qDebug()<<"before insert recoed numRecord is:"<<numRecord<<endl;
	bool test = query.prepare("insert into user_infor  (user_account, user_nickname, user_password, user_sex, user_birthday, user_mail, user_mood, user_headimage, user_questionone, user_questiontwo, user_questionthree, user_answerone, user_answertwo, user_answerthree, user_phone, user_year, user_constellation, user_blood, user_province, user_city)"
							  "values(?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?, ?)");
	if(test)
	{

		//		qDebug() << "sql shizi hao xiang mei shenme wenti";
		query.addBindValue(registerName);
		query.addBindValue(registerNickName);
		query.addBindValue(registerPasswd);
		query.addBindValue(registerSex);
		query.addBindValue(registerBirthday);
		query.addBindValue(registerMail);
		query.addBindValue("I'm very happy");
		query.addBindValue(byte);
		query.addBindValue(question1);
		query.addBindValue(question2);
		query.addBindValue(question3);
		query.addBindValue(answer1);
		query.addBindValue(answer2);
		query.addBindValue(answer3);
		//////////////////////////////////////////
		//新增部分
		//	query.addBindValue(0);	//判断是否有密保
		query.addBindValue(phone);
		query.addBindValue(animalYear);
		query.addBindValue(constellation);
		query.addBindValue(blood);
		query.addBindValue(province);
		query.addBindValue(city);
		//////////////////////////////////////////
		query.exec();
		query.exec("select count(*) from user_infor"); //聚集函数统计记录数
		int numRecordNow;
		if(query.next())
		{
			numRecordNow = query.value(0).toInt();
		}
		if(numRecordNow != numRecord)
		{//如果插入记录前后记录数量不一致,说明插入成功.通知客户端注册成功
			registerResultToClient(QString("REGISTER_SUCCESS"));
		}
		else
		{//如果插入记录前后记录数量一致,说明插入失败.通知客户端注册失败
			registerResultToClient(QString("REGISTER_FALSE"));
		}

		//		qDebug()<<"before insert recoed numRecordNow is:"<<numRecordNow<<endl;

		///////////////////回一个信息给客户端告诉它注册成功还是失败
	}
	else
	{
		qDebug()<< QString::fromUtf8("好像数据库插入有问题")<<endl;
		qDebug()<<query.lastError().text()<<endl;
	}
}

void ThreadTcpSocket::clientInitFriendsInfomationMessage(QDataStream &in, qint32 sourceID)
{//收到客户端要求获取好友列表信息的message
	emit signalTcpConnectInfor(sourceID, p_socketHandle);
	//又是一段又臭又长的变量声明
	qint32 friend_id;
	QString friend_remarks;
	QString friend_nickName;
	QString friend_sex;
	QDate friend_birthday;
	QString friend_mail;
	QString friend_mood;
	QByteArray friend_headImage;

	///////开始查询数据库
	QSqlQuery query(db);
	////////////////////////////////////////////////
	//	qDebug()<<"sourceID"<<sourceID<<endl;
	////////////////////////////////////////////////
	QString sqlSelectCommand = QString("select * from friend_infor_%1").arg(sourceID);
	/////////////////////////////////////////////////////
	//	qDebug()<<"sqlSelectCommand"<<endl<<sqlSelectCommand<<endl;
	/////////////////////////////////////////////////////
	query.exec(sqlSelectCommand);

	QByteArray bytes;
	QDataStream out(&bytes, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out << (qint32)0;		//站空间用
	out << (qint32)1;		//目标id 也就是让客户端分辨到底这个是什么的标示 1代表初始化好友列表
	out << (qint32)0;		//源id   0 代表服务器

	while (query.next())
	{
		friend_id = query.value(0).toInt();
		friend_nickName = query.value(1).toString();
		friend_sex = query.value(2).toString();
		friend_birthday = query.value(3).toDate();
		friend_mail = query.value(4).toString();
		friend_mood = query.value(5).toString();
		friend_headImage = query.value(6).toByteArray();
		friend_remarks = query.value(7).toString();
		////////////////////////////////////////////////////////////////////
		//			qDebug()<<friend_id<<friend_nickName<<friend_sex<<friend_birthday<<friend_mail<<friend_mood<<friend_remarks;
		///////////////////////////////////////////////////////////////////
		//////////然后接下来就是传输给客户端了
		out << friend_id<<friend_nickName<<friend_sex<<friend_birthday<<friend_mail
			<<friend_mood<<friend_headImage<<friend_remarks;
	}
	out.device()->seek(0);
	out << (qint32)(bytes.size() - (qint32)sizeof(qint32));
	//////////////////////////////////////////////////////////////////
	//	qDebug()<<"send client friend infor to client"<<endl;
	///////////////////////////////////////////////////////////////////
	p_tcpSocket->write(bytes);
}
void ThreadTcpSocket::clientEditRemarksMessage(QDataStream &in)
{//收到客户端修改备注的消息
	QString userID;
	qint32 friendID;
	QString newRemark;
	in>>userID>>friendID>>newRemark;
	QSqlQuery query(db);
	QString strCmd;
	strCmd+="UPDATE friend_"+userID+" SET friend_remarks='"+newRemark+"' WHERE friend_id="+QString::number(friendID);
	//	qDebug()<<strCmd;
	query.exec(strCmd);
}

void ThreadTcpSocket::clientDeleteFriendMessage(QDataStream &in)
{//收到客户端删除好友的消息
	QString userID;
	qint32 friendID;
	in>>userID>>friendID;
	QSqlQuery query(db);
	QString strCmd;
	strCmd+="DELETE FROM friend_"+userID+" WHERE friend_id="+QString::number(friendID);
	//	qDebug()<<strCmd;
	query.exec(strCmd);
}

void ThreadTcpSocket::clientLoginMessage(QDataStream &in)
{//收到客户端登录的消息
	QString clientLoginName;
	QTime clientLoginTime;
	QString clientLoginPasswd;
	QPixmap clientLoginHeadImage;
	QByteArray clientImageData;
	in >> clientLoginName >> clientLoginTime >> clientLoginPasswd >> clientImageData;
	clientLoginHeadImage.loadFromData(clientImageData, "PNG");
	//	qDebug()<<QString::fromUtf8("客户端登录的时间是")<<clientLoginTime<<endl;
	//	qDebug()<<"clientLoginName"<<clientLoginName<<endl;
	///////////////////////现在拿到账户了,就可以开始查询数据库了
	//	qDebug()<<QString::fromUtf8("客户端登录的信息")<<endl;
	QSqlQuery query(db);
	QString sqlSelectCommand = QString("select user_password, user_id from user_infor where user_account = '%1'").arg(clientLoginName);
	//	qDebug()<<sqlSelectCommand<<endl;
	query.exec(sqlSelectCommand);
	QString userPasswd;
	QString userPasswdMD5;
	QString clientID;
	if(query.next())
	{
		qDebug()<<QString::fromUtf8("在数据库中存在所登录的账户");
		userPasswd = query.value(0).toString();
		///////////////在这里找到ID
		//////////////////////////////////////////////////////////////////
		clientID = query.value(1).toString();
		//////////////////////////////////////////////////////
		//qDebug()<<"userPasswd"<<userPasswd<<"clientID"<<clientID<<endl;
		//根据client发来的Tcp数据报得到客户端登录的帐户名,然后查询数据库得到密码
		//现在是要将密码加密然后返回客户端

		emit signalTcpConnectInfor(clientID.toInt(), p_socketHandle);

		//		qDebug()<<"userId "<<clientID.toInt()<<QString::fromUtf8(" threadTcpSocket this 地址是:")<<this<<endl;
		////////////////////////////////////////////////////////////////////////////
		//hash加盐
		userPasswd += clientLoginTime.toString();
		//		qDebug()<<"userPasswd += clientLoginTime.toString() = "<<userPasswd<<endl;
		userPasswdMD5 = QCryptographicHash::hash(userPasswd.toAscii(), QCryptographicHash::Md5).toHex();
		//		qDebug()<<userPasswdMD5<<endl;
		////////////////////////////////////////////////////////////////////////////
		//nice 现在可以比较数据库中加密之后的密码和客户端传来的密码是否一致了
		if(clientLoginPasswd == userPasswdMD5)
		{//如果一致~nice 那就反馈给客户端可以登录,并且把登录时候的头像更新到服务器  ---(前提此头像不为空)
			QByteArray datagram;
			QDataStream out(&datagram, QIODevice::WriteOnly);
			out << QString("CLIENT_LOGIN_SUCCESS") << clientID;	//验证正确,你可以登录了
			p_tcpSocket->write(datagram);

			/////////////////////////////////////////////////////////////////////////////////
			//把头像更新到服务器
			if(!clientLoginHeadImage.isNull())
			{//如果传来的头像不为空就更新到数据库
				qDebug()<<QString::fromUtf8("客户端登录时头像不为空,将登录头像更新到数据库");
				QSqlQuery query(db);
				query.prepare(QString("update user_infor set user_headimage=? where user_account = '%1'").arg(clientLoginName));
				query.bindValue(0,clientImageData);
				if(query.exec())
				{
					qDebug()<<QString::fromUtf8("登录时所使用的头像正确更新到数据库");
				}
				else
				{
					qDebug()<<QString::fromUtf8("登录时所使用的头像更新到数据库时发生了错误");
				}
			}
			else
			{//如果传来的头像为空,就不作为
				qDebug()<<QString::fromUtf8("客户端登录时头像为空,所以没有更新到数据库");
			}

			////////////////////////////////////////////////////////////////////////////////

			/////登录了就顺便把聊天用的好友信息的表搞出来吧,反正第一次的时候创建,创建之后以后再创建也是会失败的
			QString userFriendTableName = QString("friend_%1").arg(clientID);
			//////////////////////////////////////////////////////////////////////////////
			//		qDebug()<<"userFriendTableName" <<userFriendTableName<<endl;
			//////////////////////////////////////////////////////////////////////////////
			QString cmd = QString("create table %1 ("
								  "friend_id INT(11) primary key,"
								  "friend_remarks VARCHAR(10) )").arg(userFriendTableName);
			//////////////////////////////////////////////
			//		qDebug()<<cmd;
			////////////////////////////////////////////
			bool databaseCreator = query.exec(cmd);
			if(databaseCreator)
			{
				//////////////////////////////////////////////////////////////////
				//			qDebug()<< "haoyouliebiao creator success!!!!!!!11"<<endl;
				/////////////////////////////////////////////////////////////////
			}
			else
			{
				/////////////////////////////////////////////////////////////////////
				//			qDebug()<<"haoyouliebiao creator flase!!!!!!!11"<<endl;
				//////////////////////////////////////////////////////////////////
			}
			/////////这里是创建好友信息的视图
			QString sqlCreateViewCommand = QString("CREATE VIEW `friend_infor_%1` (friend_id, friend_nickname, friend_sex, friend_birthday, friend_mail, friend_mood, friend_headimage, friend_remarks)"
												   " AS SELECT user_id, user_nickname, user_sex, user_birthday, user_mail, user_mood, user_headimage, friend_remarks "
												   " FROM user_infor_view, friend_%1"
												   " WHERE user_infor_view.user_id = friend_%1.friend_id").arg(clientID);
			//		qDebug()<<"sqlCreateViewCommand"<<endl<<sqlCreateViewCommand<<endl;
			query.exec(sqlCreateViewCommand);
		}
		else
		{//若不一致则登录失败
			QByteArray datagram;
			QDataStream out(&datagram, QIODevice::WriteOnly);
			out << QString("CLIENT_LOGIN_FALES") << clientID;
			p_tcpSocket->write(datagram);
		}
	}
	else
	{//这里是在数据库中查找不到账户数据的
		qDebug()<<QString::fromUtf8("所登录的的账户在数据库中不存在");
		QByteArray datagram;
		QDataStream out(&datagram, QIODevice::WriteOnly);
		out << QString("USER_NO_EXIST");
		p_tcpSocket->state();
		p_tcpSocket->write(datagram);
	}
}

void ThreadTcpSocket::registerResultToClient(const QString &str)
{//这个函数是返回注册信息给客户端的
	//QHostAddress clientIpAddress = p_tcpSocket->peerAddress();
	//quint16 clientPort = p_tcpSocket->peerPort();

	QByteArray registerResult = str.toAscii();
	qDebug()<<QString::fromUtf8("发送给客户端回馈信息之前获取到的客户端Ip地址是")<<clientAddress<<endl;
	//	udpSocket->writeDatagram(registerResult.data(), clientAddress, 8000);
	//udpSocket->writeDatagram(registerResult, QHostAddress("127.0.0.1"), 8000);
	udpSocket->writeDatagram(registerResult.data(), registerResult.size(), p_tcpSocket->peerAddress(), 8000);
	p_tcpSocket->close();
	//	qDebug()<<clientIpAddress.toString();
}

//这是用来传递个人信息
void ThreadTcpSocket::clientPersonalMessage(QDataStream &in, qint32 sourceID)
{
	QSqlQuery query(db);
	QString strCmd;
	strCmd="SELECT * FROM user_infor_view WHERE user_id="+QString::number(sourceID);
	//	qDebug()<<strCmd;
	query.exec(strCmd);
	if(query.next())//查询到了就返回
	{
		//小准备
		QByteArray bytes;
		QDataStream streamData(&bytes,QIODevice::WriteOnly);
		streamData<<(qint32)0;
		streamData<<(qint32)2<<(qint32)0;//2是告诉客户端准备读取个人信息
		//准备需要回发的个人信息
		streamData<<query.value(0).toInt();
		streamData<<query.value(1).toString();
		streamData<<query.value(2).toString();
		streamData<<query.value(3).toDate();
		streamData<<query.value(4).toString();
		streamData<<query.value(5).toString();
		streamData<<query.value(6).toByteArray();
		//计算个人信息的大小
		streamData.device()->seek(0);
		streamData<<(qint32)(bytes.size()-sizeof(qint32));
		//回发
		p_tcpSocket->write(bytes);
		//p_tcpSocket->abort();
	}
}

void ThreadTcpSocket::clientFindPasswdMessage(QDataStream &in)
{
	qDebug()<<QString::fromUtf8("客户端找回密码信息");
	qint32 messageType;//标志发送给用户的信息是用户不存在0，存在并设置了密宝问题1，存在但没设置密宝问题2。

	in >> userName;
	qDebug()<<"find"<<userName;
	query = new QSqlQuery(db);
	bool ok;
	userNameInt = userName.toInt(&ok);
	qDebug()<<"gggggg"<<userNameInt;
	QString str =QString("SELECT * FROM user_infor WHERE user_id = %1 or user_account = '%2'").arg(userNameInt).arg(userName);
	query->exec(str);//查找id还有账户,考虑还算周到啊......
	if(query->next())
	{//用户存在 messageType的值就应该只是1或者0 1代表填写有密宝问题,0代表没有密保
		//		qDebug()<<QString::fromUtf8("查找到用户信息,现在开始读数据");
		q1 = query->value(9).toString();
		q2 = query->value(10).toString();
		q3 = query->value(11).toString();
		messageType = query->value(12).toInt();

		////////////////////////////////////
		qDebug()<<q1<<" "<<q2<<" "<<q3;
		//		qDebug()<<a1<<" "<<a2<<" "<<a3;
		//		qDebug()<<"messageType: "<<messageType;
		///////////////////////////////////
		if(1 == messageType)//用户设置了密宝问题，将问题发送到客户端。
		{
			//out << messageType;

		}
		else
		{//用户没有设置密宝问题。
			q1 = "00";
			q2 = "00";
			q3 = "00";
			//	out<<messageType;
		}
	}
	else
	{//查询不到数据说明该账户不存在
		messageType = 2; //2代表不存在  其实以后可以改为#define USER_NOT_EXIT 2;

		q1 = "00";
		q2 = "00";
		q3 = "00";

	}
	QByteArray datagram;
	QDataStream out(&datagram, QIODevice::WriteOnly);
	out << (qint32)0;
	out<<1;//表示目的即返回问题
	out<<messageType<<q1<<q2<<q3;
	out.device()->seek(0);
	out << (qint32)(datagram.size() - (qint32)(sizeof(qint32)));
	qDebug()<<"find passwd"<<messageType<<q1<<q2<<q3;
	qDebug()<<datagram.size();
	p_tcpSocket->write(datagram);

}

void ThreadTcpSocket::clientNewPasswdMessage(QDataStream &in)
{
	qDebug()<<QString::fromUtf8("收到客户端新的密码");
	in >> passwd;
	//	qDebug()<<QString::fromUtf8("客户端的密码要更新为:")<<passwd<<endl;
	passwd = QCryptographicHash::hash(passwd.toAscii(), QCryptographicHash::Md5).toHex();
	queryPasswd = new QSqlQuery(db);
	queryPasswd->exec(QString("UPDATE user_infor SET user_password = '%3' WHERE user_id = %4 or user_account = '%5' " ).arg(passwd).arg(userNameInt).arg(userName));
	QByteArray byte;
	QDataStream out(&byte, QIODevice::WriteOnly);
	out<<(qint32)0;
	out<<3;
	if(queryPasswd->isActive())
	{
		out<<1;
		qDebug()<<QString::fromUtf8("似乎密码的更新操作没有出现什么差错");
	}
	else
	{
		out<<2;
	}
	out.device()->seek(0);
	out << (qint32)(byte.size() - (qint32)sizeof(qint32));
	p_tcpSocket->write(byte);
}

void ThreadTcpSocket::slotTcpConnectDisconnected()
{
	emit signalTcpConnectDisconnected(this, p_socketHandle,
									  p_tcpSocket, 0);//0这个是暂时放的而已还没有用到
}

void ThreadTcpSocket::clientSeekFriend(QDataStream &ins)
{
	QString seekFriendId;
	ins>>seekFriendId;
	qDebug()<<seekFriendId;
	bool ok;
	int seekFriendIdInt = seekFriendId.toInt(&ok);
	QString seekFriendIdStr = QString::number(seekFriendIdInt);
	if(seekFriendIdStr == seekFriendId && seekFriendId.length() == 8)
	{//因为群Id是8位int数。
		//seekGroupToSendClient(seekFriendIdInt);
		qDebug()<<"Group~~~";
		return;
	}
	//查找的是好友。
	QSqlQuery seekQuery(db);
	QString selectStr = QString("SELECT user_account, user_nickname, user_sex, user_birthday, user_mail, user_mood,user_headimage, user_id FROM user_infor WHERE user_id=%1 or user_account= '%2' ").arg(seekFriendIdInt).arg(seekFriendId);
	seekQuery.exec(selectStr);
	int addfriendId;
	QString userName_seek;
	QString userNickname_seek;
	QString userSex_seek;
	QDate userBirthday_seek;
	QString userMail_seek;
	QString userMood_seek;
	QByteArray userHeadimage_seek;
	if(seekQuery.next())
	{//查找对象存在。
		userName_seek = seekQuery.value(0).toString();
		userNickname_seek = seekQuery.value(1).toString();
		userSex_seek = seekQuery.value(2).toString();
		userBirthday_seek = seekQuery.value(3).toDate();
		userMail_seek = seekQuery.value(4).toString();
		userMood_seek = seekQuery.value(5).toString();
		userHeadimage_seek = seekQuery.value(6).toByteArray();
		addfriendId = seekQuery.value(7).toInt(&ok);
	}
	else
	{//查找对象不存在。
		addfriendId = 0;
	}

	QByteArray SeekByteFriendInformarion;
	QDataStream out(&SeekByteFriendInformarion, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	int destinationId = 7 ;
	int sourceId = 0;
	//sourceId这里是添加好友的请求者，原本接受信息时是源，现在回复信息变成目标
	out<<qint32(0)<<destinationId<<sourceId<<addfriendId<<userName_seek
	  <<userNickname_seek<<userSex_seek<<userBirthday_seek<<userMail_seek<<userMood_seek<<userHeadimage_seek;
	out.device()->seek(0);
	out<<qint32(SeekByteFriendInformarion.size() - sizeof(qint32));
	qDebug()<<QString::fromUtf8("查找到好友的资料")<<addfriendId<<userName_seek
		   <<userNickname_seek<<userSex_seek<<userBirthday_seek<<userMail_seek<<userMood_seek;

	p_tcpSocket->write(SeekByteFriendInformarion);
}
//读取有请求添加好友。
void ThreadTcpSocket::readAddFriendFromClient(QDataStream &in, int sourceId,QTcpSocket *p_tcpSocket)
{//sourceId是请求者。
	int friendId;
	in>>friendId;
	QSqlQuery seekHaveAdd(db);

	seekHaveAdd.exec(QString("SELECT friend_id FROM friend_%1 WHERE friend_id = %2").arg(sourceId).arg(friendId));
	QByteArray havaAddOrNo;
	QDataStream out(&havaAddOrNo, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	if(seekHaveAdd.next())
	{//已经是好友拉，不需要再添加。
		qDebug()<<"have add -----have add";

		//目的，源，好友。
		out<<qint32(0)<<10<<0;
		out.device()->seek(0);
		out<<qint32(havaAddOrNo.size() - sizeof(qint32));
		p_tcpSocket->write(havaAddOrNo);
		return;
	}
	out<<qint32(0)<<11<<0;
	out.device()->seek(0);
	out<<qint32(havaAddOrNo.size() - sizeof(qint32));
	p_tcpSocket->write(havaAddOrNo);

	emit signalGetConnect(friendId,this);
	qDebug()<<"add friend ID:"<<friendId;
	QDateTime time;
	time = QDateTime::currentDateTime();
	if(NULL != tcpSocketConnect && true == userLine)
	{
		qDebug()<<"tcpSocket != NULL";
		QString strTime = time.toString("yyyy-MM-dd  hh:mm:ss");
		sendAddFriendToClient( strTime, sourceId ,  ADDFRIENDREQUEST,  friendId);
	}
	else//不在线，把信息添加到个人信息转发表
	{
		qDebug()<<"tcpSocket == NULL";
		//QString souerStr = QString::number(sourceId);
		insertInforToUserchat( time, ADDFRIENDREQUEST ,  sourceId,  friendId);
		///////////添加被邀请添加信息添加到个人信息数据库/////////
	}
}

void ThreadTcpSocket::clientModifyPersonalInfoMessage(QDataStream &in, qint32 sourceID)
{ //更新个人信息功能，大漠写的
	QString nickname;
	QString sex;
	QString birthday;
	QString mail;
	QString mood;
	QByteArray byteHeadImage;
	in>>nickname>>sex>>birthday>>mail>>mood>>byteHeadImage;
	//写入数据库
	QSqlQuery query(db);
	if(!query.prepare("update user_infor set user_nickname=?,user_sex=?,user_birthday=?,user_mail=?,user_mood=?,user_headimage=? where user_id=?"))
	{
		qDebug()<<"prepare()==false:"<<query.lastError().text();
		return;
	}
	query.bindValue(0,nickname);
	query.bindValue(1,sex);
	query.bindValue(2,birthday);
	query.bindValue(3,mail);
	query.bindValue(4,mood);
	query.bindValue(5,byteHeadImage);
	query.bindValue(6,sourceID);
	if(query.exec())
	{
		qDebug()<<"update OK!";
	}
	else
	{
		qDebug()<<"exec()==false:"<<query.lastError().text();
	}
}

void ThreadTcpSocket::getConnectFromThreadTcpServer(ThreadTcpSocket *p, bool flag)
{
	//tcpSocket = new QTcpSocket;
	if(NULL != p &&  true == flag)
	{
		tcpSocketConnect= (ThreadTcpSocket const *)p;
		// tcpSocket = p->p_tcpSocket;
		userLine = true;
		qDebug()<<QString::fromUtf8("找到对应连接了");
	}
	else
	{
		tcpSocketConnect = NULL;
		userLine = false;
		qDebug()<<QString::fromUtf8("没有找到相对应的连接");
	}
}
//发送好友请求信息给相应的用户。
void ThreadTcpSocket::sendAddFriendToClient(QString strTime, int sourceId ,  int destination, int  friendId)
{//sourceId请求者。
	emit signalGetConnect(friendId,this);
	if(NULL == tcpSocketConnect)
	{
		qDebug()<<QString::fromUtf8("添加好友的没有获取链接")<<"friendId"<<friendId;
		return;
	}
	QString requesterAccount;
	QString requesterNickname;
	QSqlQuery requesterQuery(db);
	//查找请求者的数据给用户。
	requesterQuery.exec(QString("SELECT user_account,user_nickname FROM user_infor WHERE user_id = %1").arg(sourceId));
	if(requesterQuery.next())
	{
		requesterAccount = requesterQuery.value(0).toString();
		requesterNickname = requesterQuery.value(1).toString();
	}
	QByteArray requestInformationToClient;
	QDataStream out(&requestInformationToClient, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out<<qint32(0)<<destination<<0;//目的和源
	out<<friendId<<sourceId<<requesterAccount<<requesterNickname<<strTime;
	//tcpSocket->write(requestInformationToClient);
	qDebug()<<"add size"<<requestInformationToClient.size();
	tcpSocketConnect->p_tcpSocket->write(requestInformationToClient);
	qDebug()<<QString::fromUtf8("查找请求者资料发给要加的好友")<<"friend:"<<friendId<<sourceId<<requesterAccount<<requesterNickname<<strTime;
}

//information_type是8表示有好友请求，9表示是好友同意添加请求。
void ThreadTcpSocket::insertInforToUserchat(QDateTime time, int infor, int senderId, int acceptId)
{
	qDebug()<<"Insert"<<time<<infor<<senderId<<acceptId;
	QSqlQuery insertAddFriendInforToDatabase(db);
	insertAddFriendInforToDatabase.prepare("INSERT INTO add_friend(infor_date, information_type, sendaccount_id, acceptaccount_id)  VALUES(?,?,?,?)");
	insertAddFriendInforToDatabase.addBindValue(time);
	insertAddFriendInforToDatabase.addBindValue(infor);
	insertAddFriendInforToDatabase.addBindValue(senderId);
	insertAddFriendInforToDatabase.addBindValue(acceptId);
	insertAddFriendInforToDatabase.exec();
}
//读取同意添加好友信息,并查找请求者是否在线，在线发送，不在存在转发表。
void ThreadTcpSocket::readAgreeAddFriendInfor(QDataStream &in, int friendId)
{
	int requesterId;
	in>>requesterId;
	//把好友插入数据库。
	addFriendInDatabase(requesterId, friendId);
	addFriendInDatabase(friendId, requesterId);
	emit signalGetConnect(requesterId,this);
	qDebug()<<"aaaaaaaaa agree add freind:"<<friendId<<requesterId;
	QDateTime time = QDateTime::currentDateTime();

	if(NULL != tcpSocketConnect && true == userLine)
	{
		qDebug()<<"tcpSocket != NULL";
		sendAgreeAddfriendToClient(time, requesterId, friendId);
		qDebug()<<"have send";
	}
	else
	{//调用插入数据库函数，把好友同意添加好友请求放入数据库。
		qDebug()<<"tcpSocket == NULL";
	   //AGREEADDFRIEND == 9表示已经同意添加好友。
		insertInforToUserchat(time,  AGREEADDFRIEND, friendId, requesterId);
	}
}
//可以修改看接收者是否有未读信息n，n改为y。
void ThreadTcpSocket::addFriendInDatabase(int requesterID, int friendId)
{
	QSqlQuery isHavaAdd(db);
	QString str = QString("SELECT * FROM friend_%1 WHERE friend_id = %2").arg(friendId).arg(requesterID);
	isHavaAdd.exec(str);
	if(isHavaAdd.next())
	{
		return;
	}
	QSqlQuery addFriend(db);
	addFriend.prepare(QString("INSERT INTO friend_%1(friend_id)VALUES(?)").arg(friendId));
	addFriend.addBindValue(requesterID);
	addFriend.exec();
}

void ThreadTcpSocket::clientSendChatMessage(qint32 desID, QDataStream &in)
{//这里是处理客户端发来的聊天信息
	//第一步要现判断目的ID是否在线,这里根据是否能获取目的ID与服务器的链接作为判断
	emit signalGetConnect(desID, this);
	qint32 tcpDadaSize;
	QDateTime time;
	QString str;
	in >> imgCount >> imgTempDataByte >> html >> str >>time;
	qDebug() << tcpDadaSize << id_src << id_des << imgCount << html << str <<time;
	//qDebug()<<imgTempDataByte.size();
	if(userLine)
	{
		qDebug()<<"目的ID在线, 华丽的转发吧~~~";
		sendChatMessageToClient(time, str);
	}
	else
	{
		qDebug()<<"那家伙不在线,先存起来,等他上线了在给他";
		saveChatMessageToDatabase(time, str);
	}
}

void ThreadTcpSocket::sendChatMessageToClient(QDateTime time, const QString &chat)
{
	QByteArray byte;
	QDataStream out(&byte, QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	out << (qint32)0;
	out << (qint32)id_des;
	out << (qint32)id_src;
	out << imgCount << imgTempDataByte << html << chat <<time;
	out.device()->seek(0);
	qint32 size = (qint32)(byte.size() - sizeof(qint32));
	out << size;
	qDebug()<<"发信息给另外一个客户端的时候数据大小是" <<size;
	tcpSocketConnect->p_tcpSocket->write(byte);

}

void ThreadTcpSocket::saveChatMessageToDatabase(QDateTime time, const QString &chat)
{//目标ID不在线的时候就把这个存储起来~~~
	QSqlQuery query(db);
	bool test = query.prepare("insert into user_chat  (infor_id_src, infor_id_des, infor_image_count, infor_image_data, infor_html, infor_date, infor_string)"
							  "values(?, ?, ?, ?, ?, ?,?)");
	if(test)
	{
		query.addBindValue(id_src);
		query.addBindValue(id_des);
		query.addBindValue(imgCount);
		query.addBindValue(imgTempDataByte);
		query.addBindValue(html);
		query.addBindValue(time);
		query.addBindValue(chat);
		query.exec();
	}
	else
	{
		qDebug()<<"聊天信息存储到数据库出错";
		qDebug()<<query.lastError().text();
	}
}

//大漠写的,用于返回新的好友信息的
void ThreadTcpSocket::clientRequireUpdateFriendInfo(QDataStream &in, qint32 sourceID)
{
	qint32 friendID;
	in>>friendID;
	QSqlQuery query(db);
	if(!query.exec(QString("select * from friend_infor_%1 where friend_id=%2").arg(QString::number(sourceID)).arg(QString::number(friendID))))
	{
		qDebug()<<"exec()==false:"<<query.lastError().text();
		return;
	}
	if(!query.next())
	{
		qDebug()<<"no record!!!";
		return;
	}
	//准备发送道具
	QByteArray byteData;
	QDataStream streamData(&byteData,QIODevice::WriteOnly);
	//开始写数据包(好友基本信息)
	streamData<<(qint32)0<<(qint32)3<<(qint32)0;
	streamData<<query.value(0).toInt();
	streamData<<query.value(1).toString();
	streamData<<query.value(2).toString();
	streamData<<query.value(3).toDate();
	streamData<<query.value(4).toString();
	streamData<<query.value(5).toString();
	streamData<<query.value(6).toByteArray();
	streamData<<query.value(7).toString();
	//发送
	streamData.device()->seek(0);
	streamData<<(qint32)(byteData.size()-sizeof(qint32));
	p_tcpSocket->write(byteData);
}

void ThreadTcpSocket::clientPasswdProtectAnswers(QDataStream &in)
{
	QString name;
	int nameInt;
	in>>name>>a1>>a2>>a3;
	QSqlQuery *queryAnswer;
	queryAnswer = new QSqlQuery(db);
	bool ok;
	nameInt = name.toInt(&ok);
	QString answer1;
	QString answer2;
	QString answer3;
	qDebug()<<name<<nameInt;
	QString str =QString("SELECT user_answerone, user_answertwo, user_answerthree FROM user_infor WHERE user_id = %1 or user_account = '%2'").arg(nameInt).arg(name);
	queryAnswer->exec(str);
	QByteArray byte;
	QDataStream out(&byte, QIODevice::WriteOnly);
	out<<(qint32)0;
	out<<2;//分别是目的和源；
	if(queryAnswer->next())
	{
		answer1 = queryAnswer->value(0).toString();
		answer2 = queryAnswer->value(1).toString();
		answer3 = queryAnswer->value(2).toString();
		if(a1 == answer1 && a2 == answer2 && a3 == answer3)
		{//答案正确
			out<<1;//1表示答案正确。
		}
		else
		{
			out<<2;//表示答案不正确。
		}
	}
	else
	{
		out<<3;//查询数据库失败，即没有找到用户。
	}
	out.device()->seek(0);
	out<<(qint32)(byte.size()-sizeof(qint32));
	p_tcpSocket->write(byte);

	qDebug()<<answer1<<answer2<<answer3;
}

void ThreadTcpSocket::sendOffLineMessage(int id)
{

	QSqlQuery query(db);
	QString cmd = QString("select * from user_chat where infor_id_des = %1").arg(QString::number(id));
	emit signalGetConnect(id, this);
	if(!userLine)
	{
		qDebug()<<"要发送离线消息的时候用户突然不在线了";
		return;
	}
	if(query.exec(cmd))
	{//命令成功
		qDebug()<<"要发送离线消息的少年是"<<id;

		while(query.next())
		{
			id_src = query.value(0).toInt();
			id_des = query.value(1).toInt();
			imgCount = query.value(2).toInt();
			imgTempDataByte = query.value(3).toByteArray();
			html = query.value(4).toString();
			QDateTime time = query.value(5).toDateTime();
			QString chat = query.value(6).toString();
			sendChatMessageToClient(time, chat);
		}
		QString cmd = QString("delete from user_chat where infor_id_des = %1").arg(QString::number(id));
		query.exec(cmd);
	}
	else
	{//查询命令出错
		qDebug()<<"查找离线消息的时候命令出错";
		qDebug()<<query.lastError().text();
	}
}
//id就是被添加用户的ID
void ThreadTcpSocket::sendOfLineAddFriend(int id)
{
	qDebug()<<"send of line";
	QSqlQuery addFriendQuery(db);
	QString addStr = QString("SELECT sendaccount_id, infor_date, information_type, id FROM add_friend WHERE acceptaccount_id = %1").arg(id);
	addFriendQuery.exec(addStr);
	int sendtId;
	int inforType;
	QString time;
	bool k;
	int count = 0;
	while(addFriendQuery.next())
	{
		sendtId = addFriendQuery.value(0).toInt(&k);
		QDateTime dateTime = addFriendQuery.value(1).toDateTime();
		time = addFriendQuery.value(1).toDateTime().toString("yyyy-MM-dd  hh:mm::ss");
		inforType = addFriendQuery.value(2).toInt(&k);//这个也是这次.
		if(ADDFRIENDREQUEST == inforType)
		{
			sendAddFriendToClient(time, sendtId ,  inforType, id);
			qDebug()<<"time"<<time;
		}
		else if(AGREEADDFRIEND == inforType)
		{
			qDebug()<<QString::fromUtf8("转发同意添加好友请求");
			sendAgreeAddfriendToClient(dateTime, id, sendtId);
		}
		count++;
		qDebug()<<"count"<<count;
	}
	QString cmd = QString("delete from add_friend where acceptaccount_id = %1").arg(id);
	addFriendQuery.exec(cmd);
}

void ThreadTcpSocket::sendAgreeAddfriendToClient(QDateTime time,int requesterId, int friendId)
{
	emit signalGetConnect(requesterId,this);
	QByteArray addFriendSuccess;
	QDataStream out(&addFriendSuccess,QIODevice::WriteOnly);
	out.setVersion(QDataStream::Qt_4_7);
	QString timeStr = time.toString("yyyy-MM-dd  hh:mm:ss");
	out<<qint32(0)<<9<<0<<requesterId<<friendId<<timeStr;//目的，源，请求者，好友。
	out.device()->seek(0);
	out<<(qint32)(addFriendSuccess.size() - sizeof(addFriendSuccess));
	qDebug()<<"agree size"<<tcpSocketConnect->p_tcpSocket->size();
	tcpSocketConnect->p_tcpSocket->write(addFriendSuccess);

}

void ThreadTcpSocket::clientTransferFile(QDataStream &in, qint32 id_self)
{
	qint32 id_Friend;
	qint64 fileSize;
	QString fileName;
	QByteArray iconByte;
	in >> id_Friend >> fileSize >> fileName >> iconByte;

	qDebug()<<iconByte;
	//qDebug()<< id_self << id_Friend << fileSize << fileName;
	emit signalGetConnect(id_Friend, this);
	if(userLine)
	{//在线就弹他窗,通知有文件是否要接收
		sendTransferFileRequest(id_self, fileSize, fileName, iconByte);
	}
	else
	{//不在线就弹回去,是否发送数据到服务器(离线)
		////////////////
		transFileDesClientOffline(id_self, id_Friend);
	}
}

void ThreadTcpSocket::sendTransferFileRequest(qint32 id_self, qint64 size, QString name, QByteArray iconByte)
{
	QByteArray bytes;
	QDataStream out(&bytes, QIODevice::WriteOnly);

	out << qint32(0);		//占位
	out << qint32(12);	//目的,传输命令
	out << id_self;			//源
	out << size;				//要传输的文件的大小
	out << name;			//要传输的文件的名字
	out << iconByte;

	out.device()->seek(0);
	out << qint32(bytes.size() - sizeof(qint32));
	tcpSocketConnect->p_tcpSocket->write(bytes);
}

void ThreadTcpSocket::transFileDesClientOffline(qint32 id_self, qint32 id_friend)
{ //这里用于通知发送方,文件的接收方不在线
	qDebug()<<"通知发送方,文件的接收方不在线";
	emit signalGetConnect(id_self, this);
	if(userLine)
	{
		QByteArray bytes;
		QDataStream out(&bytes, QIODevice::WriteOnly);
		out.setVersion(QDataStream::Qt_4_7);
		out << qint32(0);       //size
		out << qint32(13);     //des
		out << id_friend;
		out.device()->seek(0);
		out << (qint32)(bytes.size() - sizeof(qint32));

		tcpSocketConnect->p_tcpSocket->write(bytes);
	}
	else
	{
		//........发文文件的源突然不在线了....先暂时不管他
	}
}
