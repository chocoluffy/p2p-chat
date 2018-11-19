
#include <unistd.h>

#include <QVBoxLayout>
#include <QApplication>
#include <QDebug>
#include <QTime>

#include "main.hh"

ChatDialog::ChatDialog()
{
	setWindowTitle("P2Papp");

	// Read-only text box where we display messages from everyone.
	// This widget expands both horizontally and vertically.
	textview = new QTextEdit(this);
	textview->setReadOnly(true);

	// Small text-entry box the user can enter messages.
	// This widget normally expands only horizontally,
	// leaving extra vertical space for the textview widget.
	//
	// You might change this into a read/write QTextEdit,
	// so that the user can easily enter multi-line messages.
	textline = new QLineEdit(this);

	// Lay out the widgets to appear in the main window.
	// For Qt widget and layout concepts see:
	// http://doc.qt.nokia.com/4.7-snapshot/widgets-and-layouts.html
	QVBoxLayout *layout = new QVBoxLayout();
	layout->addWidget(textview);
	layout->addWidget(textline);
	setLayout(layout);

    socket = new NetSocket();
    socket->bind();

    // Use the port number that we bind as myID. This only makes sense when we
    // run this program locally.
    socket->myID = QString::number(socket->getMyPort());
    qDebug() << "[init] my port: " << socket->myID;
    // mySeq = 1;

    // Register a callback on the local socket to receive any incoming msg
    connect(socket, SIGNAL(readyRead()), this, SLOT(gotMsgReceived()));

	// Register a callback on the textline's returnPressed signal
	// so that we can send the message entered by the user.
    connect(textline, SIGNAL(returnPressed()), this, SLOT(gotReturnPressed()));


    // Set rumor msg timeout range. 


    QTime now = QTime::currentTime();
    qsrand(now.msec());
}

QByteArray ChatDialog::serialize(QString data, QString origin, int seq_no){
    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::ReadWrite);
    QVariantMap dataMap;

    dataMap.insert("ChatText", data);
    dataMap.insert("Origin", origin);
    dataMap.insert("SeqNo", seq_no);
    dataStream << dataMap;
    qDebug() << "[sendMsg] Message: " << dataMap;
    return byteArray;
}

void ChatDialog::sendMsg(QString msg, QString origin, int seq_no, int port)
{
    QByteArray serializedData = serialize(msg, origin, seq_no);
    qDebug() << "[sendMsg] send msg to port: " << port;
    socket->writeDatagram(serializedData, QHostAddress::LocalHost, port);
}

void ChatDialog::saveMsg(QString msg, QString id, int seq){
    QVariantMap rumorMsgEntry;
    QVariantMap tmp;
    
    socket->rumorMsgMap[id].append(msg);
    socket->statusMsgMap[id] = seq + 1;
    qDebug() << "[saveMsg] current rumor msg map: " << socket->rumorMsgMap;
    qDebug() << "[saveMsg] current status msg map: " << socket->statusMsgMap;
}

void ChatDialog::add_new_node(QString id) {
    socket->rumorMsgMap.insert(id, QVector<QString>());
    socket->statusMsgMap.insert(id, 0);
    qDebug() << "[add_new_node] current rumor msg map: " << socket->rumorMsgMap;
    qDebug() << "[add_new_node] current status msg map: " << socket->statusMsgMap;
}


void ChatDialog::gotReturnPressed()
{
    // Insert some networking code here...
    int dest = socket->pickRandNeighbor();
    // qDebug() << "[gotReturnPressed] pick random neighbor: " << dest;

    QString msg = textline->text();
    if (!socket->statusMsgMap.contains(socket->myID)) {
        add_new_node(socket->myID);
    }
    int current_seq_no = socket->statusMsgMap[socket->myID].toUInt();
    saveMsg(msg, socket->myID, current_seq_no);

    sendMsg(msg, socket->myID, current_seq_no, dest);
    textview->append("[" + socket->myID + "]: " + msg);

	// Clear the textline to get ready for the next input message.
	textline->clear();
}

