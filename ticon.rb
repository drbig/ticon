#!/usr/bin/env ruby
# coding: utf-8
# vim: ts=2 et sw=2 sts=2
#
# small/ticon/ticon.rb - GTK2 tray icon skeleton
# 
# Copyright © 2014 Piotr S. Staszewski 
# Visit http://www.drbig.one.pl for contact information.
#

require 'gtk2'

class TrayIcon
  def initialize
    @icon = Gtk::StatusIcon.new
    @icon.stock = Gtk::Stock::STOP
    @icon.tooltip = 'Exit'

    @icon.signal_connect('button-release-event') {|w,e| mouse_release(w, e) }
  end

  def mouse_release(wgt, evt)
    case evt.button
    when 1
      Gtk.main_quit
    end
  end

  def run!; Gtk.main; end
end

TrayIcon.new.run! if __FILE__ == $0
