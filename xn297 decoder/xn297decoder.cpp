#include "xn297decoder.h"
#include <QFile>
#include <QFileDialog>
#include <QPixmap>
#include <QIcon>

// xn297 scramble table
static const uint8_t xn297_scramble[] = {
    0xe3, 0xb1, 0x4b, 0xea, 0x85, 0xbc, 0xe5, 0x66,
    0x0d, 0xae, 0x8c, 0x88, 0x12, 0x69, 0xee, 0x1f,
    0xc7, 0x62, 0x97, 0xd5, 0x0b, 0x79, 0xca, 0xcc,
    0x1b, 0x5d, 0x19, 0x10, 0x24, 0xd3, 0xdc, 0x3f,
    0x8e, 0xc5, 0x2f };

// scrambled, standard mode crc xorout table
static const uint16_t xn297_crc_xorout_scrambled[] = {
    0x0000, 0x3448, 0x9BA7, 0x8BBB, 0x85E1, 0x3E8C,
    0x451E, 0x18E6, 0x6B24, 0xE7AB, 0x3828, 0x814B,
    0xD461, 0xF494, 0x2503, 0x691D, 0xFE8B, 0x9BA7,
    0x8B17, 0x2920, 0x8B5F, 0x61B1, 0xD391, 0x7401,
    0x2138, 0x129F, 0xB3A0, 0x2988 }; // TODO: complete

// scrambled enhanced mode crc xorout table
static const uint16_t xn297_crc_xorout_scrambled_enhanced[] = {
    0x0000, 0x7ebf, 0x3ece, 0x07a4, 0xca52, 0x343b,
    0x53f8, 0x8cd0, 0x9eac, 0xd0c0, 0x150d, 0x5186,
    0xd251, 0xa46f, 0x8435, 0xfa2e, 0x7ebd, 0x3c7d,
    0x94e0, 0x3d5f, 0xa685, 0x4e47, 0xf045, 0xb483,
    0x7a1f, 0xdea2, 0x9642, 0xbf4b, 0x032f, 0x01d2,
    0xdc86, 0x92a5, 0x183a, 0xb760, 0xa953 };

xn297decoder::xn297decoder(QWidget *parent)
    : QMainWindow(parent)
{ 
    ui.setupUi(this);
    setWindowIcon(QIcon(QPixmap(":/xn297decoder/Resources/icon.png")));
    setFixedSize(this->size());
    setWindowFlags(Qt::MSWindowsFixedSizeDialogHint);
    statusBar()->setSizeGripEnabled(false);
    label_statusPps = new QLabel(this);
    label_statusPps->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_statusHearthbeat = new QLabel(this);
    label_statusHearthbeat->setFrameStyle(QFrame::Panel | QFrame::Sunken);
    label_statusSeparator = new QLabel(this);
    label_statusSeparator->setFixedWidth(2);
    ui.statusBar->addWidget(label_statusSeparator);
    ui.statusBar->addWidget(label_statusHearthbeat);
    ui.statusBar->addWidget(label_statusPps);

    rpc = new MaiaXmlRpcClient(QUrl("http://localhost:1235"), this);

    connect(ui.spinBox_channel, SIGNAL(valueChanged(int)), this, SLOT(spinBox_channelChanged(int)));
    connect(ui.spinBox_fineTune, SIGNAL(valueChanged(int)), this, SLOT(spinBox_fineTuneChanged(int)));
    connect(ui.spinBox_addressLength, SIGNAL(valueChanged(int)), this, SLOT(spinBox_addressLengthChanged(int)));
    connect(ui.spinBox_payloadLength, SIGNAL(valueChanged(int)), this, SLOT(spinBox_payloadLengthChanged(int)));
    connect(ui.radioButton_bitrate1M, SIGNAL(toggled(bool)), this, SLOT(radioButton_bitrate1MChanged()));
    connect(ui.pushButton_locateGnuradio, SIGNAL(clicked()), this, SLOT(pushButton_locateGnuradioClicked()));
    connect(ui.pushButton_startStopFlowgraph, SIGNAL(clicked()), this, SLOT(pushButton_startStopFlowgraphClicked()));
    connect(ui.checkBox_enhanced, SIGNAL(clicked()), this, SLOT(checkBox_enhancedClicked()));
    connect(ui.checkBox_autoLength, SIGNAL(clicked()), this, SLOT(checkBox_autoLengthClicked()));
    connect(ui.checkBox_showValid, SIGNAL(clicked()), this, SLOT(checkBox_showValidClicked()));

    settings = new QSettings(QSettings::IniFormat, QSettings::UserScope, "Goebish Apps", "xn297 decoder", this);
    load_settings();

    pps_timer = new QTimer(this);
    pps_counter = 0;
    connect(pps_timer, SIGNAL(timeout()), this, SLOT(show_pps()));
    pps_timer->start(1000);

    rpc_hearthbeat_timer = new QTimer(this);
    is_rpc_connected = false;
    connect(rpc_hearthbeat_timer, SIGNAL(timeout()), this, SLOT(rpc_hearthbeat()));
    rpc_hearthbeat();
    rpc_hearthbeat_timer->start(3000);

    socket = new QUdpSocket(this);
    socket->bind(QHostAddress::LocalHost, 1234);
    connect(socket, SIGNAL(readyRead()), this, SLOT(readPendingDatagrams()));

    gnuradio_process = new QProcess(this);
    connect(gnuradio_process, SIGNAL(stateChanged(QProcess::ProcessState)), this, SLOT(gnuradio_processStateChanged(QProcess::ProcessState)));
    connect(gnuradio_process, SIGNAL(readyReadStandardOutput()), this, SLOT(gnuradio_processStdOutput()));
    connect(gnuradio_process, SIGNAL(readyReadStandardError()), this, SLOT(gnuradio_processStdError()));
}

