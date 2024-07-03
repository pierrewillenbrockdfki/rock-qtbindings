#!/usr/bin/env ruby

require 'Qt5'
require './digitalclock.rb'

app = Qt5::Application.new(ARGV)
clock = DigitalClock.new
clock.show
app.exec
