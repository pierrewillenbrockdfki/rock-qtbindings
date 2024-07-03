#!/usr/bin/ruby -w
require 'Qt5'

class LCDRange < Qt5::Widget
  signals 'valueChanged(int)'
  slots 'setValue(int)'

  def initialize(parent = nil)
    super
    lcd = Qt5::LCDNumber.new(2)

    @slider = Qt5::Slider.new(Qt5::Horizontal)
    @slider.range = 0..99
    @slider.value = 0

    connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
    connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))

    layout = Qt5::VBoxLayout.new
    layout.addWidget(lcd)
    layout.addWidget(@slider)
    setLayout(layout)
  end

  def value()
    @slider.value()
  end

  def setValue(value)
    @slider.setValue(value)
  end
end