void xn297decoder::load_settings()
{
    bool ok;
    ui.spinBox_channel->setValue(settings->value("channel","0").toInt(&ok));
    rpc_set("channel", settings->value("channel","0").toInt(&ok));
    ui.spinBox_fineTune->setValue(settings->value("finetune","100").toInt(&ok));
    rpc_set("freq_fine", settings->value("finetune","100").toInt(&ok)*1000);
    addressLength = settings->value("address_length","5").toInt(&ok);
    ui.spinBox_addressLength->setValue(addressLength);
    payloadLength = settings->value("payload_length","15").toInt(&ok);
    ui.spinBox_payloadLength->setValue(payloadLength);
    if(settings->value("bitrate","1M") == "1M")
        ui.radioButton_bitrate1M->setChecked(true);
    else
        ui.radioButton_bitrate250k->setChecked(true);
    radioButton_bitrate1MChanged();
    if(QFile::exists(settings->value("gnuradio_launcher","").toString()))
        ui.pushButton_startStopFlowgraph->setEnabled(true);
    ui.checkBox_enhanced->setChecked(settings->value("enhanced","0") == "1");
    ui.checkBox_autoLength->setChecked(settings->value("autolen", "0") == "1");
    ui.checkBox_autoLength->setEnabled(ui.checkBox_enhanced->isChecked());
    ui.checkBox_showValid->setChecked(settings->value("showvalid", "0") == "1");
}

void xn297decoder::run_gr_flowgraph()
{
    if(!QFile::exists(settings->value("gnuradio_launcher","").toString())) {
        ui.plainTextEdit->appendHtml("gnuradio launcher not found");
        return;
    }
    if(!QFile::exists(GR_FLOWGRAPH)) {
        ui.plainTextEdit->appendHtml("gnuradio flow graph " + (QString)GR_FLOWGRAPH + " not found");
        return;
    }    
    ui.plainTextEdit->appendHtml("launching gnuradio flow graph");
    ui.plainTextEdit->appendHtml("> " + settings->value("gnuradio_launcher","").toString() + " -u " + (QString)GR_FLOWGRAPH);
    gnuradio_process->start(settings->value("gnuradio_launcher","").toString(), QStringList() << "-u" << GR_FLOWGRAPH);
}

uint8_t xn297decoder::bit_reverse(uint8_t b_in)
{
    uint8_t b_out = 0;
    for (uint8_t i = 0; i < 8; ++i) {
        b_out = (b_out << 1) | (b_in & 1);
        b_in >>= 1;
    }
    return b_out;
}

uint16_t xn297decoder::crc16_update(uint16_t crc, uint8_t a, uint8_t bits)
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

