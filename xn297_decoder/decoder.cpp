#include "decoder.h"
#include <QNetworkDatagram>

decoder::decoder(QObject *parent)
    : QObject(parent)
{
    printf("listening\n");

    rpc = new MaiaXmlRpcClient(QUrl("http://localhost:1235"), this);

    //xml rpc test
    rpc_set("rpc_var", 1337);

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 1234);

    connect(socket, SIGNAL(readyRead()),
            this, SLOT(readPendingDatagrams()));
}

decoder::~decoder()
{
    //delete rpc;
    delete socket;
}

uint8_t decoder::bit_reverse(uint8_t b_in)
{
    uint8_t b_out = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        b_out = (b_out << 1) | (b_in & 1);
        b_in >>= 1;
    }
    return b_out;
}

uint16_t decoder::crc16_update(uint16_t crc, uint8_t a, uint8_t bits)
{
    const uint16_t polynomial = 0x1021;
    crc ^= a << 8;
    while (bits--) {
        if (crc & 0x8000) {
            crc = (crc << 1) ^ polynomial;
        } else {
            crc = crc << 1;
        }
    }
    return crc;
}

void decoder::readPendingDatagrams()
{
    // xn297 scramble table
    const uint8_t xn297_scramble[] = {
    0xe3, 0xb1, 0x4b, 0xea, 0x85, 0xbc, 0xe5, 0x66,
    0x0d, 0xae, 0x8c, 0x88, 0x12, 0x69, 0xee, 0x1f,
    0xc7, 0x62, 0x97, 0xd5, 0x0b, 0x79, 0xca, 0xcc,
    0x1b, 0x5d, 0x19, 0x10, 0x24, 0xd3, 0xdc, 0x3f,
    0x8e, 0xc5, 0x2f};

    const uint16_t xn297_crc_xorout_scrambled[] = {
    0x0000, 0x3448, 0x9BA7, 0x8BBB, 0x85E1, 0x3E8C,
    0x451E, 0x18E6, 0x6B24, 0xE7AB, 0x3828, 0x814B,
    0xD461, 0xF494, 0x2503, 0x691D, 0xFE8B, 0x9BA7,
    0x8B17, 0x2920, 0x8B5F, 0x61B1, 0xD391, 0x7401,
    0x2138, 0x129F, 0xB3A0, 0x2988};

    static const uint16_t crc_initial = 0xb5d2;

    static uint8_t byte, bit_count, byte_count;
	static bool in_packet = false;

    // packet settings
	const uint8_t address_length = 5;
	const uint8_t packet_size = 24;
    const uint8_t crc_size = 2;

    static uint16_t crc;

    while(socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        for(uint i=0; i<datagram.data().size(); i++) {
            uint8_t bit = (uint8_t)datagram.data().at(i);
                if((bit & 0x02) /*&& !in_packet*/ ) { // found correlate access code bit (1st bit of address)
			    if(in_packet) {
                    printf("\n");
                }
                byte = 0;
			    bit_count = 0;
			    byte_count = 0;
                crc = crc_initial;
			    in_packet = true;
		    }
        
		    if(in_packet) {
			    if(bit & 0x01) {
				    byte |= 1 << (7-bit_count);
			    }
			    bit_count++;
			    if(bit_count > 7) {
				    if(byte_count < address_length) {
                        crc = crc16_update(crc, byte, 8);
					    byte = byte ^ xn297_scramble[byte_count];
                    }
				    else if(byte_count < address_length + packet_size) {
                        crc = crc16_update(crc, byte, 8);
					    byte = bit_reverse(byte) ^ bit_reverse(xn297_scramble[byte_count]);
				    }
                    if(byte_count == address_length || byte_count == address_length + packet_size)
                        printf("| ");
                    printf("%02x ", byte);
				    bit_count = 0;
				    byte = 0;
				    byte_count++;
				    if(byte_count == address_length + packet_size + crc_size) {
					    in_packet = false;
                        crc ^= xn297_crc_xorout_scrambled[address_length-3+packet_size];
                        printf("| %02x %02x", crc >> 8, crc & 0xff);
					    printf("\n");
				    }
			    }
            }
		}
    }
}

void decoder::rpc_set(const QString & key, int value)
{
    QVariantList args;
    args << value;
    rpc->call("set_" + key, args, 
              this, SLOT(rpc_response(QVariant &)),
              this, SLOT(rpc_fault(int, const QString &)));
}

void decoder::rpc_response(QVariant &response)
{
    // rpc ok
}

void decoder::rpc_fault(int, const QString &fault)
{
    // rpc not connected
}
