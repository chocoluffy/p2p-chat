
#include <unistd.h>

#include <QApplication>
#include <QDebug>
#include <QUdpSocket>
#include <QVBoxLayout>

#include "main.hh"

ChatDialog::ChatDialog() {
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
  // init socket and bind to a specific port.
  socket->bind();

  // Register a callback on the textline's returnPressed signal
  // so that we can send the message entered by the user.
  connect(textline, SIGNAL(returnPressed()), this, SLOT(gotReturnPressed()));

  connect(socket, SIGNAL(readyRead()), this, SLOT(recvMessage()));
}

// Ref: http://doc.qt.io/archives/qt-4.8/qudpsocket.html.
void ChatDialog::recvMessage() {
  while (socket->hasPendingDatagrams()) {
    QByteArray datagram;
    datagram.resize(socket->pendingDatagramSize());
    QHostAddress sender;
    quint16 senderPort;

    socket->readDatagram(datagram.data(), datagram.size(), &sender,
                         &senderPort);

    QDataStream dataStream(&datagram, QIODevice::ReadOnly);
    QVariantMap message_map;
    dataStream >> message_map;

    QString chat_str = message_map["ChatText"].toString();
    textview->append(chat_str);
  }
}

void ChatDialog::gotReturnPressed() {
  // Initially, just echo the string locally.
  // Insert some networking code here...
  qDebug() << "FIX: send message to other peers: " << textline->text();
  textview->append(textline->text());

  socket->send(socket->cur_port, textline->text());

  // Clear the textline to get ready for the next input message.
  textline->clear();
}

NetSocket::NetSocket() {
  // Pick a range of four UDP ports to try to allocate by default,
  // computed based on my Unix user ID.
  // This makes it trivial for up to four P2Papp instances per user
  // to find each other on the same host,
  // barring UDP port conflicts with other applications
  // (which are quite possible).
  // We use the range from 32768 to 49151 for this purpose.
  myPortMin = 32768 + (getuid() % 4096) * 4;
  myPortMax = myPortMin + 3;
}

bool NetSocket::bind() {
  // Try to bind to each of the range myPortMin..myPortMax in turn.
  for (int p = myPortMin; p <= myPortMax; p++) {
    if (QUdpSocket::bind(p)) {
      cur_port = p;
      qDebug() << "bound to UDP port " << p;
      return true;
    }
  }

  qDebug() << "Oops, no ports in my default range " << myPortMin << "-"
           << myPortMax << " available";
  return false;
}

void NetSocket::send(quint16 port, QString msg) {
  // try to send the message to my own port.
  QVariantMap qvMap;
  qvMap.insert("ChatText", msg);
  QByteArray q_byte;
  QDataStream q_data_stream(&q_byte, QIODevice::WriteOnly);
  q_data_stream << qvMap;
  writeDatagram(q_byte, QHostAddress::LocalHost, port);
}

int main(int argc, char **argv) {
  // Initialize Qt toolkit
  QApplication app(argc, argv);

  // Create an initial chat dialog window
  ChatDialog dialog;
  dialog.show();

  // Create a UDP network socket
  NetSocket sock;
  if (!sock.bind()) exit(1);

  // Enter the Qt main loop; everything else is event driven
  return app.exec();
}
