require 'Qt5'

class LCDRange < Qt5::Widget
  signals 'valueChanged(int)'
  slots 'setValue(int)', 'setRange(int, int)', 'setText(const char*)'

  def initialize(s, parent = nil)
    super(parent)
    init()
    setText(s)
  end

  def init()
    lcd = Qt5::LCDNumber.new(2)
    @slider = Qt5::Slider.new(Qt5::Horizontal)
    @slider.range = 0..99
    @slider.value = 0

    @label = Qt5::Label.new
    @label.setAlignment(Qt5::AlignHCenter.to_i | Qt5::AlignTop.to_i)
    @label.setSizePolicy(Qt5::SizePolicy::Preferred, Qt5::SizePolicy::Fixed)

    connect(@slider, SIGNAL('valueChanged(int)'), lcd, SLOT('display(int)'))
    connect(@slider, SIGNAL('valueChanged(int)'), SIGNAL('valueChanged(int)'))

    layout = Qt5::VBoxLayout.new
    layout.addWidget(lcd)
    layout.addWidget(@slider)
    setLayout(layout)

    setFocusProxy(@slider)
  end

  def value()
    @slider.value()
  end

  def setValue(value)
    @slider.setValue(value)
  end

  def range=(r)
    setRange(r.begin, r.end)
  end

  def setRange(minVal, maxVal)
    if minVal < 0 || maxVal > 99 || minVal > maxVal
      qWarning("LCDRange::setRange(#{minVal},#{maxVal})\n" +
               "\tRange must be 0..99\n" +
               "\tand minVal must not be greater than maxVal")
      return
    end
    @slider.setRange(minVal, maxVal)
  end

  def setText(s)
    @label.setText(s)
  end
end
