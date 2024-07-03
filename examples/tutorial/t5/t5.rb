#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'

class MyWidget < Qt5::Widget
  def initialize()
    super
    quit = Qt5::PushButton.new('Quit')
    quit.setFont(Qt5::Font.new('Times', 18, Qt5::Font::Bold))

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))

    lcd = Qt5::LCDNumber.new(2)

    slider = Qt5::Slider.new(Qt5::Horizontal)
    slider.range = 0..99
    slider.value = 0

    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))
    connect(slider, SIGNAL('valueChanged(int)'),
            lcd, SLOT('display(int)'))

    layout = Qt5::VBoxLayout.new
    layout.addWidget(quit)
    layout.addWidget(lcd)
    layout.addWidget(slider)
    setLayout(layout)
  end
end

app = Qt5::Application.new(ARGV)
widget = MyWidget.new
widget.show
app.exec
