/*
** EPITECH PROJECT, 2025
** Rtype
** File description:
** Qt Network UDP Server PoC
*/

#include <QCoreApplication>
#include <QUdpSocket>
#include <QHostAddress>
#include <QDebug>
#include <iostream>

class UdpServer : public QObject {
    Q_OBJECT

public:
    UdpServer(quint16 port, QObject *parent = nullptr) : QObject(parent) {
        socket = new QUdpSocket(this);

        if (!socket->bind(QHostAddress::Any, port)) {
            qCritical() << "Failed to bind to port" << port;
            QCoreApplication::exit(1);
            return;
        }

        std::cout << "Qt Network UDP Server listening on port " << port << std::endl;

        connect(socket, &QUdpSocket::readyRead, this, &UdpServer::processPendingDatagrams);
    }

private slots:
    void processPendingDatagrams() {
        while (socket->hasPendingDatagrams()) {
            QByteArray datagram;
            datagram.resize(socket->pendingDatagramSize());
            QHostAddress sender;
            quint16 senderPort;

            socket->readDatagram(datagram.data(), datagram.size(), &sender, &senderPort);

            QString message = QString::fromUtf8(datagram);
            std::cout << "Received: \"" << message.toStdString() << "\" from "
                      << sender.toString().toStdString() << ":" << senderPort << std::endl;

            // Echo back
            QString response = "Echo: " + message;
            socket->writeDatagram(response.toUtf8(), sender, senderPort);
        }
    }

private:
    QUdpSocket *socket;
};

int main(int argc, char *argv[]) {
    if (argc != 2) {
        std::cerr << "Usage: " << argv[0] << " <port>" << std::endl;
        return 1;
    }

    // QCoreApplication IS required for Qt event loop
    QCoreApplication app(argc, argv);

    quint16 port = QString(argv[1]).toUShort();

    std::cout << "Qt Network UDP Server PoC" << std::endl;
    std::cout << "Note: QCoreApplication is REQUIRED for event loop" << std::endl;

    UdpServer server(port);

    return app.exec();
}

#include "udp_server.moc"
