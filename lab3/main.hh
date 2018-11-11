#ifndef P2PAPP_MAIN_HH
#define P2PAPP_MAIN_HH

#include <QDialog>
#include <QLineEdit>
#include <QTextEdit>
#include <QUdpSocket>

class NetSocket : public QUdpSocket {
  Q_OBJECT

 public:
  int cur_port;
  QMap<QString, QVariant> serialized_instance;

  NetSocket();
  void send(QString msg);

  // Bind this socket to a P2Papp-specific default port.
  bool bind();

 private:
  int myPortMin, myPortMax;
};


class ChatDialog : public QDialog {
  Q_OBJECT

 public:
  ChatDialog();

 public slots:
  void gotReturnPressed();
  void recvMessage();

 private:
  NetSocket *socket;
  QTextEdit *textview;
  QLineEdit *textline;
};


#endif  // P2PAPP_MAIN_HH
