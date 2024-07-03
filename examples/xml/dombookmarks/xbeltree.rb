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
	
	
	
class XbelTree < Qt5::TreeWidget
	
	slots	'updateDomElement(QTreeWidgetItem *, int)'
	
	IndentSize = 4
	
	def initialize(parent = nil)
	    super(parent)
	    labels = []
	    labels << tr("Title") << tr("Location")
	
	    header().setSectionResizeMode(0, Qt5::HeaderView::Stretch)
	    setHeaderLabels(labels)
	
		@folderIcon = Qt5::Icon.new
	    @folderIcon.addPixmap(style().standardPixmap(Qt5::Style::SP_DirClosedIcon),
	                         Qt5::Icon::Normal, Qt5::Icon::Off)
	    @folderIcon.addPixmap(style().standardPixmap(Qt5::Style::SP_DirOpenIcon),
	                         Qt5::Icon::Normal, Qt5::Icon::On)
		@bookmarkIcon = Qt5::Icon.new
	    @bookmarkIcon.addPixmap(style().standardPixmap(Qt5::Style::SP_FileIcon))

		@domElementForItem = {}
		@domDocument = Qt5::DomDocument.new
	end
	
	def read(device)
	    errorStr = ""
	    errorLine = Qt5::Integer.new
	    errorColumn = Qt5::Integer.new
	
	    if !@domDocument.setContent(device, true, errorStr, errorLine,
	                                errorColumn)
	        Qt5::MessageBox.information(window(), tr("DOM Bookmarks"),
	                                 tr("Parse error at line %d, column %d:\n%s" %
	                                 [errorLine, errorColumn, errorStr]))
	        return false
	    end
	
	    root = @domDocument.documentElement()
	    if root.tagName() != "xbel"
	        Qt5::MessageBox.information(window(), tr("DOM Bookmarks"),
	                                 tr("The file is not an XBEL file."))
	        return false
	    elsif root.hasAttribute("version") &&
	               root.attribute("version") != "1.0"
	        Qt5::MessageBox.information(window(), tr("DOM Bookmarks"),
	                                 tr("The file is not an XBEL version 1.0 " \
	                                    "file."))
	        return false
	    end
	
	    clear()
	
	    disconnect(self, SIGNAL('itemChanged(QTreeWidgetItem *, int)'),
	               self, SLOT('updateDomElement(QTreeWidgetItem *, int)'))
	
	    child = root.firstChildElement("folder")
	    while !child.null?
	        parseFolderElement(child)
	        child = child.nextSiblingElement("folder")
	    end
	
	    connect(self, SIGNAL('itemChanged(QTreeWidgetItem *, int)'),
	            self, SLOT('updateDomElement(QTreeWidgetItem *, int)'))
	
	    return true
	end
	
	def write(device)
	    outf = Qt5::TextStream.new(device)
	    @domDocument.save(outf, IndentSize)
	    return true
	end
	
	def updateDomElement(item, column)
	    element = @domElementForItem[item]
	    if !element.nil?
	        if column == 0
	            oldTitleElement = element.firstChildElement("title")
	            newTitleElement = @domDocument.createElement("title")
	
	            newTitleText = @domDocument.createTextNode(item.text(0))
	            newTitleElement.appendChild(newTitleText)
	
	            element.replaceChild(newTitleElement, oldTitleElement)
	        else
	            if element.tagName == "bookmark"
	                element.setAttribute("href", item.text(1))
				end
	        end
	    end
	end
	
	def parseFolderElement(element, parentItem = nil)
	    item = createItem(element, parentItem)
	
	    title = element.firstChildElement("title").text()
	    if title.nil?
	        title = Qt5::Object.tr("Folder")
		end
	
	    item.flags = item.flags | Qt5::ItemIsEditable.to_i
	    item.setIcon(0, @folderIcon)
	    item.setText(0, title)
	
	    folded = (element.attribute("folded") != "no")
	    setItemExpanded(item, !folded)
	
	    child = element.firstChildElement()
	    while !child.null?
	        if child.tagName() == "folder"
	            parseFolderElement(child, item)
	        elsif child.tagName() == "bookmark"
	            childItem = createItem(child, item)
	
	            title = child.firstChildElement("title").text()
	            if title.empty?
	                title = Qt5::Object.tr("Folder")
				end
	
	            childItem.flags = item.flags | Qt5::ItemIsEditable.to_i
	            childItem.setIcon(0, @bookmarkIcon)
	            childItem.setText(0, title)
	            childItem.setText(1, child.attribute("href"))
	        elsif child.tagName() == "separator"
	            childItem = createItem(child, item)
	            childItem.flags = item.flags & ~(Qt5::ItemIsSelectable.to_i | Qt5::ItemIsEditable.to_i)
#	            childItem.setText(0, Qt5::String(30, 0xB7))
	            childItem.setText(0, "..............................")
	        end
	        child = child.nextSiblingElement()
	    end
	end
	
	def createItem(element, parentItem)
	    if !parentItem.nil?
	        item = Qt5::TreeWidgetItem.new(parentItem, Qt5::TreeWidgetItem::Type)
	    else
	        item = Qt5::TreeWidgetItem.new(self)
	    end
	    @domElementForItem[item] = element
	    return item
	end
end
