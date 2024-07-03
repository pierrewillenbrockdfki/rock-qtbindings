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
	
	Line = 0
	Points = 1
	Polyline = 3
	Polygon = 4
	Rect = 5
	RoundRect = 6
	Ellipse = 7
	Arc = 8
	Chord = 9
	Pie = 10
	Path = 11
	Text = 12
	Pixmap = 13
	
	slots	'shape=(int)',
    		'pen=(const QPen &)',
    		'brush=(const QBrush &)',
    		'antialiased=(bool)',
    		'transformed=(bool)'
	
	def initialize(parent = nil)
	    super(parent)
	    @shape = Polygon
	    @antialiased = false
		@pen = Qt5::Pen.new
		@brush = Qt5::Brush.new
		@pixmap = Qt5::Pixmap.new
	    @pixmap.load("images/qt-logo.png")
	
	    setBackgroundRole(Qt5::Palette::Base)
	end
	
	def minimumSizeHint()
	    return Qt5::Size.new(100, 100)
	end
	
	def sizeHint()
	    return Qt5::Size.new(400, 200)
	end
	
	def shape=(shape)
	    @shape = shape
	    update()
	end
	
	def pen=(pen)
	    @pen = pen
	    update()
	end
	
	def brush=(brush)
	    @brush = brush
	    update()
	end
	
	def antialiased=(antialiased)
	    @antialiased = antialiased
	    update()
	end
	
	def transformed=(transformed)
	    @transformed = transformed
	    update()
	end
	
	def paintEvent(event)
	    points = Qt5::Polygon.new([	Qt5::Point.new(10, 80),
	        						Qt5::Point.new(20, 10),
	        						Qt5::Point.new(80, 30),
	        						Qt5::Point.new(90, 70) ])
	
	    rect = Qt5::Rect.new(10, 20, 80, 60)
	
	    path = Qt5::PainterPath.new
	    path.moveTo(20, 80)
	    path.lineTo(20, 30)
	    path.cubicTo(80, 0, 50, 50, 80, 80)
	
	    startAngle = 30 * 16
	    arcLength = 120 * 16
	
	    painter = Qt5::Painter.new(self)
	    painter.pen = @pen

	    painter.brush = @brush
	    if @antialiased
	        painter.renderHint = Qt5::Painter::Antialiasing
		end
	
		x = 0
		while x < width() do
			y = 0
			while y < height() do
	            painter.save()
	            painter.translate(x, y)
	            if @transformed
	                painter.translate(50, 50)
	                painter.rotate(60.0)
	                painter.scale(0.6, 0.9)
	                painter.translate(-50, -50)
	            end
	
	            case @shape
	            when Line
	                painter.drawLine(rect.bottomLeft(), rect.topRight())
	            when Points
	                painter.drawPoints(points)
	            when Polyline
	                painter.drawPolyline(points)
	            when Polygon
	                painter.drawPolygon(points)
	            when Rect
	                painter.drawRect(rect)
	            when RoundRect
	                painter.drawRoundRect(rect)
	            when Ellipse
	                painter.drawEllipse(rect)
	            when Arc
	                painter.drawArc(rect, startAngle, arcLength)
	            when Chord
	                painter.drawChord(rect, startAngle, arcLength)
	            when Pie
	                painter.drawPie(rect, startAngle, arcLength)
	            when Path
	                painter.drawPath(path)
	            when Text
	                painter.drawText(rect, Qt5::AlignCenter, tr("Qt by\nTrolltech"))
	            when Pixmap
	                painter.drawPixmap(10, 10, @pixmap)
	            end
	            painter.restore()
				y += 100
	        end
			x += 100
	    end
		painter.end
	end
end
