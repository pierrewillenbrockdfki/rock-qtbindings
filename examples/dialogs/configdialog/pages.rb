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

class ConfigurationPage < Qt5::Widget
    
    def initialize(parent = nil)
        super(parent)
        configGroup = Qt5::GroupBox.new(tr("Server configuration"))
    
        serverLabel = Qt5::Label.new(tr("Server:"))
        serverCombo = Qt5::ComboBox.new do |c|
            c.addItem(tr("Trolltech (Australia)"))
            c.addItem(tr("Trolltech (Norway)"))
            c.addItem(tr("Trolltech (People's Republic of China)"))
            c.addItem(tr("Trolltech (USA)"))
        end
    
        serverLayout = Qt5::HBoxLayout.new do |s|
            s.addWidget(serverLabel)
            s.addWidget(serverCombo)
        end
    
        configGroup.layout = Qt5::VBoxLayout.new do |c|
            c.addLayout(serverLayout)
        end
    
        self.layout = Qt5::VBoxLayout.new do |m|
            m.addWidget(configGroup)
            m.addStretch(1)
        end
    end
end

class QueryPage < Qt5::Widget
    
    def initialize(parent = nil)
        super(parent)
        updateGroup = Qt5::GroupBox.new(tr("Package selection"))
        systemCheckBox = Qt5::CheckBox.new(tr("Update system"))
        appsCheckBox = Qt5::CheckBox.new(tr("Update applications"))
        docsCheckBox = Qt5::CheckBox.new(tr("Update documentation"))
    
        packageGroup = Qt5::GroupBox.new(tr("Existing packages"))
    
        packageList = Qt5::ListWidget.new
        qtItem = Qt5::ListWidgetItem.new(packageList)
        qtItem.text = tr("Qt5")
        qsaItem = Qt5::ListWidgetItem.new(packageList)
        qsaItem.text = tr("QSA")
        teamBuilderItem = Qt5::ListWidgetItem.new(packageList)
        teamBuilderItem.text = tr("Teambuilder")
    
        startUpdateButton = Qt5::PushButton.new(tr("Start update"))
    
        updateGroup.layout = Qt5::VBoxLayout.new do |u|
            u.addWidget(systemCheckBox)
            u.addWidget(appsCheckBox)
            u.addWidget(docsCheckBox)
        end
    
        packageGroup.layout = Qt5::VBoxLayout.new do |p|
            p.addWidget(packageList)
        end
    
        self.layout = Qt5::VBoxLayout.new do |m|
            m.addWidget(updateGroup)
            m.addWidget(packageGroup)
            m.addSpacing(12)
            m.addWidget(startUpdateButton)
            m.addStretch(1)
        end
    end
end

class UpdatePage < Qt5::Widget
    
    def initialize(parent = nil)
        super(parent)
        packagesGroup = Qt5::GroupBox.new(tr("Look for packages"))
    
        nameLabel = Qt5::Label.new(tr("Name:"))
        nameEdit = Qt5::LineEdit.new
    
        dateLabel = Qt5::Label.new(tr("Released after:"))
        dateEdit = Qt5::DateTimeEdit.new(Qt5::Date.currentDate())
    
        releasesCheckBox = Qt5::CheckBox.new(tr("Releases"))
        upgradesCheckBox = Qt5::CheckBox.new(tr("Upgrades"))
    
        hitsSpinBox = Qt5::SpinBox.new do |h|
            h.prefix = tr("Return up to ")
            h.suffix = tr(" results")
            h.specialValueText = tr("Return only the first result")
            h.minimum = 1
            h.maximum = 100
            h.singleStep = 10
        end

        startQueryButton = Qt5::PushButton.new(tr("Start query"))
    
        packagesGroup.layout = Qt5::GridLayout.new do |p|
            p.addWidget(nameLabel, 0, 0)
            p.addWidget(nameEdit, 0, 1)
            p.addWidget(dateLabel, 1, 0)
            p.addWidget(dateEdit, 1, 1)
            p.addWidget(releasesCheckBox, 2, 0)
            p.addWidget(upgradesCheckBox, 3, 0)
            p.addWidget(hitsSpinBox, 4, 0, 1, 2)
        end
    
        self.layout = Qt5::VBoxLayout.new do |m|
            m.addWidget(packagesGroup)
            m.addSpacing(12)
            m.addWidget(startQueryButton)
            m.addStretch(1)
        end
    end
end
