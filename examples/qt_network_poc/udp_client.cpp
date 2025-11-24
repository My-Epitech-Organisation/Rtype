/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Qt Network UDP Client PoC
*/

#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QTimer>
#include <QDebug>
#include <iostream>
#include <vector>

class UdpClient : public QObject {
    Q_OBJECT

public:
    UdpClient(const QString &host, quint16 port, QObject *parent = nullptr)
        : QObject(parent), serverAddress(host), serverPort(port), messageIndex(0) {

        socket = new QUdpSocket(this);

        std::cout << "Qt Network UDP Client connecting to "
                  << host.toStdString() << ":" << port << std::endl;

        testMessages = {
            "Hello from Qt Network!",
            "Test message 2",
            "Benchmark test"
        };

        connect(socket, &QUdpSocket::readyRead, this, &UdpClient::processResponse);

        // Start sending messages
        QTimer::singleShot(100, this, &UdpClient::sendNextMessage);
    }

private slots:
    void sendNextMessage() {
        if (messageIndex >= testMessages.size()) {
            std::cout << "All messages sent successfully!" << std::endl;
            QTimer::singleShot(500, qApp, &QCoreApplication::quit);
            return;
        }

        QString message = QString::fromStdString(testMessages[messageIndex]);
        std::cout << "Sending: \"" << message.toStdString() << "\"" << std::endl;

        socket->writeDatagram(message.toUtf8(), QHostAddress(serverAddress), serverPort);
        messageIndex++;

        // Send next message after a delay
        QTimer::singleShot(100, this, &UdpClient::sendNextMessage);
    }

    void processResponse() {
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket->pendingDatagramSize());

            socket->readDatagram(datagram.data(), datagram.size());

            QString response = QString::fromUtf8(datagram);
            std::cout << "Received: \"" << response.toStdString() << "\"" << std::endl;
        }
    }

private:
    QUdpSocket *socket;
    QString serverAddress;
    quint16 serverPort;
    std::vector<std::string> testMessages;
    size_t messageIndex;
};

int main(int argc, char *argv[]) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <host> <port>" << std::endl;
        return 1;
    }

    // QCoreApplication IS required for Qt event loop
    QCoreApplication app(argc, argv);

    QString host = argv[1];
    quint16 port = QString(argv[2]).toUShort();

    std::cout << "Qt Network UDP Client PoC" << std::endl;
    std::cout << "Note: QCoreApplication is REQUIRED for event loop" << std::endl;

    UdpClient client(host, port);

    return app.exec();
}

#include "udp_client.moc"
