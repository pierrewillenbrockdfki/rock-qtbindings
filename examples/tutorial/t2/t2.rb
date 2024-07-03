#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5';

a = Qt5::Application.new(ARGV)

quit = Qt5::PushButton.new('Quit', nil)
quit.resize(75, 30)
quit.setFont(Qt5::Font.new('Times', 18, Qt5::Font::Bold))

Qt5::Object.connect(quit, SIGNAL('clicked()'), a, SLOT('quit()'))

quit.show
a.exec
exit