void xn297decoder::decodeStd()
{
    const uint16_t crc_initial = 0xb5d2;
    const uint8_t crc_size = 2;

    static uint8_t byte, bit_count, byte_count, crc_index;
    static bool in_packet = false;
    static uint16_t crc, packet_crc;
    static QString log;
    QString temp;
    bool valid;

    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        for (uint i = 0; i<datagram.data().size(); i++) {
            uint8_t bit = (uint8_t)datagram.data().at(i);
            if ((bit & 0x02)) { // found correlate access code bit (1st bit of address)
                byte = 0;
                bit_count = 0;
                byte_count = 0;
                crc = crc_initial;
                crc_index = 1;
                packet_crc = 0;
                log.clear();
                in_packet = true;
            }
            // TODO: reverse address
            if (in_packet) {
                if (bit & 0x01) {
                    byte |= 1 << (7 - bit_count);
                }
                bit_count++;
                if (bit_count > 7) {
                    if (byte_count < addressLength) {
                        crc = crc16_update(crc, byte, 8);
                        byte = byte ^ xn297_scramble[byte_count];
                    }
                    else if (byte_count < addressLength + payloadLength) {
                        crc = crc16_update(crc, byte, 8);
                        byte = bit_reverse(byte ^ xn297_scramble[byte_count]);
                    }
                    if (byte_count == addressLength || byte_count == addressLength + payloadLength)
                        log += "<b>|</b> ";
                    if (byte_count < addressLength + payloadLength)
                        log += temp.sprintf("%02X ", byte);
                    else // crc bytes
                        packet_crc |= (uint16_t)byte << (8 * crc_index--);
                    bit_count = 0;
                    byte = 0;
                    byte_count++;
                    if (byte_count == addressLength + payloadLength + crc_size) {
                        in_packet = false;
                        crc ^= xn297_crc_xorout_scrambled[addressLength - 3 + payloadLength];
                        if (packet_crc == crc) {
                            log += temp.sprintf("<font color='green'>%02X %02X", packet_crc >> 8, packet_crc & 0xff);
                            valid = true;
                        }
                        else {
                            log += temp.sprintf("<font color='red'><b>%02X %02X", packet_crc >> 8, packet_crc & 0xff);
                            valid = false;
                        }
                        if(!ui.checkBox_showValid->isChecked() || valid)
                            ui.plainTextEdit->appendHtml(log);
                        pps_counter++;
                    }
                }
            }
        }
    }
}

