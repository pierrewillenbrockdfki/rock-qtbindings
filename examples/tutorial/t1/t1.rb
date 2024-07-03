#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'

a = Qt5::Application.new(ARGV)
hello = Qt5::PushButton.new('Hello World!', nil)
hello.resize(100, 30)
hello.show()
a.exec()
