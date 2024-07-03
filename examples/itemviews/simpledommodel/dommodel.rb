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
        
require './domitem.rb'
    
class DomModel < Qt5::AbstractItemModel

    def initialize(document, parent)
        super(parent)
        @domDocument = document
        @rootItem = DomItem.new(@domDocument, 0)
    end
    
    def columnCount(parent = Qt5::ModelIndex.new)
        return 3
    end
    
    def data(index, role)
        if !index.valid?
            return Qt5::Variant.new
        end
    
        if role != Qt5::DisplayRole
            return Qt5::Variant.new
        end
    
        item = index.internalPointer
        node = item.node
        attributes = []
        attributeMap = node.attributes
    
        case index.column
        when 0
            return Qt5::Variant.new(node.nodeName)
        when 1
            for i in 0...attributeMap.length
                attribute = attributeMap.item(i)
                attributes << attribute.nodeName() + '="' + 
                                attribute.nodeValue() + '"'
            end
            return Qt5::Variant.new(attributes.join(" "))
        when 2
            if node.nodeValue.nil?
                return Qt5::Variant.new
            else
                return Qt5::Variant.new(node.nodeValue().split("\n").join(" "))
            end
        else
            return Qt5::Variant.new
        end
    end
    
    def flags(index)
        if !index.valid?
            return Qt5::ItemIsEnabled
        end
    
        return Qt5::ItemIsEnabled | Qt5::ItemIsSelectable
    end
    
    def headerData(section, orientation, role = Qt5::DisplayRole)
        if orientation == Qt5::Horizontal && role == Qt5::DisplayRole
            case section
            when 0
                return Qt5::Variant.new(tr("Name"))
            when 1
                return Qt5::Variant.new(tr("Attributes"))
            when 2
                return Qt5::Variant.new(tr("Value"))
            else
                return Qt5::Variant.new
            end
        end
    
        return Qt5::Variant.new
    end
    
    def index(row, column, parent = Qt5::ModelIndex.new)
        if !parent.valid?
            parentItem = @rootItem
        else
            parentItem = parent.internalPointer()
        end
    
        childItem = parentItem.child(row)
        if ! childItem.nil?
            return createIndex(row, column, childItem)
        else
            return Qt5::ModelIndex.new
        end
    end
    
    def parent(child)
        if !child.valid?
            return Qt5::ModelIndex.new
        end
    
        childItem = child.internalPointer()
        parentItem = childItem.parent()
    
        if parentItem.nil? || parentItem == @rootItem
            return Qt5::ModelIndex.new
        end
    
        return createIndex(parentItem.row(), 0, parentItem)
    end
    
    def rowCount(parent = Qt5::ModelIndex.new)
        if !parent.valid?
            parentItem = @rootItem
        else
            parentItem = parent.internalPointer
        end
    
        return parentItem.node().childNodes().length
    end
end
