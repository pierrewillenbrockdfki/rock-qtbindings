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

require './pixeldelegate.rb'
require './imagemodel.rb'

class MainWindow < Qt5::MainWindow
    
    slots   :chooseImage,
            :printImage,
            :showAboutBox,
			:updateView
    
    def initialize()
        super
        @currentPath = Qt5::Dir.home.absolutePath
        @model = ImageModel.new(Qt5::Image.new, self)
    
		centralWidget = Qt5::Widget.new

        @view = Qt5::TableView.new
        @view.showGrid = false
        @view.horizontalHeader.hide
        @view.verticalHeader.hide

        delegate = PixelDelegate.new(self)
        @view.itemDelegate = delegate

    	pixelSizeLabel = Qt5::Label.new(tr("Pixel size:"))
    	pixelSizeSpinBox = Qt5::SpinBox.new do |s|
    		s.minimum = 1
    		s.maximum = 32
    		s.value = 12
		end
    
        fileMenu = Qt5::Menu.new(tr("&File"), self)
        openAction = fileMenu.addAction(tr("&Open..."))
        openAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+O"))
    
        @printAction = fileMenu.addAction(tr("&Print..."))
        @printAction.enabled = false
        @printAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+P"))
    
        quitAction = fileMenu.addAction(tr("E&xit"))
        quitAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+Q"))
    
        helpMenu = Qt5::Menu.new(tr("&Help"), self)
        aboutAction = helpMenu.addAction(tr("&About"))
    
        menuBar.addMenu(fileMenu)
        menuBar.addSeparator
        menuBar.addMenu(helpMenu)
    
        connect(openAction, SIGNAL(:triggered), self, SLOT(:chooseImage))
        connect(@printAction, SIGNAL(:triggered), self, SLOT(:printImage))
        connect(quitAction, SIGNAL(:triggered), $qApp, SLOT(:quit))
        connect(aboutAction, SIGNAL(:triggered), self, SLOT(:showAboutBox))
    	connect(pixelSizeSpinBox, SIGNAL('valueChanged(int)'),
            delegate, SLOT('pixelSize=(int)'))
    	connect(pixelSizeSpinBox, SIGNAL('valueChanged(int)'),
            self, SLOT(:updateView))

		controlsLayout = Qt5::HBoxLayout.new do |c|
			c.addWidget(pixelSizeLabel)
			c.addWidget(pixelSizeSpinBox)
			c.addStretch(1)
		end
	
		centralWidget.layout = Qt5::VBoxLayout.new do |m|
			m.addWidget(@view)
			m.addLayout(controlsLayout)
		end
	
		self.centralWidget = centralWidget
  
        setWindowTitle(tr("Pixelator"))
        resize(640, 480)
    end
    
    def chooseImage
        fileName = Qt5::FileDialog.getOpenFileName(self,
            tr("Choose an image"), @currentPath, "*")
    
        if !fileName.nil?
            if openImage(fileName)
                @currentPath = fileName
            end
        end
    end
    
    def openImage(fileName)
        image = Qt5::Image.new
    
        if image.load(fileName)
            @model = ImageModel.new(image)
            @view.model = @model
            
            if fileName !~ %r{^:/}
                @currentPath = fileName
                setWindowTitle("%s - Pixelator" % @currentPath)
            end
    
            @printAction.enabled = true
    		updateView
        end
    end
    
    def printImage()
        if @model.rowCount(Qt5::ModelIndex.new()).model.columnCount(Qt5::ModelIndex.new()) > 90000
            answer = Qt5::MessageBox::question(self, tr("Large Image Size"),
                tr("The printed image may be very large. Are you sure that " \
                   "you want to print it?"),
                Qt5::MessageBox::Yes, Qt5::MessageBox::No)
            if answer == Qt5::MessageBox::No
                return
            end
        end
    
        printer = Qt5::Printer.new(Qt5::Printer::HighResolution)
    
        dlg = Qt5::PrintDialog.new(printer, self)
        dlg.windowTitle = tr("Print Image")
    
        if dlg.exec != Qt5::Dialog::Accepted
            return
        end
    
        painter = Qt5::Painter.new
        painter.begin(printer)
    
        rows = @model.rowCount(Qt5::ModelIndex.new)
        columns = @model.columnCount(Qt5::ModelIndex.new)
        sourceWidth = (columns+1) * PixelDelegate::ItemSize
        sourceHeight = (rows+1) * PixelDelegate::ItemSize
    
        painter.save
    
        xscale = printer.pageRect.width/sourceWidth.to_f
        yscale = printer.pageRect.height/sourceHeight.to_f
        scale = [xscale, yscale].min
    
        painter.translate(printer.paperRect.x + printer.pageRect.width/2,
                          printer.paperRect.y + printer.pageRect.height/2)
        painter.scale(scale, scale)
        painter.translate(-sourceWidth/2, -sourceHeight/2)
    
        option = Qt5::StyleOptionViewItem.new
        parent = Qt5::ModelIndex.new
    
        progress = Qt5::ProgressDialog.new(tr("Printing..."), tr("Cancel"), 0, rows, self)
        y = PixelDelegate::ItemSize/2
    
        for row in 0...rows do
            progress.value = row
            $qApp.processEvents
            if progress.wasCanceled
                break
            end
    
            x = PixelDelegate::ItemSize/2
    
            for column in 0...columns do
                option.rect = Qt5::Rect.new(x.to_i, y.to_i, PixelDelegate::ItemSize, PixelDelegate::ItemSize)
                @view.itemDelegate().paint(painter, option,
                                            @model.index(row, column, parent))
                x += PixelDelegate::ItemSize
            end
            y += PixelDelegate::ItemSize
        end
        progress.value = rows
    
        painter.restore
        painter.end
    
        if progress.wasCanceled()
            Qt5::MessageBox.information(self, tr("Printing canceled"),
                tr("The printing process was canceled."), Qt5::MessageBox::Cancel)
        end
    end
    
    def showAboutBox()
        Qt5::MessageBox.about(self, tr("About the Pixelator example"),
            tr("This example demonstrates how a standard view and a custom\n" \
               "delegate can be used to produce a specialized representation\n " \
               "of data in a simple custom model."))
    end

	def updateView
        for row in 0...@model.rowCount(Qt5::ModelIndex.new) do
            @view.resizeRowToContents(row)
        end
        for column in 0...@model.columnCount(Qt5::ModelIndex.new) do
            @view.resizeColumnToContents(column)
        end
	end
end