void xn297decoder::decodeEnhanced()
{
    const uint16_t crc_initial = 0xb5d2;
    uint8_t i;
    static bool in_packet = false;
    static uint8_t byte;
    static uint8_t bit_count;
    static uint8_t byte_count;
    static uint16_t crc;
    static uint8_t payload_index;
    static uint8_t address_index;
    static uint8_t crc_index;
    static uint8_t pcf_len;
    static uint8_t pcf_len2;
    static uint8_t pcf_id;
    static uint8_t pcf_noack;
    static uint8_t tmp_payload;
    static uint8_t address[5];
    static uint8_t payload[32];
    static uint8_t crc_rx[2];
    static QString log;
    QString temp;
    bool valid;

    while (socket->hasPendingDatagrams()) {
        QNetworkDatagram datagram = socket->receiveDatagram();
        for (uint j = 0; j<datagram.data().size(); j++) {
            uint8_t bit = (uint8_t)datagram.data().at(j);
            if (bit & 0x02) { // found correlate access code bit (1st bit of address)
                byte = 0;
                bit_count = 0;
                byte_count = 0;
                crc = crc_initial;
                payload_index = 0;
                address_index = 0;
                crc_index = 0;
                log.clear();
                in_packet = true;
            }

            if (in_packet) {
                if (bit & 0x01) {
                    byte |= 1 << (7 - bit_count);
                }
                bit_count++;
                if (bit_count > 7) {
                    // TODO: reverse address
                    if (byte_count < addressLength) {
                        crc = crc16_update(crc, byte, 8);
                        address[address_index++] = byte ^ xn297_scramble[byte_count];
                    }
                    else if (byte_count == addressLength) { // 8 msb of PCF
                        crc = crc16_update(crc, byte, 8);
                        byte = byte ^ xn297_scramble[byte_count];
                        pcf_len2 = byte >> 1;
                        if(ui.checkBox_autoLength->isChecked()) {
                            pcf_len = byte >> 1;  // automatic payload size
                            if (pcf_len > 32)
                                pcf_len = 32;
                        }
                        else {
                            pcf_len = ui.spinBox_payloadLength->value();
                        }
                        pcf_id = (byte & 1) << 1; // pid msb
                    }
                    else if (byte_count == addressLength + 1) { // 2 lsb of PCF + 6 bit of payload
                        tmp_payload = byte << 2; // 6 bit of payload (scrambled+reversed)
                        crc = crc16_update(crc, byte, 8);
                        byte = byte ^ xn297_scramble[byte_count];
                        pcf_id |= byte >> 7; // pid lsb
                        pcf_noack = (byte & 0x7f) >> 6; // no_ack flag
                    }
                    else if (payload_index < pcf_len) {
                        tmp_payload |= byte >> 6; // 2 remaining bit of payload

                        uint8_t tt = byte;
                        if (payload_index == pcf_len - 1) tt = tt & 0xc0; // pad last byte lsb with 000000
                        crc = crc16_update(crc, tt, payload_index == pcf_len - 1 ? 2 : 8);

                        uint8_t xor = xn297_scramble[byte_count - 1] << 2 | xn297_scramble[byte_count] >> 6;
                        payload[payload_index++] = bit_reverse(tmp_payload ^ xor);
                        tmp_payload = byte << 2; // 6 next bit of payload
                    }
                    else { // crc
                        tmp_payload |= byte >> 6;
                        crc_rx[crc_index++] = tmp_payload;
                        tmp_payload = byte << 2;
                    }
                    bit_count = 0;
                    byte = 0;
                    byte_count++;
                    if (byte_count == addressLength + 2 + pcf_len + 2) { // address | pcf | payload |crc 
                        for (i = 0; i<addressLength; i++)
                            log += temp.sprintf("%02x ", address[i]);
                        log += temp.sprintf("<b>|</b> %02x %x %x <b>|</b> ", pcf_len2, pcf_id, pcf_noack);
                        for (i = 0; i<pcf_len; i++)
                            log += temp.sprintf("%02x ", payload[i]);
                        log += "<b>|</b> ";
                        
                        uint16_t crc_xorout = xn297_crc_xorout_scrambled_enhanced[addressLength+pcf_len-3]; // 0x8435 = crc xorout for 12 byte payload

                        if ((crc ^ crc_xorout) == ((crc_rx[0] << 8) | crc_rx[1])) {
                            log += "<font color='green'>";
                            valid = true;
                        }
                        else {
                            log += "<font color='red'><b>";
                            valid = false;
                        }
                        for (i = 0; i<crc_index; i++)
                            log += temp.sprintf("%02x ", crc_rx[i]);
                        
                        //log += temp.sprintf("%04x ", crc);
                        log += temp.sprintf("%04x", crc ^ ((crc_rx[0] << 8) | crc_rx[1]));
                    
                        if (!ui.checkBox_showValid->isChecked() || valid)
                            ui.plainTextEdit->appendHtml(log);
                        pps_counter++;
                        in_packet = false;
                    }
                }
            }
        }
    }
}

void xn297decoder::readPendingDatagrams()
{
    if (ui.checkBox_enhanced->isChecked())
        decodeEnhanced();
    else
        decodeStd();
}

void xn297decoder::rpc_set(const QString & key, int value)
{
    rpc->call("set_" + key, QVariantList() << value, 
              this, SLOT(rpc_response(QVariant &)),
              this, SLOT(rpc_fault(int, const QString &)));
}

void xn297decoder::rpc_response(QVariant &response)
{
    
}

void xn297decoder::rpc_fault(int, const QString &fault)
{
    
}

void xn297decoder::rpc_hearthbeat_response(QVariant &response)
{
    if(!is_rpc_connected) {
        label_statusHearthbeat->setText("RPC hearthbeat Ok");
        is_rpc_connected = true;
        load_settings();
    }
}

void xn297decoder::rpc_hearthbeat_fault(int, const QString &fault)
{
    is_rpc_connected = false;
}

