#ifndef SERVERMAINDIALOG_H
#define SERVERMAINDIALOG_H

#include "ui_ServerMainDialog.h"
#include "ThreadTcpServer.h"
#include <QUdpSocket>
#include <QSqlDatabase>
#include <QSqlQuery>

class ServerMainDialog : public QDialog, private Ui::ServerMainDialog
{
    Q_OBJECT
    
public:
    explicit ServerMainDialog(QWidget *parent = 0);
private slots:
	void on_pushButtonMiinSize_clicked();

private:
	void connectToDatabase();
private:
	QSqlDatabase db;
	ThreadTcpServer *threadTcpServer;
	//////////////////////////////////////
};

#endif // SERVERMAINDIALOG_H
