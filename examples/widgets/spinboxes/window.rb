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
    
    
class Window < Qt5::Widget
        
    slots 'changePrecision(int)',
          'setFormatString(const QString&)'
    
    def initialize(parent = nil)
        super(parent)
        createSpinBoxes()
        createDateTimeEdits()
        createDoubleSpinBoxes()
    
        layout = Qt5::HBoxLayout.new do |l|
            l.addWidget(@spinBoxesGroup)
            l.addWidget(@editsGroup)
            l.addWidget(@doubleSpinBoxesGroup)
        end

        setLayout(layout)
    
        setWindowTitle(tr("Spin Boxes"))
    end
    
    def createSpinBoxes()
        @spinBoxesGroup = Qt5::GroupBox.new(tr("Spinboxes"))
    
        integerLabel = Qt5::Label.new(tr("Enter a value between %d and %d:" % [-20, 20]))

        integerSpinBox = Qt5::SpinBox.new do |i|
            i.range = -20..20
            i.singleStep = 1
            i.value = 0
        end
    
        zoomLabel = Qt5::Label.new(tr("Enter a zoom value between %d and %d:" % [0, 1000]))

        zoomSpinBox = Qt5::SpinBox.new do |z|
            z.range = 0..1000
            z.singleStep = 10
            z.suffix = "%"
            z.specialValueText = tr("Automatic")
            z.value = 100
        end
    
        priceLabel = Qt5::Label.new(tr("Enter a price between %d and %d:" % [0, 999]))

        @priceSpinBox = Qt5::SpinBox.new do |s|
            s.range = 0..999
            s.singleStep = 1
            s.prefix = "$"
            s.value = 99
        end
    
        spinBoxLayout = Qt5::VBoxLayout.new do |s|
            s.addWidget(integerLabel)
            s.addWidget(integerSpinBox)
            s.addWidget(zoomLabel)
            s.addWidget(zoomSpinBox)
            s.addWidget(priceLabel)
            s.addWidget(@priceSpinBox)
        end

        @spinBoxesGroup.layout = spinBoxLayout
    end
    
    def createDateTimeEdits()
        @editsGroup = Qt5::GroupBox.new(tr("Date and time spin boxes"))
    
        dateLabel = Qt5::Label.new
        dateEdit = Qt5::DateTimeEdit.new(Qt5::Date.currentDate())
        dateEdit.setDateRange(Qt5::Date.new(2005, 1, 1), Qt5::Date.new(2010, 12, 31))
        dateLabel.text = tr("Appointment date (between %s and %s:" %
                           [dateEdit.minimumDate().toString(Qt5::ISODate),
                            dateEdit.maximumDate().toString(Qt5::ISODate) ] )
    
        timeLabel = Qt5::Label.new
        timeEdit = Qt5::DateTimeEdit.new(Qt5::Time.currentTime())
        timeEdit.setTimeRange(Qt5::Time.new(9, 0, 0, 0), Qt5::Time.new(16, 30, 0, 0))
        timeLabel.text = tr("Appointment time (between %s and %s:" %
                           [timeEdit.minimumTime().toString(Qt5::ISODate),
                            timeEdit.maximumTime().toString(Qt5::ISODate) ] )
    
        @meetingLabel = Qt5::Label.new
        @meetingEdit = Qt5::DateTimeEdit.new(Qt5::DateTime.currentDateTime())
    
        formatLabel = Qt5::Label.new(tr("Format string for the meeting date and time:"))
        formatComboBox = Qt5::ComboBox.new do |f|
            f.addItem("yyyy-MM-dd hh:mm:ss (zzz ms)")
            f.addItem("hh:mm:ss MM/dd/yyyy")
            f.addItem("hh:mm:ss dd/MM/yyyy")
            f.addItem("hh:mm:ss")
            f.addItem("hh:mm ap")
        end
    
        connect(formatComboBox, SIGNAL('activated(const QString&)'),
                self, SLOT('setFormatString(const QString&)'))
    
        setFormatString(formatComboBox.currentText())
    
        editsLayout = Qt5::VBoxLayout.new do |l|
            l.addWidget(dateLabel)
            l.addWidget(dateEdit)
            l.addWidget(timeLabel)
            l.addWidget(timeEdit)
            l.addWidget(@meetingLabel)
            l.addWidget(@meetingEdit)
            l.addWidget(formatLabel)
            l.addWidget(formatComboBox)
        end

        @editsGroup.layout = editsLayout
    end
    
    def setFormatString(formatString)
        @meetingEdit.displayFormat = formatString
        if @meetingEdit.displayedSections() & Qt5::DateTimeEdit::DateSections_Mask.to_i
            @meetingEdit.setDateRange(Qt5::Date.new(2004, 11, 1), Qt5::Date.new(2005, 11, 30))
            @meetingLabel.text = tr("Meeting date (between %s and %s:" %
                [@meetingEdit.minimumDate().toString(Qt5::ISODate),
                 @meetingEdit.maximumDate().toString(Qt5::ISODate) ] )
        else
            @meetingEdit.setTimeRange(Qt5::Time.new(0, 7, 20, 0), Qt5::Time.new(21, 0, 0, 0))
            @meetingLabel.text = tr("Meeting time (between %s and %s:" %
                [@meetingEdit.minimumTime().toString(Qt5::ISODate),
                 @meetingEdit.maximumTime().toString(Qt5::ISODate) ] )
        end
    end
    
    def createDoubleSpinBoxes()
        @doubleSpinBoxesGroup = Qt5::GroupBox.new(tr("Double precision spinboxes"))
    
        precisionLabel = Qt5::Label.new(tr("Number of decimal places to show:"))

        precisionSpinBox = Qt5::SpinBox.new do |s|
            s.range = 0..14
            s.value = 2
        end
    
        doubleLabel = Qt5::Label.new(tr("Enter a value between %d and %d:" % [-20, 20]))

        @doubleSpinBox = Qt5::DoubleSpinBox.new do |s|
            s.range = -20.0..20.0
            s.singleStep = 1.0
            s.value = 0.0
        end
    
        scaleLabel = Qt5::Label.new(tr("Enter a scale factor between %2f and %2f:" % [0.0, 1000.0]))

        @scaleSpinBox = Qt5::DoubleSpinBox.new do |s|
            s.range = 0.0..1000.0
            s.singleStep = 10.0
            s.suffix = "%"
            s.specialValueText = tr("No scaling")
            s.value = 100.0
        end
    
        priceLabel = Qt5::Label.new(tr("Enter a price between %2f and %2f:" % [0.0, 1000.0]))

        @priceSpinBox = Qt5::DoubleSpinBox.new do |s|
            s.range = 0.0..1000.0
            s.singleStep = 1.0
            s.prefix = "$"
            s.value = 99.99
        end
    
        connect(precisionSpinBox, SIGNAL('valueChanged(int)'),
                self, SLOT('changePrecision(int)'))
    
        spinBoxLayout = Qt5::VBoxLayout.new do |s|
            s.addWidget(precisionLabel)
            s.addWidget(precisionSpinBox)
            s.addWidget(doubleLabel)
            s.addWidget(@doubleSpinBox)
            s.addWidget(scaleLabel)
            s.addWidget(@scaleSpinBox)
            s.addWidget(priceLabel)
            s.addWidget(@priceSpinBox)
        end

        @doubleSpinBoxesGroup.layout = spinBoxLayout
    end
    
    def changePrecision(decimals)
        @doubleSpinBox.decimals = decimals
        @scaleSpinBox.decimals = decimals
        @priceSpinBox.decimals = decimals
    end
end
