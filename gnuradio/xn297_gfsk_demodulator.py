#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: XN297 Demodulator
# Author: goebish
# Description: gnuradio flow graph for xn297decoder
# Generated: Fri Mar 15 20:40:01 2019
##################################################

from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio.eng_option import eng_option
from gnuradio.filter import firdes
from grc_gnuradio import blks2 as grc_blks2
from optparse import OptionParser
import SimpleXMLRPCServer
import osmosdr
import threading
import time


class xn297_gfsk_demodulator(gr.top_block):

    def __init__(self):
        gr.top_block.__init__(self, "XN297 Demodulator")

        ##################################################
        # Variables
        ##################################################
        self.channel_spacing = channel_spacing = .5e6
        self.channel = channel = 18
        self.samp_rate = samp_rate = 2e6
        self.hearthbeat = hearthbeat = 0
        self.freq_offset = freq_offset = (channel_spacing / 2) + (channel_spacing * 0.1)
        self.freq_fine = freq_fine = 100e3
        self.freq = freq = 2.4e9 + (channel*1e6)
        self.channel_trans = channel_trans = channel_spacing*0.5
        self.bitrate = bitrate = 0

        ##################################################
        # Blocks
        ##################################################
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(('localhost', 1235), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        self.xmlrpc_server_0_thread = threading.Thread(target=self.xmlrpc_server_0.serve_forever)
        self.xmlrpc_server_0_thread.daemon = True
        self.xmlrpc_server_0_thread.start()
        self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + '' )
        self.osmosdr_source_0.set_sample_rate(samp_rate)
        self.osmosdr_source_0.set_center_freq(freq+freq_offset+freq_fine, 0)
        self.osmosdr_source_0.set_freq_corr(0, 0)
        self.osmosdr_source_0.set_dc_offset_mode(0, 0)
        self.osmosdr_source_0.set_iq_balance_mode(0, 0)
        self.osmosdr_source_0.set_gain_mode(False, 0)
        self.osmosdr_source_0.set_gain(0, 0)
        self.osmosdr_source_0.set_if_gain(27, 0)
        self.osmosdr_source_0.set_bb_gain(27, 0)
        self.osmosdr_source_0.set_antenna('', 0)
        self.osmosdr_source_0.set_bandwidth(0, 0)

        self.freq_xlating_fir_filter_xxx_0 = filter.freq_xlating_fir_filter_ccc(1, (firdes.low_pass(1, samp_rate, channel_spacing, channel_trans, firdes.WIN_BLACKMAN, 6.76)), -freq_offset, samp_rate)
        self.digital_gfsk_demod_0_0 = digital.gfsk_demod(
        	samples_per_symbol=int(samp_rate / 250e3),
        	sensitivity=1,
        	gain_mu=0.175,
        	mu=0.05,
        	omega_relative_limit=0.01,
        	freq_error=0.0,
        	verbose=False,
        	log=False,
        )
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
        	samples_per_symbol=int(samp_rate / 1e6),
        	sensitivity=1,
        	gain_mu=0.175,
        	mu=0.05,
        	omega_relative_limit=0.01,
        	freq_error=0.0,
        	verbose=False,
        	log=False,
        )
        self.digital_correlate_access_code_bb_0_0_0 = digital.correlate_access_code_bb('011100010000111101010101', 0)
        self.blocks_udp_sink_0 = blocks.udp_sink(gr.sizeof_char*1, '127.0.0.1', 1234, 1472, True)
        self.bitrate_selector_0 = grc_blks2.selector(
        	item_size=gr.sizeof_char*1,
        	num_inputs=2,
        	num_outputs=1,
        	input_index=bitrate,
        	output_index=0,
        )
        self.bitrate_selector = grc_blks2.selector(
        	item_size=gr.sizeof_gr_complex*1,
        	num_inputs=1,
        	num_outputs=2,
        	input_index=0,
        	output_index=bitrate,
        )



        ##################################################
        # Connections
        ##################################################
        self.connect((self.bitrate_selector, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.bitrate_selector, 1), (self.digital_gfsk_demod_0_0, 0))
        self.connect((self.bitrate_selector_0, 0), (self.digital_correlate_access_code_bb_0_0_0, 0))
        self.connect((self.digital_correlate_access_code_bb_0_0_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.bitrate_selector_0, 0))
        self.connect((self.digital_gfsk_demod_0_0, 0), (self.bitrate_selector_0, 1))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.bitrate_selector, 0))
        self.connect((self.osmosdr_source_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))

    def get_channel_spacing(self):
        return self.channel_spacing

    def set_channel_spacing(self, channel_spacing):
        self.channel_spacing = channel_spacing
        self.set_freq_offset((self.channel_spacing / 2) + (self.channel_spacing * 0.1))
        self.set_channel_trans(self.channel_spacing*0.5)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))

    def get_channel(self):
        return self.channel

    def set_channel(self, channel):
        self.channel = channel
        self.set_freq(2.4e9 + (self.channel*1e6))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.osmosdr_source_0.set_sample_rate(self.samp_rate)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))

    def get_hearthbeat(self):
        return self.hearthbeat

    def set_hearthbeat(self, hearthbeat):
        self.hearthbeat = hearthbeat

    def get_freq_offset(self):
        return self.freq_offset

    def set_freq_offset(self, freq_offset):
        self.freq_offset = freq_offset
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine, 0)
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(-self.freq_offset)

    def get_freq_fine(self):
        return self.freq_fine

    def set_freq_fine(self, freq_fine):
        self.freq_fine = freq_fine
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine, 0)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine, 0)

    def get_channel_trans(self):
        return self.channel_trans

    def set_channel_trans(self, channel_trans):
        self.channel_trans = channel_trans
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))

    def get_bitrate(self):
        return self.bitrate

    def set_bitrate(self, bitrate):
        self.bitrate = bitrate
        self.bitrate_selector_0.set_input_index(int(self.bitrate))
        self.bitrate_selector.set_output_index(int(self.bitrate))


def main(top_block_cls=xn297_gfsk_demodulator, options=None):

    tb = top_block_cls()
    tb.start()
    try:
        raw_input('Press Enter to quit: ')
    except EOFError:
        pass
    tb.stop()
    tb.wait()


if __name__ == '__main__':
    main()
