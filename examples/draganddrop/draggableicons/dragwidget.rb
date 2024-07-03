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

class DragWidget < Qt5::Frame
	
	def initialize(parent = nil, flags = 0)
	    super(parent, flags)
	    setMinimumSize(200, 200)
	    setFrameStyle(Qt5::Frame::Sunken | Qt5::Frame::StyledPanel)
	    setAcceptDrops(true)
	
	    boatIcon = Qt5::Label.new(self)
	    boatIcon.pixmap = Qt5::Pixmap.new("images/boat.png")
	    boatIcon.move(20, 20)
	    boatIcon.show()
	    boatIcon.attribute = Qt5::WA_DeleteOnClose
	
	    carIcon = Qt5::Label.new(self)
	    carIcon.pixmap = Qt5::Pixmap.new("images/car.png")
	    carIcon.move(120, 20)
	    carIcon.show()
	    carIcon.attribute = Qt5::WA_DeleteOnClose
	
	    houseIcon = Qt5::Label.new(self)
	    houseIcon.pixmap = Qt5::Pixmap.new("images/house.png")
	    houseIcon.move(20, 120)
	    houseIcon.show()
	    houseIcon.attribute = Qt5::WA_DeleteOnClose
	end
	
	def dragEnterEvent(event)
	    if event.mimeData().hasFormat("application/x-dnditemdata")
	        if event.source() == self
	            event.dropAction = Qt5::MoveAction
	            event.accept()
	        else
	            event.acceptProposedAction()
	        end
	    else
	        event.ignore()
	    end
	end
	
	def dropEvent(event)
	    if event.mimeData().hasFormat("application/x-dnditemdata")
	        itemData = event.mimeData().data("application/x-dnditemdata")
	        dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::ReadOnly.to_i)
	        
	        pixmap = Qt5::Pixmap.new
	        offset = Qt5::Point.new
	        dataStream >> pixmap >> offset
	        newIcon = Qt5::Label.new(self)
	        newIcon.pixmap = pixmap
	        newIcon.move(event.pos() - offset)
	        newIcon.show()
	        newIcon.attribute = Qt5::WA_DeleteOnClose
	
	        if event.source() == self
	            event.dropAction = Qt5::MoveAction
	            event.accept()
	        else
	            event.acceptProposedAction()
	        end
	    else
	        event.ignore()
	    end
	end
	
	def mousePressEvent(event)
	    child = childAt(event.pos())
	    if child.nil?
	        return
		end
	
	    pixmap = child.pixmap.copy
	
	    itemData = Qt5::ByteArray.new("")
	    dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::WriteOnly.to_i)
	    dataStream << pixmap << (event.pos() - child.pos())
	    mimeData = Qt5::MimeData.new
	    mimeData.setData("application/x-dnditemdata", itemData)
	        
	    drag = Qt5::Drag.new(self)
	    drag.mimeData = mimeData
	    drag.pixmap = pixmap
	    drag.hotSpot = event.pos - child.pos
	
	    tempPixmap = pixmap.copy
	    painter = Qt5::Painter.new
	    painter.begin(tempPixmap)
	    painter.fillRect(pixmap.rect(), Qt5::Brush.new(Qt5::Color.new(127, 127, 127, 127)))
	    painter.end
	
	    child.pixmap = tempPixmap
	
	    if drag.start(Qt5::CopyAction | Qt5::MoveAction) == Qt5::MoveAction
	        child.close()
	    else
	        child.show()
	        child.pixmap = pixmap
	    end
	end
end
