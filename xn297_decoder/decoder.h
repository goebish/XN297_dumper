#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <qudpsocket.h>
#include "maiaXmlRpcClient.h"

class decoder : public QObject
{
    Q_OBJECT

public:
    decoder(QObject *parent);
    ~decoder();

private:
    QUdpSocket *socket;
    uint8_t bit_reverse(uint8_t b_in);
    uint16_t crc16_update(uint16_t crc, uint8_t a, uint8_t bits);
    MaiaXmlRpcClient *rpc;

private slots:
    void readPendingDatagrams();
    void rpc_response(QVariant &);
    void rpc_fault(int, const QString &);
};

#endif // DECODER_H
