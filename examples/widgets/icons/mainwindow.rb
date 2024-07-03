=begin
**
** Copyright (C) 2004-2005 Trolltech AS. All rights reserved.
**
** This file is part of the example classes of the Qt Toolkit.
**
** This file may be used under the terms of the GNU General Public
** License version 2.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of
** self file.  Please review the following information to ensure GNU
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

require './iconpreviewarea.rb'
require './iconsizespinbox.rb'
require './imagedelegate.rb'

class MainWindow < Qt5::MainWindow

    slots   'about()',
            'changeStyle(bool)',
            'changeSize()',
            'changeIcon()',
            'addImage()',
            'removeAllImages()'

    def initialize()
		super
        @centralWidget = Qt5::Widget.new
        setCentralWidget(@centralWidget)
    
        createPreviewGroupBox()
        createImagesGroupBox()
        createIconSizeGroupBox()
    
        createActions()
        createMenus()
        createContextMenu()
    
        mainLayout = Qt5::GridLayout.new
        mainLayout.addWidget(@imagesGroupBox, 0, 0)
        mainLayout.addWidget(@iconSizeGroupBox, 1, 0)
        mainLayout.addWidget(@previewGroupBox, 0, 1, 2, 1)
        @centralWidget.layout = mainLayout
    
        setWindowTitle(tr("Icons"))
        checkCurrentStyle()
        @otherRadioButton.click()
        resize(860, 400)
    end
    
    def about()
        Qt5::MessageBox.about(self, tr("About Icons"),
                tr("The <b>Icons</b> example illustrates how Qt renders an icon in " +
                "different modes (active, normal, and disabled) and states (on " +
                "and off) based on a set of images."))
    end
    
    def changeStyle(checked)
        if !checked
            return
        end
    
        action = sender()
        style = Qt5::StyleFactory.create(action.data().toString())
        Qt5::Application.style = style
    
        @smallRadioButton.text = tr("Small (%d x %d" % 
                [    style.pixelMetric(Qt5::Style::PM_SmallIconSize),
                    style.pixelMetric(Qt5::Style::PM_SmallIconSize) ] )
        @largeRadioButton.text = tr("Large (%d x %d" % 
                [    style.pixelMetric(Qt5::Style::PM_LargeIconSize),
                    style.pixelMetric(Qt5::Style::PM_LargeIconSize) ] )
        @toolBarRadioButton.text = tr("Toolbars (%d x %d" % 
                [    style.pixelMetric(Qt5::Style::PM_ToolBarIconSize),
                    style.pixelMetric(Qt5::Style::PM_ToolBarIconSize) ] )
        @listViewRadioButton.text = tr("List views (%d x %d" % 
                [    style.pixelMetric(Qt5::Style::PM_ListViewIconSize),
                    style.pixelMetric(Qt5::Style::PM_ListViewIconSize) ] )
        @iconViewRadioButton.text = tr("Icon views (%d x %d" % 
                [    style.pixelMetric(Qt5::Style::PM_IconViewIconSize),
                    style.pixelMetric(Qt5::Style::PM_IconViewIconSize) ] )
    
        changeSize()
    end
    
    def changeSize()
        if @otherRadioButton.checked?
            extent = @otherSpinBox.value
        else
            if @smallRadioButton.checked?
                metric = Qt5::Style::PM_SmallIconSize
            elsif @largeRadioButton.checked?
                metric = Qt5::Style::PM_LargeIconSize
            elsif @toolBarRadioButton.checked?
                metric = Qt5::Style::PM_ToolBarIconSize
            elsif @listViewRadioButton.checked?
                metric = Qt5::Style::PM_ListViewIconSize
            else
                metric = Qt5::Style::PM_IconViewIconSize
            end
            extent = Qt5::Application::style().pixelMetric(metric)
        end

        @previewArea.size = Qt5::Size.new(extent, extent)
        @otherSpinBox.enabled = @otherRadioButton.checked?
    end
    
    def changeIcon()
		icon = Qt5::Icon.new
        (0...@imagesTable.rowCount).each do |row|
            item0 = @imagesTable.item(row, 0)
            item1 = @imagesTable.item(row, 1)
            item2 = @imagesTable.item(row, 2)
    
            if item0.checkState() == Qt5::Checked
                if item1.text() == tr("Normal")
                    mode = Qt5::Icon::Normal
                elsif item1.text() == tr("Active")
                    mode = Qt5::Icon::Active
                else
                    mode = Qt5::Icon::Disabled
                end
    
                if item2.text() == tr("On")
                    state = Qt5::Icon::On
                else
                    state = Qt5::Icon::Off
                end
    
                fileName = item0.data(Qt5::UserRole).toString()
                image = Qt5::Image.new(fileName)
                if !image.nil?
                    icon.addPixmap(Qt5::Pixmap.fromImage(image), mode, state)
                end
            end
        end
        @previewArea.icon = icon
    end
    
    def addImage()
        fileNames = Qt5::FileDialog.getOpenFileNames(self,
                                        tr("Open Images"), "",
                                        tr("Images (*.png *.xpm *.jpg);;" +
                                        "All Files (*)") )
        if !fileNames.nil?
            fileNames.each do |fileName|
                row = @imagesTable.rowCount()
                @imagesTable.rowCount = row + 1
    
                imageName = Qt5::FileInfo.new(fileName).baseName()
                item0 = Qt5::TableWidgetItem.new(imageName)
                item0.setData(Qt5::UserRole, Qt5::Variant.new(fileName))
                item0.flags &= ~Qt5::ItemIsEditable
    
                item1 = Qt5::TableWidgetItem.new(tr("Normal"))
                item2 = Qt5::TableWidgetItem.new(tr("Off"))
    
                if @guessModeStateAct.checked?
                    if fileName.include?("_act")
                        item1.text = tr("Active")
                    elsif fileName.include?("_dis")
                        item1.text = tr("Disabled")
                    end
    
                    if fileName.include?("_on")
                        item2.text = tr("On")
                    end
                end
    
                @imagesTable.setItem(row, 0, item0)
                @imagesTable.setItem(row, 1, item1)
                @imagesTable.setItem(row, 2, item2)
                @imagesTable.openPersistentEditor(item1)
                @imagesTable.openPersistentEditor(item2)
    
                item0.checkState = Qt5::Checked
            end
        end
    end
    
    def removeAllImages()
        @imagesTable.rowCount = 0
        changeIcon()
    end
    
    def createPreviewGroupBox()
        @previewGroupBox = Qt5::GroupBox.new(tr("Preview"))
    
        @previewArea = IconPreviewArea.new
    
        layout = Qt5::VBoxLayout.new
        layout.addWidget(@previewArea)
        @previewGroupBox.layout = layout
    end
    
    def createImagesGroupBox()
        @imagesGroupBox = Qt5::GroupBox.new(tr("Images"))
        @imagesGroupBox.setSizePolicy(Qt5::SizePolicy::Expanding,
                                    Qt5::SizePolicy::Expanding)
    
        labels = []
        labels << tr("Image") << tr("Mode") << tr("State")
    
        @imagesTable = Qt5::TableWidget.new
        @imagesTable.setSizePolicy(Qt5::SizePolicy::Expanding, Qt5::SizePolicy::Ignored)
        @imagesTable.selectionMode = Qt5::AbstractItemView::NoSelection
        @imagesTable.columnCount = 3
        @imagesTable.horizontalHeaderLabels = labels
        @imagesTable.itemDelegate = ImageDelegate.new(self)
    
        @imagesTable.horizontalHeader().resizeSection(0, 160)
        @imagesTable.horizontalHeader().resizeSection(1, 80)
        @imagesTable.horizontalHeader().resizeSection(2, 80)
        @imagesTable.verticalHeader().hide()
    
        connect(@imagesTable, SIGNAL('itemChanged(QTableWidgetItem*)'),
                self, SLOT('changeIcon()'))
    
        layout = Qt5::VBoxLayout.new
        layout.addWidget(@imagesTable)
        @imagesGroupBox.layout = layout
    end
    
    def createIconSizeGroupBox()
        @iconSizeGroupBox = Qt5::GroupBox.new(tr("Icon Size"))
    
        @smallRadioButton = Qt5::RadioButton.new
        @largeRadioButton = Qt5::RadioButton.new
        @toolBarRadioButton = Qt5::RadioButton.new
        @listViewRadioButton = Qt5::RadioButton.new
        @iconViewRadioButton = Qt5::RadioButton.new
        @otherRadioButton = Qt5::RadioButton.new(tr("Other:"))
    
        @otherSpinBox = IconSizeSpinBox.new
        @otherSpinBox.range = 8..128
        @otherSpinBox.value = 64
    
        connect(@toolBarRadioButton, SIGNAL('toggled(bool)'),
                self, SLOT('changeSize()'))
        connect(@listViewRadioButton, SIGNAL('toggled(bool)'),
                self, SLOT('changeSize()'))
        connect(@iconViewRadioButton, SIGNAL('toggled(bool)'),
                self, SLOT('changeSize()'))
        connect(@smallRadioButton, SIGNAL('toggled(bool)'), self, SLOT('changeSize()'))
        connect(@largeRadioButton, SIGNAL('toggled(bool)'), self, SLOT('changeSize()'))
        connect(@otherRadioButton, SIGNAL('toggled(bool)'), self, SLOT('changeSize()'))
        connect(@otherSpinBox, SIGNAL('valueChanged(int)'), self, SLOT('changeSize()'))
    
        otherSizeLayout = Qt5::HBoxLayout.new
        otherSizeLayout.addWidget(@otherRadioButton)
        otherSizeLayout.addWidget(@otherSpinBox)
    
        layout = Qt5::GridLayout.new
        layout.addWidget(@smallRadioButton, 0, 0)
        layout.addWidget(@largeRadioButton, 1, 0)
        layout.addWidget(@toolBarRadioButton, 2, 0)
        layout.addWidget(@listViewRadioButton, 0, 1)
        layout.addWidget(@iconViewRadioButton, 1, 1)
        layout.addLayout(otherSizeLayout, 2, 1)
        @iconSizeGroupBox.layout = layout
    end
    
    def createActions()
        @addImageAct = Qt5::Action.new(tr("&Add Image..."), self)
        @addImageAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+A"))
        connect(@addImageAct, SIGNAL('triggered()'), self, SLOT('addImage()'))
    
        @removeAllImagesAct = Qt5::Action.new(tr("&Remove All Images"), self)
        @removeAllImagesAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+R"))
        connect(@removeAllImagesAct, SIGNAL('triggered()'),
                self, SLOT('removeAllImages()'))
    
        @exitAct = Qt5::Action.new(tr("&Quit"), self)
        @exitAct.shortcut = Qt5::KeySequence.new(tr("Ctrl+Q"))
        connect(@exitAct, SIGNAL('triggered()'), self, SLOT('close()'))
    
        @styleActionGroup = Qt5::ActionGroup.new(self)
        
        Qt5::StyleFactory::keys().each do |styleName|
            action = Qt5::Action.new(@styleActionGroup)
            action.text = tr("%s Style" % styleName)
            action.data = Qt5::Variant.new(styleName)
            action.checkable = true
            connect(action, SIGNAL('triggered(bool)'), self, SLOT('changeStyle(bool)'))
        end
    
        @guessModeStateAct = Qt5::Action.new(tr("&Guess Image Mode/State"), self)
        @guessModeStateAct.checkable = true
        @guessModeStateAct.checked = true
    
        @aboutAct = Qt5::Action.new(tr("&About"), self)
        connect(@aboutAct, SIGNAL('triggered()'), self, SLOT('about()'))
    
        @aboutQtAct = Qt5::Action.new(tr("About &Qt"), self)
        connect(@aboutQtAct, SIGNAL('triggered()'), $qApp, SLOT('aboutQt()'))
    end
    
    def createMenus()
        @fileMenu = menuBar().addMenu(tr("&File"))
        @fileMenu.addAction(@addImageAct)
        @fileMenu.addAction(@removeAllImagesAct)
        @fileMenu.addSeparator()
        @fileMenu.addAction(@exitAct)
    
        @viewMenu = menuBar().addMenu(tr("&View"))

        @styleActionGroup.actions().each do |action|
            @viewMenu.addAction(action)
        end
        @viewMenu.addSeparator()
        @viewMenu.addAction(@guessModeStateAct)
    
        menuBar().addSeparator()
    
        @helpMenu = menuBar().addMenu(tr("&Help"))
        @helpMenu.addAction(@aboutAct)
        @helpMenu.addAction(@aboutQtAct)
    end
    
    def createContextMenu()
        @imagesTable.contextMenuPolicy = Qt5::ActionsContextMenu
        @imagesTable.addAction(@addImageAct)
        @imagesTable.addAction(@removeAllImagesAct)
    end
    
    def checkCurrentStyle()
        @styleActionGroup.actions().each do |action|
            styleName = action.data().toString()
            candidate = Qt5::StyleFactory.create(styleName)

            if candidate.metaObject().className() ==
               Qt5::Application.style().metaObject().className()
                action.trigger()
                return
            end
        end
    end
end
