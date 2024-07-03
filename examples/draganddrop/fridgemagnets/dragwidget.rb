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

require './draglabel.rb'

class DragWidget < Qt5::Widget
	
	def initialize(parent = nil)
	    super(parent)
	    dictionaryFile = Qt5::File.new("words.txt")
	    dictionaryFile.open(Qt5::File::ReadOnly.to_i)
	    inputStream = Qt5::TextStream.new(dictionaryFile)
	
	    x = 5
	    y = 5
	
	    while !inputStream.atEnd()
	        word = ""
	        inputStream >> word
	        if !word.empty?
	            wordLabel = DragLabel.new(word, self)
	            wordLabel.move(x, y)
	            wordLabel.show()
	            x += wordLabel.width() + 2
	            if x >= 245
	                x = 5
	                y += wordLabel.height() + 2
	            end
	        end
	    end
	
	    newPalette = palette()
	    newPalette.setColor(Qt5::Palette::Background, Qt5::Color.new(Qt5::white))
	    setPalette(newPalette)
	
	    setAcceptDrops(true)
	    setMinimumSize(400, [200, y].max)
	    setWindowTitle(tr("Fridge Magnets"))
	end
	
	def dragEnterEvent(event)
	    if event.mimeData().hasFormat("application/x-fridgemagnet")
	        if children().include? event.source()
	            event.dropAction = Qt5::MoveAction
	            event.accept()
	        else
	            event.acceptProposedAction()
	        end
	    elsif event.mimeData().hasText()
	        event.acceptProposedAction()
	    else
	        event.ignore()
	    end
	end
	
	def dragMoveEvent(event)
	    if event.mimeData().hasFormat("application/x-fridgemagnet")
	        if children().include? event.source()
	            event.dropAction = Qt5::MoveAction
	            event.accept()
	        else
	            event.acceptProposedAction()
	        end
	    elsif event.mimeData().hasText()
	        event.acceptProposedAction()
	    else
	        event.ignore()
	    end
	end
	
	def dropEvent(event)
	    if event.mimeData().hasFormat("application/x-fridgemagnet")
	        itemData = event.mimeData().data("application/x-fridgemagnet")
	        dataStream = Qt5::DataStream.new(itemData, Qt5::IODevice::ReadOnly.to_i)
	        
	        text = ""
	        offset = Qt5::Point.new
	        dataStream >> text >> offset

	        newLabel = DragLabel.new(text.to_s, self)
	        newLabel.move(event.pos() - offset)
	        newLabel.show()
	
	        if children().include? event.source()
	            event.dropAction = Qt5::MoveAction
	            event.accept()
	        else
	            event.acceptProposedAction()
	        end
	    elsif event.mimeData().hasText()
	        pieces = event.mimeData().text().split(Qt5::RegExp("\\s+"),
	                             Qt5::String::SkipEmptyParts)
	        position = Qt5::Point.new(event.pos.x, event.pos.y)
	
			pieces.each do |piece|
	            newLabel = DragLabel.new(piece, self)
	            newLabel.move(position)
	            newLabel.show()
	
	            position += Qt5::Point.new(newLabel.width(), 0)
	        end
	
	        event.acceptProposedAction()
	    else
	        event.ignore()
	    end
	end
end
