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
	
require './pieview.rb'

class MainWindow < Qt5::MainWindow
	
	slots 'openFile(const QString)', 'openFile()',
    		'saveFile()'
	
	def initialize()
		super
	    fileMenu = Qt5::Menu.new(tr("&File"), self)
	    openAction = fileMenu.addAction(tr("&Open"))
	    openAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+O"))
	    saveAction = fileMenu.addAction(tr("&Save"))
	    saveAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+S"))
	    quitAction = fileMenu.addAction(tr("E&xit"))
	    quitAction.shortcut = Qt5::KeySequence.new(tr("Ctrl+Q"))
	
	    setupModel()
	    setupViews()
	
	    connect(openAction, SIGNAL('triggered()'), self, SLOT('openFile()'))
	    connect(saveAction, SIGNAL('triggered()'), self, SLOT('saveFile()'))
	    connect(quitAction, SIGNAL('triggered()'), $qApp, SLOT('quit()'))
	
	    menuBar().addMenu(fileMenu)
	    statusBar()
	
	    openFile(":/Charts/qtdata.cht")
	
	    setWindowTitle(tr("Chart"))
	    resize(640, 480)
	end
	
	def setupModel()
	    @model = Qt5::StandardItemModel.new(8, 2, self)
	    @model.setHeaderData(0, Qt5::Horizontal, Qt5::Variant.new(tr("Label")))
	    @model.setHeaderData(1, Qt5::Horizontal, Qt5::Variant.new(tr("Quantity")))
	end
	
	def setupViews()
	    splitter = Qt5::Splitter.new
	    table = Qt5::TableView.new
	    @pieChart = PieView.new
	    splitter.addWidget(table)
	    splitter.addWidget(@pieChart)
	    splitter.setStretchFactor(0, 0)
	    splitter.setStretchFactor(1, 1)
	
	    table.model = @model
	    @pieChart.model = @model
	
	    @selectionModel = Qt5::ItemSelectionModel.new(@model)
	    table.selectionModel = @selectionModel
	    @pieChart.selectionModel = @selectionModel
	
	    setCentralWidget(splitter)
	end
	
	def openFile(path = nil)
	    if path.nil?
	        fileName = Qt5::FileDialog.getOpenFileName(self, tr("Choose a data file"),
	                                                "", "*.cht")
	    else
	        fileName = path
		end
	
	    if !fileName.nil?
	        file = Qt5::File.new(fileName)
	
	        if file.open(Qt5::File::ReadOnly | Qt5::File::Text)
	            stream = Qt5::TextStream.new(file)
	
	            @model.removeRows(0, @model.rowCount(Qt5::ModelIndex.new), Qt5::ModelIndex.new)
	
	            row = 0
	            line = stream.readLine()
				while !line.nil?
					@model.insertRows(row, 1, Qt5::ModelIndex.new())

					pieces = line.split(",")

					@model.setData(@model.index(row, 0, Qt5::ModelIndex.new),
									Qt5::Variant.new(pieces[0]))
					@model.setData(@model.index(row, 1, Qt5::ModelIndex.new),
									Qt5::Variant.new(pieces[1]))
					@model.setData(@model.index(row, 0, Qt5::ModelIndex.new),
									qVariantFromValue(Qt5::Color.new(pieces[2])), Qt5::DecorationRole)
	                row += 1

	                line = stream.readLine()
	            end
	
	            file.close()
	            statusBar().showMessage(tr("Loaded %s" % fileName), 2000)
	        end
	    end
	end
	
	def saveFile()
	    fileName = Qt5::FileDialog.getSaveFileName(self,
	        tr("Save file as"), "", "*.cht")
	
	    if !fileName.nil?
	        file = Qt5::File.new(fileName)
	        stream = Qt5::TextStream.new(file)
	
	        if file.open(Qt5::File::WriteOnly | Qt5::File::Text)
				(0...@model.rowCount(Qt5::ModelIndex.new)).each do |row|
	                pieces = []
	
	                pieces.push(@model.data(@model.index(row, 0, Qt5::ModelIndex.new),
	                                          Qt5::DisplayRole).toString)
	                pieces.push(@model.data(@model.index(row, 1, Qt5::ModelIndex.new),
	                                          Qt5::DisplayRole).toString)
	                pieces.push(@model.data(@model.index(row, 0, Qt5::ModelIndex.new),
	                                          Qt5::DecorationRole).toString)
	
	                stream << pieces.join(",") << "\n"
	            end
	        end
	
	        file.close()
	        statusBar().showMessage(tr("Saved %s" % fileName), 2000)
	    end
	end
end
