#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'

class MyWidget < Qt5::Widget
  def initialize(parent = nil)
    super
    setFixedSize(200, 120)

    quit = Qt5::PushButton.new('Quit', self)
    quit.setGeometry(62, 40, 75, 30)
    quit.setFont(Qt5::Font.new('Times', 18, Qt5::Font::Bold))

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
  end
end

app = Qt5::Application.new(ARGV)

widget = MyWidget.new
widget.show
app.exec