void ChatDialog::gotMsgReceived()
{
    QByteArray buffer;
    buffer.resize(socket->pendingDatagramSize());

    QHostAddress sender;
    quint16 senderPort;
    socket->readDatagram(buffer.data(), buffer.size(), &sender, &senderPort);

    QVariantMap dataMap;
    QDataStream serializer(&buffer, QIODevice::ReadOnly);
    serializer >> dataMap;

    qDebug() << "";

    // Two possible msg type.
    if (dataMap.contains("Want")) {
        // status msg.
        QMap<QString, QVariant> peer_status_map = dataMap["Want"].toMap();
        status_msg_handler(peer_status_map, senderPort);
    }
    else {
        // rumor msg.
        qDebug() << "[gotMsgReceived] rumor msg." << dataMap;
        QString msg = dataMap["ChatText"].toString();
        QString id = dataMap["Origin"].toString();
        int seq = dataMap["SeqNo"].toInt();

        if (!socket->statusMsgMap.contains(id)) {
            add_new_node(id);
        }

        quint32 current_seq_num = socket->statusMsgMap[id].toUInt();
        int if_origin_diff = QString::compare(id, socket->myID, Qt::CaseInsensitive); 
        if (current_seq_num == seq && if_origin_diff) {
            qDebug() << "[gotMsgReceived]  correct seq no and diff origin.";

            // only save the msg when it is desired by the seq num.
            saveMsg(msg, id, seq);

            // Display msg on the window
            textview->append("[" + id + "]: " + msg);

            int dest = socket->pickRandNeighbor();
            qDebug() << "[gotMsgReceived] start monger to: " << dest;

            // start rumor mongering. forward msg to a random neighbor.
            sendMsg(msg, id, current_seq_num, dest);

            // send back status msg.
            socket->send_status_msg(senderPort);
        } else {
            qDebug() << "[gotMsgReceived] receive wrong rumor msg seq no or same origin.";
            qDebug() << "[gotMsgReceived] my current seq no:" << current_seq_num << " but received :" << seq;
            qDebug() << "[gotMsgReceived] my current id:" << socket->myID << " and origin :" << id;

            // send back status msg.
            socket->send_status_msg(senderPort);
        }
    }
}

void NetSocket::send_status_msg(quint16 sender_port){
    QByteArray byteArray;
    QDataStream dataStream(&byteArray, QIODevice::ReadWrite);
    QVariantMap dataMap;
	dataMap.insert("Want", statusMsgMap);
	
	dataStream << dataMap;
	writeDatagram(byteArray, QHostAddress:: LocalHost, sender_port);
}

void ChatDialog::status_msg_handler(QMap<QString, QVariant> peer_status_map, quint16 sender_port) {
    // when receive status msg from peers. 
    // compare myself's status_map with peer_status_map.
        // if peer not contains this origin || less seq num. 
            // me -> peer.
        // else me not contains the origin || less seq num. 
            // send status msg contains "want".
    qDebug() << "";
    qDebug() << "[status_msg_handler]:";
    qDebug() << "my status map:" << socket->statusMsgMap;
    qDebug() << "peer map:" << peer_status_map;
    for(QVariantMap::iterator i = socket->statusMsgMap.begin(); i != socket->statusMsgMap.end(); ++i) {
        QString this_id = i.key();
        quint32 this_seqno = i.value().toUInt();
        if (!peer_status_map.contains(this_id) || peer_status_map[this_id].toUInt() < this_seqno) {
            // me -> peer.
            qDebug() << "[status_msg_handler] me -> peer";
            int msg_id;
            if (!peer_status_map.contains(this_id)) {
                msg_id = 0;
            } else {
                msg_id = peer_status_map[this_id].toUInt();
            }
            sendMsg(socket->rumorMsgMap[this_id][msg_id], this_id, msg_id, sender_port);
            return;
        }
    }

    for(QVariantMap::iterator i = peer_status_map.begin(); i != peer_status_map.end(); ++i) {
        QString this_id = i.key();
        quint32 this_seqno = i.value().toUInt();
        if (!socket->statusMsgMap.contains(this_id) || socket->statusMsgMap[this_id].toUInt() < this_seqno) {
            // peer -> me.
            qDebug() << "[status_msg_handler] peer -> me";
            socket->send_status_msg(sender_port);
            return;
        }
    }
}

NetSocket::NetSocket()
{
	// Pick a range of four UDP ports to try to allocate by default,
	// computed based on my Unix user ID.
	// This makes it trivial for up to four P2Papp instances per user
	// to find each other on the same host,
	// barring UDP port conflicts with other applications
	// (which are quite possible).
	// We use the range from 32768 to 49151 for this purpose.
	myPortMin = 32768 + (getuid() % 4096)*4;
    myPortMax = myPortMin + 2;
    QTime now = QTime::currentTime();
    qsrand(now.msec());
}

bool NetSocket::bind()
{
	// Try to bind to each of the range myPortMin..myPortMax in turn.
	for (int p = myPortMin; p <= myPortMax; p++) {
		if (QUdpSocket::bind(p)) {
			qDebug() << "bound to UDP port " << p;
            myPort = p;
			return true;
		}
	}

	qDebug() << "Oops, no ports in my default range " << myPortMin
		<< "-" << myPortMax << " available";
	return false;
}

int NetSocket::getMyPort()
{
    return myPort;
}

int NetSocket::pickRandNeighbor()
{
    int port = myPort;
    // true random.
    while(port == myPort){
          port = qrand() % 3 + myPortMin; // gap + 1
    }

    // false random.
    // port++;
    // if (port > myPortMax) {
    //     port = myPortMin;
    // }

    return port;
}

int main(int argc, char **argv)
{
	// Initialize Qt toolkit
	QApplication app(argc,argv);

	// Create an initial chat dialog window
	ChatDialog dialog;
	dialog.show();

	// Enter the Qt main loop; everything else is event driven
	return app.exec();
}


