#pragma once

#include <QtWidgets/QMainWindow>
#include <QUdpSocket>
#include <QSettings>
#include <Qtimer>
#include <QProcess>
#include <QLabel>
#include "ui_xn297decoder.h"
#include "maiaXmlRpcClient.h"

#define GR_FLOWGRAPH "xn297_gfsk_demodulator.py"

class xn297decoder : public QMainWindow
{
    Q_OBJECT

public:
    xn297decoder(QWidget *parent = Q_NULLPTR);

private:
    Ui::xn297decoderClass ui;
    QUdpSocket *socket;
    uint8_t bit_reverse(uint8_t b_in);
    uint16_t crc16_update(uint16_t crc, uint8_t a, uint8_t bits);
    MaiaXmlRpcClient *rpc;
    void rpc_set(const QString & key, int value);
    int addressLength;
    int payloadLength;
    QSettings *settings;
    void load_settings();
    QTimer *pps_timer;
    QTimer *rpc_hearthbeat_timer;
    bool is_rpc_connected;
    uint pps_counter;
    QProcess *gnuradio_process;
    void run_gr_flowgraph();
    QLabel *label_statusPps;
    QLabel *label_statusHearthbeat;
    QLabel *label_statusSeparator;

private slots:
    void readPendingDatagrams();
    void rpc_hearthbeat_response(QVariant &);
    void rpc_hearthbeat_fault(int, const QString &);
    void rpc_response(QVariant &);
    void rpc_fault(int, const QString &);
    void spinBox_channelChanged(int);
    void spinBox_fineTuneChanged(int);
    void spinBox_addressLengthChanged(int);
    void spinBox_payloadLengthChanged(int);
    void radioButton_bitrate1MChanged();
    void pushButton_locateGnuradioClicked();
    void pushButton_startStopFlowgraphClicked();
    void show_pps(); // pps timer timout
    void rpc_hearthbeat();
    void gnuradio_processStateChanged(QProcess::ProcessState newState);
    void gnuradio_processStdOutput();
    void gnuradio_processStdError();
};
