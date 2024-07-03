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
    
    
class PiecesList < Qt5::ListWidget
    
    def initialize(parent = nil)
        super(parent)
        setDragEnabled(true)
        setViewMode(Qt5::ListView::IconMode)
        setIconSize(Qt5::Size.new(60, 60))
        setSpacing(10)
        setAcceptDrops(true)
        setDropIndicatorShown(true)
    end
    
    def dragMoveEvent(event)
        if event.mimeData().hasFormat("image/x-puzzle-piece")
            event.dropAction = Qt5::MoveAction
            event.accept()
        else
            event.ignore()
        end
    end
    
    def dropEvent(event)
        if event.mimeData().hasFormat("image/x-puzzle-piece")
            pieceData = event.mimeData().data("image/x-puzzle-piece")
            dataStream = Qt5::DataStream.new(pieceData, Qt5::IODevice::ReadOnly.to_i)
            pixmap = Qt5::Pixmap.new
            location = Qt5::Point.new
            dataStream >> pixmap >> location
    
            addPiece(pixmap, location)
    
            event.dropAction = Qt5::MoveAction
            event.accept()
        else
            event.ignore()
		end
    end
    
    def addPiece(pixmap, location)
        pieceItem = Qt5::ListWidgetItem.new(self)
        pieceItem.icon = Qt5::Icon.new(pixmap)
        pieceItem.setData(Qt5::UserRole, qVariantFromValue(pixmap))
        pieceItem.setData(Qt5::UserRole.to_i + 1, Qt5::Variant.new(location))
        pieceItem.setFlags( Qt5::ItemIsEnabled.to_i | Qt5::ItemIsSelectable.to_i |
                            Qt5::ItemIsDragEnabled.to_i )
    end
    
    def startDrag(supportedActions)
        item = currentItem()
        itemData = Qt5::ByteArray.new("")
        dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::WriteOnly.to_i)
        pixmap = qVariantValue(Qt5::Pixmap, item.data(Qt5::UserRole))
        location = item.data(Qt5::UserRole+1).toPoint()

        dataStream << pixmap << location

        mimeData = Qt5::MimeData.new
        mimeData.setData("image/x-puzzle-piece", itemData)
    
        drag = Qt5::Drag.new(self)
        drag.mimeData = mimeData
        drag.hotSpot = Qt5::Point.new(pixmap.width/2, pixmap.height/2)
        drag.pixmap = pixmap
    
        if drag.start(Qt5::MoveAction.to_i) == Qt5::MoveAction
            takeItem(row(item)).dispose
        end
    end
end
