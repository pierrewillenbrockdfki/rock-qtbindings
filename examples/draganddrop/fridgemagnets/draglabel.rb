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
		
class DragLabel < Qt5::Label
	
	def initialize(text, parent = nil)
	    super(parent)
	    fm = Qt5::FontMetrics.new(font())
	    size = fm.size(Qt5::TextSingleLine, text)
	
	    image = Qt5::Image.new(size.width() + 12, size.height() + 12, Qt5::Image::Format_ARGB32_Premultiplied)
	    image.fill(qRgba(0, 0, 0, 0))
	
	    font = Qt5::Font.new
	    font.styleStrategy = Qt5::Font::ForceOutline
	
	    painter = Qt5::Painter.new
	    painter.begin(image)
	    painter.renderHint = Qt5::Painter::Antialiasing
	    painter.brush = Qt5::Brush.new(Qt5::white)
	    painter.drawRoundRect(Qt5::RectF.new(0.5, 0.5, image.width()-1, image.height()-1), 25, 25)
	
	    painter.font = font
	    painter.brush = Qt5::Brush.new(Qt5::black)
	    painter.drawText(Qt5::Rect.new(Qt5::Point.new(6, 6), size), Qt5::AlignCenter, text)
	    painter.end
	
	    setPixmap(Qt5::Pixmap.fromImage(image))
	    @labelText = text
	end
	
	def mousePressEvent(event)
	    itemData = Qt5::ByteArray.new("")
	    dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::WriteOnly.to_i)
		hotSpot = Qt5::Point.new(event.pos.x, event.pos.y)
		hotSpot -= rect.topLeft
	    dataStream << @labelText << hotSpot

	    mimeData = Qt5::MimeData.new
	    mimeData.setData("application/x-fridgemagnet", itemData)
	    mimeData.text = @labelText
	
	    drag = Qt5::Drag.new(self)
	    drag.mimeData = mimeData
	    drag.hotSpot = hotSpot
	    drag.pixmap = pixmap.copy
	
	    hide()
	
	    if drag.start(Qt5::MoveAction.to_i) == Qt5::MoveAction
	        close()
	    else
	        show()
		end
	end
end
