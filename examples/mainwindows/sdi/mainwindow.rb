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
	
class MainWindow < Qt5::MainWindow

	slots   'newFile()',
    		'open()',
    		'save()',
    		'saveAs()',
    		'about()',
    		'documentWasModified()'
	
	attr_reader :curFile

	@@sequenceNumber = 0
	
	def initialize(fileName = "")
		super()
	    init()

		if fileName.empty?
			setCurrentFile("")
		else
	    	loadFile(fileName)
		end
	end
	
	def closeEvent(event)
	    if maybeSave()
	        writeSettings()
	        event.accept()
	    else
	        event.ignore()
	    end
	end
	
	def newFile()
	    other = MainWindow.new
	    other.move(x() + 40, y() + 40)
	    other.show()
	end
	
	def open()
	    fileName = Qt5::FileDialog.getOpenFileName(self)
	    if !fileName.nil?
	        existing = findMainWindow(fileName)
	        if !existing.nil?
	            existing.show()
	            existing.raise()
	            existing.activateWindow()
	            return
	        end
	
	        if @isUntitled && @textEdit.document().empty? &&
	                !isWindowModified()
	            loadFile(fileName)
	        else
	            other = MainWindow.new(fileName)
	            if other.isUntitled
	                other.dispose
	                return
	            end
	            other.move(x() + 40, y() + 40)
	            other.show()
	        end
	    end
	end
	
	def save()
	    if @isUntitled
	        return saveAs()
	    else
	        return saveFile(@curFile)
	    end
	end
	
	def saveAs()
	    fileName = Qt5::FileDialog.getSaveFileName(self, tr("Save As"),
	                                                    @curFile)
	    if fileName.nil?
	        return false
		end
	
	    return saveFile(fileName)
	end
	
	def about()
	   Qt5::MessageBox::about(self, tr("About SDI"),
	            tr("The <b>SDI</b> example demonstrates how to write single " +
	               "document interface applications using Qt."))
	end
	
	def documentWasModified()
	    setWindowModified(true)
	end
	
	def init()
	    setAttribute(Qt5::WA_DeleteOnClose)
	
	    @isUntitled = true
	
	    @textEdit = Qt5::TextEdit.new
	    setCentralWidget(@textEdit)
	
	    createActions()
	    createMenus()
	    createToolBars()
	    createStatusBar()
	
	    readSettings()
	
	    connect(@textEdit.document(), SIGNAL('contentsChanged()'),
	            self, SLOT('documentWasModified()'))
	end
	
	def createActions()
	    @newAct = Qt5::Action.new(Qt5::Icon.new("images/new.png"), tr("&New"), self)
	    @newAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+N") )
	    @newAct.statusTip = tr("Create a file.new")
	    connect(@newAct, SIGNAL('triggered()'), self, SLOT('newFile()'))
	
	    @openAct = Qt5::Action.new(Qt5::Icon.new("images/open.png"), tr("&Open..."), self)
	    @openAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+O") )
	    @openAct.statusTip = tr("Open an existing file")
	    connect(@openAct, SIGNAL('triggered()'), self, SLOT('open()'))
	
	    @saveAct = Qt5::Action.new(Qt5::Icon.new("images/save.png"), tr("&Save"), self)
	    @saveAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+S") )
	    @saveAct.statusTip = tr("Save the document to disk")
	    connect(@saveAct, SIGNAL('triggered()'), self, SLOT('save()'))
	
	    @saveAsAct = Qt5::Action.new(tr("Save &As..."), self)
	    @saveAsAct.statusTip = tr("Save the document under a name.new")
	    connect(@saveAsAct, SIGNAL('triggered()'), self, SLOT('saveAs()'))
	
	    @closeAct = Qt5::Action.new(tr("&Close"), self)
	    @closeAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+W") )
	    @closeAct.statusTip = tr("Close self window")
	    connect(@closeAct, SIGNAL('triggered()'), self, SLOT('close()'))
	
	    @exitAct = Qt5::Action.new(tr("E&xit"), self)
	    @exitAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+Q") )
	    @exitAct.statusTip = tr("Exit the application")
	    connect(@exitAct, SIGNAL('triggered()'), $qApp, SLOT('closeAllWindows()'))
	
	    @cutAct = Qt5::Action.new(Qt5::Icon.new("images/cut.png"), tr("Cu&t"), self)
	    @cutAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+X") )
	    @cutAct.setStatusTip(tr("Cut the current selection's contents to the " +
	                            "clipboard"))
	    connect(@cutAct, SIGNAL('triggered()'), @textEdit, SLOT('cut()'))
	
	    @copyAct = Qt5::Action.new(Qt5::Icon.new("images/copy.png"), tr("&Copy"), self)
	    @copyAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+C") )
	    @copyAct.setStatusTip(tr("Copy the current selection's contents to the " +
	                             "clipboard"))
	    connect(@copyAct, SIGNAL('triggered()'), @textEdit, SLOT('copy()'))
	
	    @pasteAct = Qt5::Action.new(Qt5::Icon.new("images/paste.png"), tr("&Paste"), self)
	    @pasteAct.shortcut = Qt5::KeySequence.new( tr("Ctrl+V") )
	    @pasteAct.setStatusTip(tr("Paste the clipboard's contents into the current " +
	                              "selection"))
	    connect(@pasteAct, SIGNAL('triggered()'), @textEdit, SLOT('paste()'))
	
	    @aboutAct = Qt5::Action.new(tr("&About"), self)
	    @aboutAct.statusTip = tr("Show the application's About box")
	    connect(@aboutAct, SIGNAL('triggered()'), self, SLOT('about()'))
	
	    @aboutQtAct = Qt5::Action.new(tr("About &Qt"), self)
	    @aboutQtAct.statusTip = tr("Show the Qt library's About box")
	    connect(@aboutQtAct, SIGNAL('triggered()'), $qApp, SLOT('aboutQt()'))
	
	
	    @cutAct.enabled = false
	    @copyAct.enabled = false
	    connect(@textEdit, SIGNAL('copyAvailable(bool)'),
	            @cutAct, SLOT('setEnabled(bool)'))
	    connect(@textEdit, SIGNAL('copyAvailable(bool)'),
	            @copyAct, SLOT('setEnabled(bool)'))
	end
	
	def createMenus()
	    @fileMenu = menuBar().addMenu(tr("&File"))
	    @fileMenu.addAction(@newAct)
	    @fileMenu.addAction(@openAct)
	    @fileMenu.addAction(@saveAct)
	    @fileMenu.addAction(@saveAsAct)
	    @fileMenu.addSeparator()
	    @fileMenu.addAction(@closeAct)
	    @fileMenu.addAction(@exitAct)
	
	    @editMenu = menuBar().addMenu(tr("&Edit"))
	    @editMenu.addAction(@cutAct)
	    @editMenu.addAction(@copyAct)
	    @editMenu.addAction(@pasteAct)
	
	    menuBar().addSeparator()
	
	    @helpMenu = menuBar().addMenu(tr("&Help"))
	    @helpMenu.addAction(@aboutAct)
	    @helpMenu.addAction(@aboutQtAct)
	end
	
	def createToolBars()
	    @fileToolBar = addToolBar(tr("File"))
	    @fileToolBar.addAction(@newAct)
	    @fileToolBar.addAction(@openAct)
	    @fileToolBar.addAction(@saveAct)
	
	    @editToolBar = addToolBar(tr("Edit"))
	    @editToolBar.addAction(@cutAct)
	    @editToolBar.addAction(@copyAct)
	    @editToolBar.addAction(@pasteAct)
	end
	
	def createStatusBar()
	    statusBar().showMessage(tr("Ready"))
	end
	
	def readSettings()
	    settings = Qt5::Settings.new("Trolltech", "SDI Example")
	    pos = settings.value("pos", Qt5::Variant.new(Qt5::Point.new(200, 200))).toPoint()
	    size = settings.value("size", Qt5::Variant.new(Qt5::Size.new(400, 400))).toSize()
	    move(pos)
	    resize(size)
	end
	
	def writeSettings()
	    settings = Qt5::Settings.new("Trolltech", "SDI Example")
	    settings.setValue("pos", Qt5::Variant.new(pos()))
	    settings.setValue("size", Qt5::Variant.new(size()))
	end
	
	def maybeSave()
	    if @textEdit.document().isModified()
	        ret = Qt5::MessageBox.warning(self, tr("SDI"),
	                     tr("The document has been modified.\n" +
	                        "Do you want to save your changes?"),
	                     Qt5::MessageBox::Yes | Qt5::MessageBox::Default,
	                     Qt5::MessageBox::No,
	                     Qt5::MessageBox::Cancel | Qt5::MessageBox::Escape)
	        if ret == Qt5::MessageBox::Yes
	            return save()
	        elsif ret == Qt5::MessageBox::Cancel
	            return false
			end
	    end
	    return true
	end

	def loadFile(fileName)
	    file = Qt5::File.new(fileName)
	    if !file.open(Qt5::IODevice::ReadOnly | Qt5::IODevice::Text)
	        Qt5::MessageBox.warning(self, tr("SDI"),
	                             tr("Cannot read file %s:\n%s." % [fileName, file.errorString]))
	        return
	    end
	
	    inf = Qt5::TextStream.new(file)
	    Qt5::Application.overrideCursor = Qt5::Cursor.new(Qt5::WaitCursor)
	    @textEdit.plainText = inf.readAll
	    Qt5::Application::restoreOverrideCursor
	
	    setCurrentFile(fileName)
	    statusBar().showMessage(tr("File loaded"), 2000)
	end
	
	def saveFile(fileName)
	    file = Qt5::File.new(fileName)
	    if !file.open(Qt5::IODevice::WriteOnly | Qt5::IODevice::Text)
	        Qt5::MessageBox.warning(self, tr("SDI"),
	                             tr("Cannot write file %s:\n%s." % [fileName, file.errorString]))
	        return
	    end
	
	    outf = Qt5::TextStream.new(file)
	    Qt5::Application.overrideCursor = Qt5::Cursor.new(Qt5::WaitCursor)
	    outf << @textEdit.toPlainText
		outf.flush
	    Qt5::Application.restoreOverrideCursor
	
	    setCurrentFile(fileName)
	    statusBar().showMessage(tr("File saved"), 2000)
	end
	
	def setCurrentFile(fileName)
	    @isUntitled = fileName.empty?
	    if @isUntitled
	        @curFile = tr("document%d.txt" % (@@sequenceNumber += 1))
	    else
	        @curFile = Qt5::FileInfo.new(fileName).canonicalFilePath()
	    end
	
	    @textEdit.document().modified = false
	    setWindowModified(false)
	
	    setWindowTitle(tr("%s[*] - %s" % [strippedName(@curFile), tr("SDI")]))
	end
	
	def strippedName(fullFileName)
	    return Qt5::FileInfo.new(fullFileName).fileName()
	end
	
	def findMainWindow(fileName)
	    canonicalFilePath = Qt5::FileInfo.new(fileName).canonicalFilePath()
	
	    $qApp.topLevelWidgets.each do |widget|
	        if widget.kind_of?(MainWindow) && widget.curFile == canonicalFilePath
	            return mainWin
			end
	    end
	    return nil
	end
end
