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
	
class PiecesModel < Qt5::AbstractListModel
    
	RAND_MAX = 2147483647
	
	def initialize(parent)
	    super(parent)
		@locations = []
		@pixmaps = []
	end
	
	def data(index, role)
	    if !index.valid?
	        return Qt5::Variant.new
		end
	
	    if role == Qt5::DecorationRole
	        return qVariantFromValue(Qt5::Icon.new(@pixmaps[index.row].scaled(60, 60,
	                         Qt5::KeepAspectRatio, Qt5::SmoothTransformation)))
	    elsif role == Qt5::UserRole
	        return qVariantFromValue(@pixmaps[index.row])
	    elsif role == Qt5::UserRole + 1
	        return qVariantFromValue(@locations[index.row])
		end
	
	    return Qt5::Variant.new
	end
	
	def addPiece(pixmap, location)
	    if (2.0*rand(RAND_MAX)/(RAND_MAX+1.0)).to_i == 1
	        beginInsertRows(Qt5::ModelIndex.new, 0, 0)
	        @pixmaps.unshift(pixmap)
	        @locations.unshift(location)
	        endInsertRows()
	    else
	        beginInsertRows(Qt5::ModelIndex.new, @pixmaps.size, @pixmaps.size)
	        @pixmaps.push(pixmap)
	        @locations.push(location)
	        endInsertRows()
	    end

	end
	
	def flags(index)
	    if index.valid?
	        return (Qt5::ItemIsEnabled | Qt5::ItemIsSelectable |
	                Qt5::ItemIsDragEnabled | Qt5::ItemIsSelectable | Qt5::ItemIsDropEnabled)
	    end
	
	    return Qt5::ItemIsEnabled | Qt5::ItemIsDropEnabled
	end
	
	def removeRows(row, count, parent)
	    if parent.valid?
	        return false
		end
	
	    if row >= @pixmaps.size() || row + count <= 0
	        return false
		end
	
	    beginRow = [0, row].max
	    endRow = [row + count - 1, @pixmaps.size() - 1].min
	
	    beginRemoveRows(parent, beginRow, endRow)
	
	    while beginRow <= endRow
	        @pixmaps.delete(beginRow)
	        @locations.delete(beginRow)
	        beginRow += 1
	    end
	
	    endRemoveRows()
	    return true
	end
	
	def mimeTypes()
	    types = []
	    types << "image/x-puzzle-piece"
	    return types
	end
	
	def mimeData(indexes)
	    mimeData = Qt5::MimeData.new
	    encodedData = Qt5::ByteArray.new
	
	    stream = Qt5::DataStream.new(encodedData, Qt5::IODevice::WriteOnly)
	
	    indexes.each do |index|
	        if index.valid?
	            pixmap = qVariantValue(Qt5::Pixmap, data(index, Qt5::UserRole))
	            location = data(index, Qt5::UserRole+1).toPoint
	            stream << pixmap << location
	        end
	    end
	
	    mimeData.setData("image/x-puzzle-piece", encodedData)
	    return mimeData
	end
	
	def dropMimeData(data, action, row, column, parent)
	    if !data.hasFormat("image/x-puzzle-piece")
	        return false
		end
	
	    if action == Qt5::IgnoreAction
	        return true
		end
	
	    if column > 0
	        return false
		end
	
	    if !parent.valid? && row < 0
	        endRow = @pixmaps.size
	    elsif !parent.valid?
	        endRow = [row, pixmaps.size()].min
	    else
	        endRow = parent.row
		end
	
	    encodedData = data.data("image/x-puzzle-piece")
	    stream = Qt5::DataStream.new(encodedData, Qt5::IODevice::ReadOnly)
	
	    while !stream.atEnd
	        pixmap = Qt5::Pixmap.new
	        location = Qt5::Point.new
	        stream >> pixmap >> location
	
	        beginInsertRows(Qt5::ModelIndex.new, endRow, endRow)
	        @pixmaps.insert(endRow, pixmap)
	        @locations.insert(endRow, location)
	        endInsertRows()
	
	        endRow += 1
	    end
	
	    return true
	end
	
	def rowCount(parent)
	    if parent.valid?
	        return 0
	    else
	        return @pixmaps.size
		end
	end
	
	def supportedDropActions
	    return Qt5::CopyAction | Qt5::MoveAction
	end
end
