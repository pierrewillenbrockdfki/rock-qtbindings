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
    
    
class PuzzleWidget < Qt5::Widget
    
    signals 'puzzleCompleted()'
    
    def initialize(parent = nil)
        super(parent)
        setAcceptDrops(true)
        setMinimumSize(400, 400)
        setMaximumSize(400, 400)

        @pieceLocations = []
        @piecePixmaps = []
        @pieceRects = []
    end
    
    def clear()
        @pieceLocations.clear()
        @piecePixmaps.clear()
        @pieceRects.clear()
        @highlightedRect = Qt5::Rect.new()
        @inPlace = 0
        update()
    end

	def dragEnterEvent(event)
    	if event.mimeData.hasFormat("image/x-puzzle-piece")
        	event.accept
    	else
        	event.ignore
		end
	end
    
    def dragLeaveEvent(event)
        updateRect = @highlightedRect
        @highlightedRect = Qt5::Rect.new()
        update(updateRect)
        event.accept()
    end
    
    def dragMoveEvent(event)
        updateRect = @highlightedRect.united(targetSquare(event.pos()))
    
        if event.mimeData().hasFormat("image/x-puzzle-piece") &&
            findPiece(targetSquare(event.pos)) == -1
    
            @highlightedRect = targetSquare(event.pos())
puts "dragMoveEvent event.dropAction = Qt5::MoveAction"
            event.dropAction = Qt5::MoveAction
            event.accept()
        else
            @highlightedRect = Qt5::Rect.new()
            event.ignore()
        end
    
        update(updateRect)
    end
    
    def dropEvent(event)
        if event.mimeData().hasFormat("image/x-puzzle-piece") &&
            findPiece(targetSquare(event.pos)) == -1
    
            pieceData = event.mimeData().data("image/x-puzzle-piece")
            dataStream = Qt5::DataStream.new(pieceData, Qt5::IODevice::ReadOnly.to_i)
            square = targetSquare(event.pos)
            pixmap = Qt5::Pixmap.new
            location = Qt5::Point.new
            dataStream >> pixmap >> location
    
            @pieceLocations.push location
            @piecePixmaps.push pixmap
            @pieceRects.push square
    
            @highlightedRect = Qt5::Rect.new()
            update(square)
    
puts "dropEvent event.dropAction = Qt5::MoveAction"
            event.dropAction = Qt5::MoveAction
            event.accept()
p event.dropAction    
            if location == Qt5::Point.new(square.x()/80, square.y()/80)
                @inPlace += 1
                if @inPlace == 25
                    emit puzzleCompleted()
                end
            end
        else
            @highlightedRect = Qt5::Rect.new()
            event.ignore()
        end
    end
    
    def findPiece(pieceRect)
        (0...@pieceRects.size).each do |i|
            if pieceRect == @pieceRects[i]
                return i
            end
        end
        return -1
    end
    
    def mousePressEvent(event)
        square = targetSquare(event.pos())
        found = findPiece(square)
    
        if found == -1
            return
        end
    
        location = @pieceLocations[found]
        pixmap = @piecePixmaps[found]
        @pieceLocations.delete_at(found)
        @piecePixmaps.delete_at(found)
        @pieceRects.delete_at(found)
    
        if location == Qt5::Point.new(square.x()/80, square.y()/80)
            @inPlace -= 1
        end
    
        update(square)
    
        itemData = Qt5::ByteArray.new("")
        dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::WriteOnly.to_i)
    
        dataStream << pixmap << location
    
        mimeData = Qt5::MimeData.new
        mimeData.setData("image/x-puzzle-piece", itemData)
    
        drag = Qt5::Drag.new(self)
        drag.mimeData = mimeData
        drag.hotSpot = event.pos - square.topLeft()
        drag.pixmap = pixmap
    
        if drag.start(Qt5::MoveAction) == 0
            @pieceLocations.insert(found, location)
            @piecePixmaps.insert(found, pixmap)
            @pieceRects.insert(found, square)
            update(targetSquare(event.pos()))

			if (location == Qt5::Point.new(square.x()/80, square.y()/80))
				@inPlace += 1
			end
        end
    end
    
    def paintEvent(event)
        painter = Qt5::Painter.new
        painter.begin(self)
        painter.fillRect(event.rect(), Qt5::Brush.new(Qt5::white))
    
        if @highlightedRect.isValid()
            painter.brush = Qt5::Brush.new(Qt5::Color.new("#ffcccc"))
            painter.pen = Qt5::NoPen
            painter.drawRect(@highlightedRect.adjusted(0, 0, -1, -1))
        end
    
        (0...@pieceRects.size).each do |i|
            painter.drawPixmap(@pieceRects[i], @piecePixmaps[i])
        end
        painter.end
    end
    
     def targetSquare(position)
        return Qt5::Rect.new(position.x()/80 * 80, position.y()/80 * 80, 80, 80)
    end
end
