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
	
	slots	'fillRule=(Qt5::FillRule)',
    		'setFillGradient(const QColor &, const QColor &)',
    		'penWidth=(int)',
    		'penColor=(const QColor &)',
    		'rotationAngle=(int)'
		
	def initialize(path, parent = nil)
	    super(parent)
	    @penWidth = 1
	    @rotationAngle = 0
		@path = path
		@fillColor1 = Qt5::Color.new
		@fillColor2 = Qt5::Color.new
		@penWidth = 0
		@penColor = Qt5::Color.new
	    setBackgroundRole(Qt5::Palette::Base)
	end
	
	def minimumSizeHint()
	    return Qt5::Size.new(50, 50)
	end
	
	def sizeHint()
	    return Qt5::Size.new(100, 100)
	end
	
	def fillRule=(rule)
	    @path.fillRule = rule
	    update()
	end
	
	def setFillGradient(color1, color2)
	    @fillColor1 = color1
	    @fillColor2 = color2
	    update()
	end
	
	def penWidth=(width)
	    @penWidth = width
	    update()
	end
	
	def penColor=(color)
	    @penColor = color
	    update()
	end
	
	def rotationAngle=(degrees)
	    @rotationAngle = degrees
	    update()
	end
	
	def paintEvent(event)
	    painter = Qt5::Painter.new(self)
	    painter.renderHint = Qt5::Painter::Antialiasing
	    painter.scale(width() / 100.0, height() / 100.0)
	    painter.translate(50.0, 50.0)
	    painter.rotate(-@rotationAngle)
	    painter.translate(-50.0, -50.0)
	
	    painter.pen = Qt5::Pen.new(Qt5::Brush.new(@penColor), @penWidth, Qt5::SolidLine, Qt5::RoundCap,
	                        Qt5::RoundJoin)
	    gradient = Qt5::LinearGradient.new(0, 0, 0, 100)
	    gradient.setColorAt(0.0, @fillColor1)
	    gradient.setColorAt(1.0, @fillColor2)
	    painter.brush = Qt5::Brush.new(gradient)
	    painter.drawPath(@path)
		painter.end
	end
end
