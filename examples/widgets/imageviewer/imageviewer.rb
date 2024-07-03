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


class ImageViewer < Qt5::MainWindow

slots   'open()',
        'print()',
        'zoomIn()',
        'zoomOut()',
        'normalSize()',
        'fitToWindow()',
        'about()'

	def initialize(parent = nil)
		super(parent)
		@imageLabel = Qt5::Label.new
		@imageLabel.backgroundRole = Qt5::Palette::Base
		@imageLabel.setSizePolicy(Qt5::SizePolicy::Ignored, Qt5::SizePolicy::Ignored)
		@imageLabel.scaledContents = true
	
		@scrollArea = Qt5::ScrollArea.new
		@scrollArea.backgroundRole = Qt5::Palette::Dark
		@scrollArea.widget = @imageLabel
		setCentralWidget(@scrollArea)
	
		createActions()
		createMenus()
	
		setWindowTitle(tr("Image Viewer"))
		resize(500, 400)
		@printer = Qt5::Printer.new
	end
	
	def open()
		fileName = Qt5::FileDialog::getOpenFileName(self,
										tr("Open File"), Qt5::Dir.currentPath())
		if !fileName.nil?
			image = Qt5::Image.new(fileName)
			if image.nil?
				Qt5::MessageBox.information(self, tr("Image Viewer"),
										tr("Cannot load %s." % fileName))
				return
			end
			@imageLabel.pixmap = Qt5::Pixmap.fromImage(image)
			@scaleFactor = 1.0
	
			@printAct.enabled = true
			@fitToWindowAct.enabled = true
			updateActions()
	
			if !@fitToWindowAct.checked?
				@imageLabel.adjustSize
			end
		end
	end
	
	def print()
		dialog = Qt5::PrintDialog.new(@printer, self)
		if dialog.exec
			painter = Qt5::Painter.new(@printer)
			rect = painter.viewport
			size = @imageLabel.pixmap.size
			size.scale(rect.size(), Qt5::KeepAspectRatio)
			painter.viewport = Qt5::Rect.new(rect.x, rect.y, size.width, size.height)
			painter.window = @imageLabel.pixmap.rect
			painter.drawPixmap(0, 0, @imageLabel.pixmap)
		end
	end
	
	def zoomIn()
		scaleImage(1.25)
	end
	
	def zoomOut()
		scaleImage(0.8)
	end
	
	def normalSize()
		@imageLabel.adjustSize()
		@scaleFactor = 1.0
	end
	
	def fitToWindow()
		fitToWindow = @fitToWindowAct.checked?
		@scrollArea.widgetResizable = fitToWindow
		if !fitToWindow
			@imageLabel.adjustSize()
		end
	
		updateActions()
	end
	
	
	def about()
		Qt5::MessageBox::about(self, tr("About Image Viewer"),
				tr("<p>The <b>Image Viewer</b> example shows how to combine Qt5::Label " +
				"and Qt5::ScrollArea to display an image. Qt5::Label is typically used " +
				"for displaying a text, but it can also display an image. " +
				"Qt5::ScrollArea provides a scrolling view around another widget. " +
				"If the child widget exceeds the size of the frame, Qt5::ScrollArea " +
				"automatically provides scroll bars. </p><p>The example " +
				"demonstrates how Qt5::Label's ability to scale its contents " +
				"(Qt5::Label::scaledContents), and Qt5::ScrollArea's ability to " +
				"automatically resize its contents " +
				"(Qt5::ScrollArea.widgetResizable), can be used to implement " +
				"zooming and scaling features. </p><p>In addition the example " +
				"shows how to use Qt5::Painter to print an image.</p>"))
	end
	
	def createActions()
		@openAct = Qt5::Action.new(tr("&Open..."), self)
		@openAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+O"))
		connect(@openAct, SIGNAL('triggered()'), self, SLOT('open()'))
	
		@printAct = Qt5::Action.new(tr("&Print..."), self)
		@printAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+P"))
		@printAct.enabled = false
		connect(@printAct, SIGNAL('triggered()'), self, SLOT('print()'))
	
		@exitAct = Qt5::Action.new(tr("E&xit"), self)
		@exitAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+Q"))
		connect(@exitAct, SIGNAL('triggered()'), self, SLOT('close()'))
	
		@zoomInAct = Qt5::Action.new(tr("Zoom &In (25%)"), self)
		@zoomInAct.shortcut = Qt5::KeySequence.new(tr("Ctrl++"))
		@zoomInAct.enabled = false
		connect(@zoomInAct, SIGNAL('triggered()'), self, SLOT('zoomIn()'))
	
		@zoomOutAct = Qt5::Action.new(tr("Zoom &Out (25%)"), self)
		@zoomOutAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+-"))
		@zoomOutAct.enabled = false
		connect(@zoomOutAct, SIGNAL('triggered()'), self, SLOT('zoomOut()'))
	
		@normalSizeAct = Qt5::Action.new(tr("&Normal Size"), self)
		@normalSizeAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+S"))
		@normalSizeAct.enabled = false
		connect(@normalSizeAct, SIGNAL('triggered()'), self, SLOT('normalSize()'))
	
		@fitToWindowAct = Qt5::Action.new(tr("&Fit to Window"), self)
		@fitToWindowAct.enabled = false
		@fitToWindowAct.checkable = true
		@fitToWindowAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+F"))
		connect(@fitToWindowAct, SIGNAL('triggered()'), self, SLOT('fitToWindow()'))
	
		@aboutAct = Qt5::Action.new(tr("&About"), self)
		connect(@aboutAct, SIGNAL('triggered()'), self, SLOT('about()'))
	
		@aboutQtAct = Qt5::Action.new(tr("About &Qt"), self)
		connect(@aboutQtAct, SIGNAL('triggered()'), $qApp, SLOT('aboutQt()'))
	end
	
	def createMenus()
		@fileMenu = Qt5::Menu.new(tr("&File"), self)
		@fileMenu.addAction(@openAct)
		@fileMenu.addAction(@printAct)
		@fileMenu.addSeparator()
		@fileMenu.addAction(@exitAct)
	
		@viewMenu = Qt5::Menu.new(tr("&View"), self)
		@viewMenu.addAction(@zoomInAct)
		@viewMenu.addAction(@zoomOutAct)
		@viewMenu.addAction(@normalSizeAct)
		@viewMenu.addSeparator()
		@viewMenu.addAction(@fitToWindowAct)
	
		@helpMenu = Qt5::Menu.new(tr("&Help"), self)
		@helpMenu.addAction(@aboutAct)
		@helpMenu.addAction(@aboutQtAct)
	
		menuBar().addMenu(@fileMenu)
		menuBar().addMenu(@viewMenu)
		menuBar().addMenu(@helpMenu)
	end
	
	def updateActions()
		@zoomInAct.enabled = !@fitToWindowAct.checked?
		@zoomOutAct.enabled = !@fitToWindowAct.checked?
		@normalSizeAct.enabled = !@fitToWindowAct.checked?
	end
	
	def scaleImage(factor)
		@scaleFactor *= factor
		@imageLabel.resize(@imageLabel.pixmap().size() * @scaleFactor)
	
		adjustScrollBar(@scrollArea.horizontalScrollBar(), factor)
		adjustScrollBar(@scrollArea.verticalScrollBar(), factor)
	
		@zoomInAct.enabled = @scaleFactor < 3.0
		@zoomOutAct.enabled = @scaleFactor > 0.333
	end
	
	def adjustScrollBar(scrollBar, factor)
		scrollBar.value = factor * scrollBar.value
								+ ((factor - 1) * scrollBar.pageStep/2)
	end
end
