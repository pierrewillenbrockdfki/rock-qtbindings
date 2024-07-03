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
    
require './tetrixboard.rb'
    
class TetrixWindow < Qt5::Widget
    
    def initialize(parent = nil)
        super(parent)
        @board = TetrixBoard.new(self)
    
        @nextPieceLabel = Qt5::Label.new
        @nextPieceLabel.frameStyle = Qt5::Frame::Box | Qt5::Frame::Raised
        @nextPieceLabel.alignment = Qt5::AlignCenter.to_i
        @board.nextPieceLabel = @nextPieceLabel
    
        @scoreLcd = Qt5::LCDNumber.new(5)
        @scoreLcd.segmentStyle = Qt5::LCDNumber::Filled
        @levelLcd = Qt5::LCDNumber.new(2)
        @levelLcd.segmentStyle = Qt5::LCDNumber::Filled
        @linesLcd = Qt5::LCDNumber.new(5)
        @linesLcd.segmentStyle = Qt5::LCDNumber::Filled
    
        @startButton = Qt5::PushButton.new(tr("&Start"))
        @startButton.focusPolicy = Qt5::NoFocus
        @quitButton = Qt5::PushButton.new(tr("&Quit"))
        @quitButton.focusPolicy = Qt5::NoFocus
        @pauseButton = Qt5::PushButton.new(tr("&Pause"))
        @pauseButton.focusPolicy = Qt5::NoFocus
    
        connect(@startButton, SIGNAL('clicked()'), @board, SLOT('start()'))
        connect(@quitButton , SIGNAL('clicked()'), $qApp, SLOT('quit()'))
        connect(@pauseButton, SIGNAL('clicked()'), @board, SLOT('pause()'))
        connect(@board, SIGNAL('scoreChanged(int)'), @scoreLcd, SLOT('display(int)'))
        connect(@board, SIGNAL('levelChanged(int)'), @levelLcd, SLOT('display(int)'))
        connect(@board, SIGNAL('linesRemovedChanged(int)'),
                @linesLcd, SLOT('display(int)'))
    
        layout = Qt5::GridLayout.new do |l|
            l.addWidget(createLabel(tr("NEXT")), 0, 0)
            l.addWidget(@nextPieceLabel, 1, 0)
            l.addWidget(createLabel(tr("LEVEL")), 2, 0)
            l.addWidget(@levelLcd, 3, 0)
            l.addWidget(@startButton, 4, 0)
            l.addWidget(@board, 0, 1, 6, 1)
            l.addWidget(createLabel(tr("SCORE")), 0, 2)
            l.addWidget(@scoreLcd, 1, 2)
            l.addWidget(createLabel(tr("LINES REMOVED")), 2, 2)
            l.addWidget(@linesLcd, 3, 2)
            l.addWidget(@quitButton, 4, 2)
            l.addWidget(@pauseButton, 5, 2)
        end

        setLayout(layout)
    
        setWindowTitle(tr("Tetrix"))
        resize(550, 370)
    end
    
    def createLabel(text)
        lbl = Qt5::Label.new(text)
        lbl.alignment = Qt5::AlignHCenter | Qt5::AlignBottom
        return lbl
    end
    
end