void xn297decoder::rpc_hearthbeat()
{
    rpc->call("set_hearthbeat", QVariantList() << 1, 
              this, SLOT(rpc_hearthbeat_response(QVariant &)),
              this, SLOT(rpc_hearthbeat_fault(int, const QString &)));
}

void xn297decoder::spinBox_channelChanged(int value)
{
    rpc_set("channel", value);
    uint freq = 2.4e9 + value*1e6 + ui.spinBox_fineTune->value()*1000; 
    ui.label_frequency->setText(QString::number((float)freq/1e6, 'f', 2) + " MHz");
    settings->setValue("channel", QString::number(value));
}

void xn297decoder::spinBox_fineTuneChanged(int value)
{
    rpc_set("freq_fine", value*1000);
    uint freq = 2.4e9 + ui.spinBox_channel->value()*1e6 + value*1000; 
    ui.label_frequency->setText(QString::number((float)freq/1e6, 'f', 2) + " MHz");
    settings->setValue("finetune", QString::number(value));
}

void xn297decoder::spinBox_addressLengthChanged(int value)
{
    addressLength = value;
    settings->setValue("address_length", QString::number(value));
}

void xn297decoder::spinBox_payloadLengthChanged(int value)
{
    payloadLength = value;
    settings->setValue("payload_length", QString::number(value));
}

void xn297decoder::show_pps()
{
    if(is_rpc_connected)
        label_statusPps->setText(QString::number(pps_counter) + " pps");
    else
        label_statusHearthbeat->setText("Waiting for RPC hearthbeat");
    pps_counter = 0;
}

void xn297decoder::radioButton_bitrate1MChanged()
{
    if(ui.radioButton_bitrate1M->isChecked()) {
        settings->setValue("bitrate", "1M");
        rpc_set("bitrate", 0);
    }
    else {
        settings->setValue("bitrate", "250k");
        rpc_set("bitrate", 1);
    }
}

void xn297decoder::pushButton_locateGnuradioClicked()
{
    QString file = QFileDialog::getOpenFileName(this, "Locate gnuradio launcher", settings->value("gnuradio_launcher", "C:\\Program files").toString() , "run_gr.bat", nullptr);
    if(QFile::exists(file)) {
        settings->setValue("gnuradio_launcher", file);
        ui.pushButton_startStopFlowgraph->setEnabled(true);
    }
}

void xn297decoder::pushButton_startStopFlowgraphClicked()
{
    if(gnuradio_process->state() == QProcess::Running) {
        gnuradio_process->kill();
    }
    else {
        run_gr_flowgraph();
    }
}

void xn297decoder::gnuradio_processStateChanged(QProcess::ProcessState newState) {
    switch(newState) {
        case QProcess::Running:
            ui.plainTextEdit->appendHtml("flow graph starting");
            ui.pushButton_startStopFlowgraph->setText("stop gnuradio flow graph");
            break;
        case QProcess::NotRunning:
            ui.plainTextEdit->appendHtml("gnuradio flow graph stopped");
            ui.pushButton_startStopFlowgraph->setText("start gnuradio flow graph");
            break;
    }
}

void xn297decoder::gnuradio_processStdOutput()
{
    QString output = (QString)(gnuradio_process->readAllStandardOutput());
    if(output.indexOf("Press Enter") < 0)
        ui.plainTextEdit->appendHtml(output);
}

void xn297decoder::gnuradio_processStdError()
{
    QString output = (QString)(gnuradio_process->readAllStandardError());
    // strip XMLRPC server debug log
    if(output.indexOf("POST") < 0)
        ui.plainTextEdit->appendHtml(output);
}

void xn297decoder::checkBox_enhancedClicked()
{
    if (ui.checkBox_enhanced->isChecked()) {
        ui.checkBox_autoLength->setEnabled(true);
    }
    else {
        ui.checkBox_autoLength->setEnabled(false);
    }
    settings->setValue("enhanced", ui.checkBox_enhanced->isChecked() ? "1" : "0");
}

void xn297decoder::checkBox_autoLengthClicked()
{
    settings->setValue("autolen", ui.checkBox_autoLength->isChecked() ? "1" : "0");
}

void xn297decoder::checkBox_showValidClicked()
{
    settings->setValue("showvalid", ui.checkBox_showValid->isChecked() ? "1" : "0");
}
