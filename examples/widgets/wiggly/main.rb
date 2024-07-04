#!/usr/bin/env ruby

require 'Qt5'
require './dialog.rb'

app = Qt::Application.new(ARGV)
dialog = Dialog.new
dialog.show
app.exec
