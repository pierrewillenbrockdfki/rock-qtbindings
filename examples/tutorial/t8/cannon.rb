require 'Qt5'

class CannonField < Qt5::Widget
  signals 'angleChanged(int)'
  slots 'setAngle(int)'

  def initialize(parent = nil)
    super
    @currentAngle = 45
    setPalette(Qt5::Palette.new(Qt5::Color.new(250, 250, 200)))
    setAutoFillBackground(true)
  end

  def setAngle(degrees)
    if degrees < 5
      degrees = 5
    elsif degrees > 70
      degrees = 70
    end
    if @currentAngle == degrees
      return
    end
    @currentAngle = degrees
    repaint()
    emit angleChanged(@currentAngle)
  end

  def paintEvent(event)
    p = Qt5::Painter.new(self)
    p.drawText(200, 200, "Angle = %d" % @currentAngle)
    p.end()
  end

  def sizePolicy()
    return Qt5::SizePolicy.new(Qt5::SizePolicy::Expanding, Qt5::SizePolicy::Expanding)
  end
end
