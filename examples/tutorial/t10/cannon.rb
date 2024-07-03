require 'Qt5'

class CannonField < Qt5::Widget
  signals 'angleChanged(int)', 'forceChanged(int)'
  slots 'setAngle(int)', 'setForce(int)'

  def initialize(parent = nil)
    super
    @ang = 45
    @f = 0
    setPalette(Qt5::Palette.new(Qt5::Color.new(250, 250, 200)))
    setAutoFillBackground(true)
  end

  def setAngle(degrees)
    if degrees < 5
      degrees = 5
    elsif degrees > 70
      degrees = 70
    end
    if @ang == degrees
      return
    end
    @ang = degrees
    repaint()
    emit angleChanged(@ang)
  end

  def setForce(newton)
    if newton < 0
      newton = 0
    end
    if @f == newton
      return
    end
    @f = newton
    emit forceChanged(@f)
  end

  def paintEvent(e)
    if !e.rect().intersects(cannonRect())
      return
    end

    cr = cannonRect()
    pix = Qt5::Pixmap.new(cr.size())
    pix.fill(Qt5::Color.new(250, 250, 200))

    painter = Qt5::Painter.new(pix)
    painter.setBrush(Qt5::Brush.new(Qt5::blue))
    painter.setPen(Qt5::NoPen)
    painter.translate(0, pix.height() - 1)
    painter.drawPie(Qt5::Rect.new(-35, -35, 70, 70), 0, 90 * 16)
    painter.rotate(- @ang)
    painter.drawRect(Qt5::Rect.new(33, -4, 15, 8))
    painter.end()

    painter.begin(self)
    painter.drawPixmap(cr.topLeft(), pix)
    painter.end()
  end

  def cannonRect()
    r = Qt5::Rect.new(0, 0, 50, 50)
    r.moveBottomLeft(rect().bottomLeft())
    return r
  end

  def sizePolicy()
    return Qt5::SizePolicy.new(Qt5::SizePolicy::Expanding, Qt5::SizePolicy::Expanding)
  end
end
