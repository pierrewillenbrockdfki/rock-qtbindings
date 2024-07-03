=begin
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** this file.  Please review the following information to ensure GNU
** General Public Licensing requirements will be met:
** http://www.trolltech.com/products/qt/opensource.html
**
** If you are unsure which license is appropriate for your use, please
** review the following information:
** http://www.trolltech.com/products/qt/licensing.html or contact the
** sales department at sales@trolltech.com.
**
** This file is provided AS IS with NO WARRANTY OF ANY KIND, INCLUDING THE
** WARRANTY OF DESIGN, MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
**

** Translated to QtRuby by Richard Dale
=end
	
class RenderArea < Qt5::Widget
	
	NoTransformation = 0
	Translate = 1
	Rotate = 2
	Scale = 3
	
	def initialize(parent = nil)
	    super(parent)
	    newFont = font()
	    newFont.pixelSize = 12
	    setFont(newFont)
	
	    fontMetrics = Qt5::FontMetrics.new(newFont)
	    @xBoundingRect = fontMetrics.boundingRect(tr("x"))
	    @yBoundingRect = fontMetrics.boundingRect(tr("y"))
		@operations = []
		@shape = Qt5::PainterPath.new
	end
	
	def operations=(operations)
	    @operations = Array.new(operations)
	    update()
	end
	
	def shape=(shape)
	    @shape = shape
	    update()
	end
	
	def minimumSizeHint()
	    return Qt5::Size.new(50, 50)
	end
	
	def sizeHint()
	    return Qt5::Size.new(232, 232)
	end
	
	def paintEvent(event)
	    painter = Qt5::Painter.new(self)
	    painter.renderHint = Qt5::Painter::Antialiasing
	    painter.fillRect(event.rect(), Qt5::Brush.new(Qt5::white))
	
	    painter.translate(66, 66)
	
	    painter.save()
	    transformPainter(painter)
	    drawShape(painter)
	    painter.restore()
	
	    drawOutline(painter)
	
	    painter.save()
	    transformPainter(painter)
	    drawCoordinates(painter)
	    painter.restore()
		painter.end
	end
	
	def drawCoordinates(painter)
	    painter.pen = Qt5::Pen.new(Qt5::Color.new(Qt5::red))
	
	    painter.drawLine(0, 0, 50, 0)
	    painter.drawLine(48, -2, 50, 0)
	    painter.drawLine(48, 2, 50, 0)
	    painter.drawText(60 - @xBoundingRect.width() / 2,
	                     0 + @xBoundingRect.height() / 2, tr("x"))
	
	    painter.drawLine(0, 0, 0, 50)
	    painter.drawLine(-2, 48, 0, 50)
	    painter.drawLine(2, 48, 0, 50)
	    painter.drawText(0 - @yBoundingRect.width() / 2,
	                     60 + @yBoundingRect.height() / 2, tr("y"))
	end
	
	def drawOutline(painter)
	    painter.pen = Qt5::Pen.new(Qt5::Color.new(Qt5::darkGreen))
	    painter.pen = Qt5::Pen.new(Qt5::DashLine)
	    painter.brush = Qt5::NoBrush
	    painter.drawRect(0, 0, 100, 100)
	end
	
	def drawShape(painter)
	    painter.fillPath(@shape, Qt5::Brush.new(Qt5::blue))
	end
	
	def transformPainter(painter)
		(0...@operations.length).each do |i|
	        case @operations[i]
	        when Translate
	            painter.translate(50, 50)
	        when Scale
	            painter.scale(0.75, 0.75)
	        when Rotate
	            painter.rotate(60)
	        when NoTransformation
	        else
	            
	        end
	    end
	end
end
