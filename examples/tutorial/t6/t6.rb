#!/usr/bin/env ruby
$VERBOSE = true; $:.unshift File.dirname($0)

require 'Qt5'

class LCDRange < Qt5::Widget
  def initialize(parent = nil)
    super
    lcd = Qt5::LCDNumber.new(2)
    slider = Qt5::Slider.new(Qt5::Horizontal)
    slider.range = 0..99
    slider.value = 0

    lcd.connect(slider, SIGNAL('valueChanged(int)'), SLOT('display(int)'))

    layout = Qt5::VBoxLayout.new
    layout.addWidget(lcd)
    layout.addWidget(slider)
    setLayout(layout)
  end
end

class MyWidget < Qt5::Widget
  def initialize()
    super
    quit = Qt5::PushButton.new('Quit')
    quit.setFont(Qt5::Font.new('Times', 18, Qt5::Font::Bold))
    connect(quit, SIGNAL('clicked()'), $qApp, SLOT('quit()'))

    grid = Qt5::GridLayout.new

    for row in 0..3
      for column in 0..3
        grid.addWidget(LCDRange.new, row, column)
      end
    end

    layout = Qt5::VBoxLayout.new
    layout.addWidget(quit)
    layout.addLayout(grid)
    setLayout(layout)
  end
end

app = Qt5::Application.new(ARGV)
widget = MyWidget.new
widget.show
app.exec
