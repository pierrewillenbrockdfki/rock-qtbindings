#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'

app = Qt5::Application.new(ARGV)

window = Qt5::Widget.new
window.resize(200, 120)

quit = Qt5::PushButton.new('Quit', window)
quit.font = Qt5::Font.new('Times', 18, Qt5::Font::Bold)
quit.setGeometry(10, 40, 180, 40)
app.connect(quit, SIGNAL('clicked()'), app, SLOT('quit()'))

window.show
app.exec
