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
    
    slots   'fontSize=(int)',
            'month=(int)',
            'year=(QDate)'
    
    def initialize(parent = nil)
        super
        @selectedDate = Qt5::Date.currentDate
        @fontSize = 10
    
        centralWidget = Qt5::Widget.new
    
        dateLabel = Qt5::Label.new(tr("Date:"))
        monthCombo = Qt5::ComboBox.new
    
        (1..12).each do |month|
            monthCombo.addItem(Qt5::Date.longMonthName(month))
        end
    
        yearEdit = Qt5::DateTimeEdit.new do |y|
        	y.displayFormat = "yyyy"
        	y.setDateRange(Qt5::Date.new(1753, 1, 1), Qt5::Date.new(8000, 1, 1))
		end
    
        monthCombo.currentIndex = @selectedDate.month - 1
        yearEdit.date = @selectedDate
    
        fontSizeLabel = Qt5::Label.new(tr("Font size:"))

        fontSizeSpinBox = Qt5::SpinBox.new do |f|
        	f.range = 1..64
        	f.value = 10
		end
    
        @editor = Qt5::TextBrowser.new
        insertCalendar()
    
        connect(monthCombo, SIGNAL('activated(int)'), self, SLOT('month=(int)'))
        connect(yearEdit, SIGNAL('dateChanged(QDate)'), self, SLOT('year=(QDate)'))
        connect(fontSizeSpinBox, SIGNAL('valueChanged(int)'),
                self, SLOT('fontSize=(int)'))
    
        controlsLayout = Qt5::HBoxLayout.new do |c|
            c.addWidget(dateLabel)
            c.addWidget(monthCombo)
            c.addWidget(yearEdit)
            c.addSpacing(24)
            c.addWidget(fontSizeLabel)
            c.addWidget(fontSizeSpinBox)
            c.addStretch(1)
        end
    
        centralWidget.layout = Qt5::VBoxLayout.new do |c|
            c.addLayout(controlsLayout)
            c.addWidget(@editor, 1)
        end
    
        self.centralWidget = centralWidget
    end
    
    def insertCalendar()
        @editor.clear
        cursor = @editor.textCursor
    
        date = Qt5::Date.new(@selectedDate.year(), @selectedDate.month(), 1)
    
        tableFormat = Qt5::TextTableFormat.new do |t|
            t.alignment = Qt5::AlignHCenter.to_i
            t.background = Qt5::Brush.new(Qt5::Color.new("#e0e0e0"))
            t.cellPadding = 2
            t.cellSpacing = 4
        end

        constraints = []
        constraints << Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) <<
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) << 
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) <<
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) <<
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) <<
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14) <<
                    Qt5::TextLength.new(Qt5::TextLength::PercentageLength, 14)
        tableFormat.columnWidthConstraints = constraints
    
        table = cursor.insertTable(1, 7, tableFormat)
    
        frame = cursor.currentFrame
        frameFormat = frame.frameFormat
        frameFormat.border = 1
        frame.frameFormat = frameFormat
    
        format = cursor.charFormat()
        format.fontPointSize = @fontSize
    
        boldFormat = Qt5::TextCharFormat.new
        boldFormat.merge(format)
        boldFormat.fontWeight = Qt5::Font::Bold
    
        highlightedFormat = Qt5::TextCharFormat.new
        highlightedFormat.merge(boldFormat)
        highlightedFormat.background = Qt5::Brush.new(Qt5::yellow)
    
        (1..7).each do |weekDay|
            cell = table.cellAt(0, weekDay-1)
            cellCursor = cell.firstCursorPosition()
            cellCursor.insertText("%s" % Qt5::Date.longDayName(weekDay),
                                  boldFormat)
        end
    
        table.insertRows(table.rows(), 1)
    
        while date.month == @selectedDate.month
            weekDay = date.dayOfWeek
            cell = table.cellAt(table.rows-1, weekDay-1)
            cellCursor = cell.firstCursorPosition
    
            if date == Qt5::Date.currentDate
                cellCursor.insertText("%s" % date.day(), highlightedFormat)
            else
                cellCursor.insertText("%s" % date.day(), format)
            end
    
            date = date.addDays(1)
            if weekDay == 7 && date.month == @selectedDate.month
                table.insertRows(table.rows, 1)
            end
        end
    
        setWindowTitle(tr("Calendar for %s %s" %
            [Qt5::Date.longMonthName(@selectedDate.month), @selectedDate.year]))
    end
    
    def fontSize=(size)
        @fontSize = size
        insertCalendar()
    end
    
    def month=(month)
        @selectedDate = Qt5::Date.new(@selectedDate.year, month + 1, @selectedDate.day)
        insertCalendar()
    end
    
    def year=(date)
        @selectedDate = Qt5::Date.new(date.year, @selectedDate.month, @selectedDate.day)
        insertCalendar()
    end
end
