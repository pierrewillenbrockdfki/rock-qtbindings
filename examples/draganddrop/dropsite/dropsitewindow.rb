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
		
require './dropsitewidget.rb'
	
class DropSiteWindow < Qt5::Widget
	
	slots 'updateFormatsTable(const QMimeData *)'
	
	def initialize(parent = nil)
	    super(parent)
	    @abstractLabel = Qt5::Label.new(tr("The Drop Site example accepts drops from other " \
	                                  "applications, and displays the MIME formats " \
	                                  "provided by the drag object."))
	    @abstractLabel.wordWrap = true
	    @abstractLabel.adjustSize()
	
	    @dropArea = DropArea.new
	    connect(@dropArea, SIGNAL('changed(const QMimeData*)'),
	            self, SLOT('updateFormatsTable(const QMimeData*)'))
	
	    labels = []
	    labels << tr("Format") << tr("Content")

	    @formatsTable = Qt5::TableWidget.new
        @formatsTable.setColumnCount(2)
        @formatsTable.setEditTriggers(Qt5::AbstractItemView::NoEditTriggers)
        @formatsTable.setHorizontalHeaderLabels(labels)
        @formatsTable.horizontalHeader.setStretchLastSection(true)

	    @quitButton = Qt5::PushButton.new(tr("Quit"))
	    @clearButton = Qt5::PushButton.new(tr("Clear"))

        @buttonBox = Qt5::DialogButtonBox.new
        @buttonBox.addButton(@clearButton, Qt5::DialogButtonBox::ActionRole)
        @buttonBox.addButton(@quitButton, Qt5::DialogButtonBox::RejectRole)

	    connect(@quitButton, SIGNAL('pressed()'), self, SLOT('close()'))
	    connect(@clearButton, SIGNAL('pressed()'), @dropArea, SLOT('clear()'))

	    @layout = Qt5::VBoxLayout.new do |l|
			l.addWidget(@abstractLabel)
			l.addWidget(@dropArea)
			l.addWidget(@formatsTable)
			l.addWidget(@buttonBox)
		end
	
	    setLayout(@layout)
	    setWindowTitle(tr("Drop Site"))
	    setMinimumSize(350, 500)
	end
	
	def updateFormatsTable(mimeData = nil)
	    @formatsTable.rowCount = 0
	
	    if mimeData.nil?
	        return
		end

	    formats = mimeData.formats()

	    formats.each do |format|
	        formatItem = Qt5::TableWidgetItem.new(format)
	        formatItem.flags = Qt5::ItemIsEnabled
	        formatItem.textAlignment = Qt5::AlignTop | Qt5::AlignLeft
		
	        text = ""
	        if format == "text/plain"
                text = mimeData.text.gsub(/\s+/, " ")
            elsif format == "text/html"
                text = mimeData.text.gsub(/\s+/, " ")
            elsif format == "text/uri-list"
                urlList = mimeData.urls
                urlList.each do |url|
                    text << url.path + " "
                end
	        else
	            data = mimeData.data(format)
	            hexdata = ""
                data.to_s.each_byte { |b| hexdata << ("%2.2x " % b) }
	            text << hexdata
	        end
	
	        row = @formatsTable.rowCount()
	        @formatsTable.insertRow(row)
	        @formatsTable.setItem(row, 0, Qt5::TableWidgetItem.new(format))
	        @formatsTable.setItem(row, 1, Qt5::TableWidgetItem.new(text))
	    end
	
	    @formatsTable.resizeColumnToContents(0)
	end
end
