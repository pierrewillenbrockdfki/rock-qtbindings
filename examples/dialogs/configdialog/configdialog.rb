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
    
require './pages.rb'

class ConfigDialog < Qt5::Dialog
    
    slots 'changePage(QListWidgetItem*, QListWidgetItem*)'
    
    def initialize()
        super
        @contentsWidget = Qt5::ListWidget.new do |c|
            c.viewMode = Qt5::ListView::IconMode
            c.iconSize = Qt5::Size.new(96, 84)
            c.movement = Qt5::ListView::Static
            c.maximumWidth = 128
            c.spacing = 12
        end
    
        @pagesWidget = Qt5::StackedWidget.new do |p|
            p.addWidget(ConfigurationPage.new)
            p.addWidget(UpdatePage.new)
            p.addWidget(QueryPage.new)
        end
    
        closeButton = Qt5::PushButton.new(tr("Close"))
    
        createIcons()
        @contentsWidget.currentRow = 0
    
        connect(closeButton, SIGNAL('clicked()'), self, SLOT('close()'))
    
        horizontalLayout = Qt5::HBoxLayout.new do |h|
            h.addWidget(@contentsWidget)
            h.addWidget(@pagesWidget, 1)
        end
    
        buttonsLayout = Qt5::HBoxLayout.new do |b|
            b.addStretch(1)
            b.addWidget(closeButton)
        end
    
        self.layout = Qt5::VBoxLayout.new do |m|
            m.addLayout(horizontalLayout)
            m.addStretch(1)
            m.addSpacing(12)
            m.addLayout(buttonsLayout)
        end
    
        self.windowTitle = tr("Config Dialog")
    end
    
    def createIcons
        configButton = Qt5::ListWidgetItem.new(@contentsWidget) do |c|
            c.icon = Qt5::Icon.new("images/config.png")
            c.text = tr("Configuration")
            c.textAlignment = Qt5::AlignHCenter
            c.flags = Qt5::ItemIsSelectable | Qt5::ItemIsEnabled
        end

        updateButton = Qt5::ListWidgetItem.new(@contentsWidget) do |u|
            u.icon = Qt5::Icon.new("images/update.png")
            u.text = tr("Update")
            u.textAlignment = Qt5::AlignHCenter
            u.flags = Qt5::ItemIsSelectable | Qt5::ItemIsEnabled
        end

        queryButton = Qt5::ListWidgetItem.new(@contentsWidget) do |q|
            q.icon = Qt5::Icon.new("images/query.png")
            q.text = tr("Query")
            q.textAlignment = Qt5::AlignHCenter
            q.flags = Qt5::ItemIsSelectable | Qt5::ItemIsEnabled
        end
    
        connect(@contentsWidget,
                SIGNAL('currentItemChanged(QListWidgetItem*, QListWidgetItem*)'),
                self, SLOT('changePage(QListWidgetItem*, QListWidgetItem*)'))
    end
    
    def changePage(current, previous)
        if current.nil?
            current = previous
        end

        @pagesWidget.currentIndex = @contentsWidget.row(current)
    end
end
