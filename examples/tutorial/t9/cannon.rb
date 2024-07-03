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
    painter = Qt5::Painter.new(self)

    painter.setPen(Qt5::NoPen)
    painter.setBrush(Qt5::Brush.new(Qt5::blue))

    painter.translate(0, rect().bottom())
    painter.drawPie(Qt5::Rect.new(-35, -35, 70, 70), 0, 90 * 16)
    painter.rotate(- @currentAngle)
    painter.drawRect(Qt5::Rect.new(33, -4, 15, 8))
    painter.end()
  end

  def sizePolicy()
    return Qt5::SizePolicy.new(Qt5::SizePolicy::Expanding, Qt5::SizePolicy::Expanding)
  end
end
