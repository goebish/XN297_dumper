#!/usr/bin/env python2
# -*- coding: utf-8 -*-
##################################################
# GNU Radio Python Flow Graph
# Title: Xn297 Gfsk Demodulator
# Generated: Mon Mar 11 03:29:52 2019
##################################################

if __name__ == '__main__':
    import ctypes
    import sys
    if sys.platform.startswith('linux'):
        try:
            x11 = ctypes.cdll.LoadLibrary('libX11.so')
            x11.XInitThreads()
        except:
            print "Warning: failed to XInitThreads()"

from gnuradio import blocks
from gnuradio import digital
from gnuradio import eng_notation
from gnuradio import filter
from gnuradio import gr
from gnuradio import wxgui
from gnuradio.eng_option import eng_option
from gnuradio.fft import window
from gnuradio.filter import firdes
from gnuradio.wxgui import fftsink2
from gnuradio.wxgui import forms
from gnuradio.wxgui import scopesink2
from grc_gnuradio import wxgui as grc_wxgui
from optparse import OptionParser
import SimpleXMLRPCServer
import osmosdr
import threading
import time
import wx


class xn297_gfsk_demodulator(grc_wxgui.top_block_gui):

    def __init__(self):
        grc_wxgui.top_block_gui.__init__(self, title="Xn297 Gfsk Demodulator")
        _icon_path = "E:\Program Files\GNURadio-3.7\share\icons\hicolor\scalable/apps\gnuradio-grc.png"
        self.SetIcon(wx.Icon(_icon_path, wx.BITMAP_TYPE_ANY))

        ##################################################
        # Variables
        ##################################################
        self.samp_rate = samp_rate = 4e6
        self.freq_slider = freq_slider = 18
        self.data_rate = data_rate = 1e6
        self.channel_spacing = channel_spacing = .5e6
        self.samp_per_sym = samp_per_sym = int(samp_rate / data_rate)
        self.rpc_var = rpc_var = 0
        self.freq_offset = freq_offset = (channel_spacing / 2) + (channel_spacing * 0.1)
        self.freq_fine_slider = freq_fine_slider = 150e3
        self.freq = freq = 2.4e9 + (freq_slider*1e6)
        self.channel_trans = channel_trans = channel_spacing*0.5

        ##################################################
        # Blocks
        ##################################################
        _freq_fine_slider_sizer = wx.BoxSizer(wx.VERTICAL)
        self._freq_fine_slider_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_freq_fine_slider_sizer,
        	value=self.freq_fine_slider,
        	callback=self.set_freq_fine_slider,
        	label='Freq fine',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._freq_fine_slider_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_freq_fine_slider_sizer,
        	value=self.freq_fine_slider,
        	callback=self.set_freq_fine_slider,
        	minimum=0,
        	maximum=1000000,
        	num_steps=100,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.Add(_freq_fine_slider_sizer)
        self.xmlrpc_server_0 = SimpleXMLRPCServer.SimpleXMLRPCServer(('localhost', 1235), allow_none=True)
        self.xmlrpc_server_0.register_instance(self)
        self.xmlrpc_server_0_thread = threading.Thread(target=self.xmlrpc_server_0.serve_forever)
        self.xmlrpc_server_0_thread.daemon = True
        self.xmlrpc_server_0_thread.start()
        self.wxgui_scopesink2_0 = scopesink2.scope_sink_f(
        	self.GetWin(),
        	title='Data',
        	sample_rate=data_rate,
        	v_scale=0.5,
        	v_offset=0,
        	t_scale=0,
        	ac_couple=False,
        	xy_mode=False,
        	num_inputs=1,
        	trig_mode=wxgui.TRIG_MODE_NORM,
        	y_axis_label='Counts',
        )
        self.Add(self.wxgui_scopesink2_0.win)
        self.wxgui_fftsink2_0 = fftsink2.fft_sink_c(
        	self.GetWin(),
        	baseband_freq=freq+freq_fine_slider,
        	y_per_div=10,
        	y_divs=10,
        	ref_level=0,
        	ref_scale=2.0,
        	sample_rate=samp_rate,
        	fft_size=1024,
        	fft_rate=15,
        	average=False,
        	avg_alpha=None,
        	title='FFT Plot',
        	peak_hold=False,
        	win=window.blackmanharris,
        )
        self.Add(self.wxgui_fftsink2_0.win)
        self._rpc_var_static_text = forms.static_text(
        	parent=self.GetWin(),
        	value=self.rpc_var,
        	callback=self.set_rpc_var,
        	label='rpc_var',
        	converter=forms.int_converter(),
        )
        self.Add(self._rpc_var_static_text)
        self.osmosdr_source_0 = osmosdr.source( args="numchan=" + str(1) + " " + 'hackrf=0' )
        self.osmosdr_source_0.set_sample_rate(samp_rate)
        self.osmosdr_source_0.set_center_freq(freq+freq_offset+freq_fine_slider, 0)
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
        _freq_slider_sizer = wx.BoxSizer(wx.VERTICAL)
        self._freq_slider_text_box = forms.text_box(
        	parent=self.GetWin(),
        	sizer=_freq_slider_sizer,
        	value=self.freq_slider,
        	callback=self.set_freq_slider,
        	label='Freq coarse',
        	converter=forms.int_converter(),
        	proportion=0,
        )
        self._freq_slider_slider = forms.slider(
        	parent=self.GetWin(),
        	sizer=_freq_slider_sizer,
        	value=self.freq_slider,
        	callback=self.set_freq_slider,
        	minimum=0,
        	maximum=127,
        	num_steps=127,
        	style=wx.SL_HORIZONTAL,
        	cast=int,
        	proportion=1,
        )
        self.Add(_freq_slider_sizer)
        self.digital_gfsk_demod_0 = digital.gfsk_demod(
        	samples_per_symbol=samp_per_sym,
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
        self.blocks_uchar_to_float_0 = blocks.uchar_to_float()
        self.blocks_add_const_vxx_0 = blocks.add_const_vff((-1.5, ))



        ##################################################
        # Connections
        ##################################################
        self.connect((self.blocks_add_const_vxx_0, 0), (self.wxgui_scopesink2_0, 0))
        self.connect((self.blocks_uchar_to_float_0, 0), (self.blocks_add_const_vxx_0, 0))
        self.connect((self.digital_correlate_access_code_bb_0_0_0, 0), (self.blocks_uchar_to_float_0, 0))
        self.connect((self.digital_correlate_access_code_bb_0_0_0, 0), (self.blocks_udp_sink_0, 0))
        self.connect((self.digital_gfsk_demod_0, 0), (self.digital_correlate_access_code_bb_0_0_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.digital_gfsk_demod_0, 0))
        self.connect((self.freq_xlating_fir_filter_xxx_0, 0), (self.wxgui_fftsink2_0, 0))
        self.connect((self.osmosdr_source_0, 0), (self.freq_xlating_fir_filter_xxx_0, 0))

    def get_samp_rate(self):
        return self.samp_rate

    def set_samp_rate(self, samp_rate):
        self.samp_rate = samp_rate
        self.set_samp_per_sym(int(self.samp_rate / self.data_rate))
        self.wxgui_fftsink2_0.set_sample_rate(self.samp_rate)
        self.osmosdr_source_0.set_sample_rate(self.samp_rate)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))

    def get_freq_slider(self):
        return self.freq_slider

    def set_freq_slider(self, freq_slider):
        self.freq_slider = freq_slider
        self.set_freq(2.4e9 + (self.freq_slider*1e6))
        self._freq_slider_slider.set_value(self.freq_slider)
        self._freq_slider_text_box.set_value(self.freq_slider)

    def get_data_rate(self):
        return self.data_rate

    def set_data_rate(self, data_rate):
        self.data_rate = data_rate
        self.set_samp_per_sym(int(self.samp_rate / self.data_rate))
        self.wxgui_scopesink2_0.set_sample_rate(self.data_rate)

    def get_channel_spacing(self):
        return self.channel_spacing

    def set_channel_spacing(self, channel_spacing):
        self.channel_spacing = channel_spacing
        self.set_freq_offset((self.channel_spacing / 2) + (self.channel_spacing * 0.1))
        self.set_channel_trans(self.channel_spacing*0.5)
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))

    def get_samp_per_sym(self):
        return self.samp_per_sym

    def set_samp_per_sym(self, samp_per_sym):
        self.samp_per_sym = samp_per_sym

    def get_rpc_var(self):
        return self.rpc_var

    def set_rpc_var(self, rpc_var):
        self.rpc_var = rpc_var
        self._rpc_var_static_text.set_value(self.rpc_var)

    def get_freq_offset(self):
        return self.freq_offset

    def set_freq_offset(self, freq_offset):
        self.freq_offset = freq_offset
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine_slider, 0)
        self.freq_xlating_fir_filter_xxx_0.set_center_freq(-self.freq_offset)

    def get_freq_fine_slider(self):
        return self.freq_fine_slider

    def set_freq_fine_slider(self, freq_fine_slider):
        self.freq_fine_slider = freq_fine_slider
        self._freq_fine_slider_slider.set_value(self.freq_fine_slider)
        self._freq_fine_slider_text_box.set_value(self.freq_fine_slider)
        self.wxgui_fftsink2_0.set_baseband_freq(self.freq+self.freq_fine_slider)
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine_slider, 0)

    def get_freq(self):
        return self.freq

    def set_freq(self, freq):
        self.freq = freq
        self.wxgui_fftsink2_0.set_baseband_freq(self.freq+self.freq_fine_slider)
        self.osmosdr_source_0.set_center_freq(self.freq+self.freq_offset+self.freq_fine_slider, 0)

    def get_channel_trans(self):
        return self.channel_trans

    def set_channel_trans(self, channel_trans):
        self.channel_trans = channel_trans
        self.freq_xlating_fir_filter_xxx_0.set_taps((firdes.low_pass(1, self.samp_rate, self.channel_spacing, self.channel_trans, firdes.WIN_BLACKMAN, 6.76)))


def main(top_block_cls=xn297_gfsk_demodulator, options=None):

    tb = top_block_cls()
    tb.Start(True)
    tb.Wait()


if __name__ == '__main__':
    main()
