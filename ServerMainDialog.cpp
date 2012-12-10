#include "ServerMainDialog.h"
#include <QDebug>
#include <QSqlError>
ServerMainDialog::ServerMainDialog(QWidget *parent) :
	QDialog(parent)
{
	setupUi(this);
	connectToDatabase();
	threadTcpServer = new ThreadTcpServer(db,this);
	if(!threadTcpServer->listen(QHostAddress::Any,8888))
	{
		qDebug()<<"threadTcpServer have error!!";
		qDebug()<<threadTcpServer->errorString();
		close();
	}
}

void ServerMainDialog::connectToDatabase()
{
	db = QSqlDatabase::addDatabase("QMYSQL", "tcpServer");
	db.setHostName("127.0.0.1");
	db.setDatabaseName("chat");
	db.setUserName("root");
		db.setPassword("small");
	if(db.open())
	{
		return;
	}
	else
	{
		qDebug()<< db.lastError().text();

		return;
	}
}

void ServerMainDialog::on_pushButtonMiinSize_clicked()
{
	this->hide();
}
