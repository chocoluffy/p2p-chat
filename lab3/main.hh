#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QTextEdit>
#include <QLineEdit>
#include <QUdpSocket>

class NetSocket : public QUdpSocket
{
    Q_OBJECT

public:

    QString myID;
    QMap<QString, QVector<QString> > rumorMsgMap;
    QMap<QString, QVariant> statusMsgMap;

    NetSocket();

    // Bind this socket to a P2Papp-specific default port.
    bool bind();
    int getMyPort();
    int pickRandNeighbor();
    void send_status_msg(quint16 sender_port);
private:
    int myPortMin, myPortMax, myPort;
};

#endif // P2PAPP_MAIN_HH



class ChatDialog : public QDialog
{
	Q_OBJECT

public:
	ChatDialog();
    void status_msg_handler(QMap<QString, QVariant> peer_status_map, quint16 sender_port);

public slots:
    void add_new_node(QString id);
	void gotReturnPressed();
    void gotMsgReceived();
    QByteArray serialize(QString data, QString origin, int seq_no);
    void sendMsg(QString msg, QString origin, int seq_no, int port);
    void saveMsg(QString msg, QString id, int seq);

private:
	QTextEdit *textview;
	QLineEdit *textline;
    NetSocket *socket;
};


