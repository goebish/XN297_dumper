#ifndef DECODER_H
#define DECODER_H

#include <QObject>
#include <qudpsocket.h>

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

private slots:
    void readPendingDatagrams();
};

#endif // DECODER_H
