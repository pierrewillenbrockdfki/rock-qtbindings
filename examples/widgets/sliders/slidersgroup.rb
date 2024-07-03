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
    
    
class SlidersGroup < Qt5::GroupBox
    
    signals 'valueChanged(int)'
    
    slots   'value=(int)',
            'setMinimum(int)',
            'setMaximum(int)',
            'invertAppearance(bool)',
            'invertKeyBindings(bool)'
    
    def initialize(orientation, title, parent = nil)
        super(title, parent)
        @slider = Qt5::Slider.new(orientation)
        @slider.focusPolicy = Qt5::StrongFocus
        @slider.tickPosition = Qt5::Slider::TicksBothSides
        @slider.tickInterval = 10
        @slider.singleStep = 1
    
        @scrollBar = Qt5::ScrollBar.new(orientation)
        @scrollBar.focusPolicy = Qt5::StrongFocus
    
        @dial = Qt5::Dial.new
        @dial.focusPolicy = Qt5::StrongFocus
    
        connect(@slider, SIGNAL('valueChanged(int)'), @scrollBar, SLOT('setValue(int)'))
        connect(@scrollBar, SIGNAL('valueChanged(int)'), @dial, SLOT('setValue(int)'))
        connect(@dial, SIGNAL('valueChanged(int)'), @slider, SLOT('setValue(int)'))
        connect(@dial, SIGNAL('valueChanged(int)'), self, SIGNAL('valueChanged(int)'))
    
        if orientation == Qt5::Horizontal
            direction = Qt5::BoxLayout::TopToBottom
        else
            direction = Qt5::BoxLayout::LeftToRight
        end

        @slidersLayout = Qt5::BoxLayout.new(direction) do |l|
			l.addWidget(@slider)
			l.addWidget(@scrollBar)
			l.addWidget(@dial)
		end
        setLayout(@slidersLayout)
    end
    
    def value=(value)
        @slider.value = value
    end
    
    def setMinimum(value)
        @slider.minimum = value
        @scrollBar.minimum = value
        @dial.minimum = value
    end
    
    def setMaximum(value)
        @slider.maximum = value
        @scrollBar.maximum = value
        @dial.maximum = value
    end
    
    def invertAppearance(invert)
        @slider.invertedAppearance = invert
        @scrollBar.invertedAppearance = invert
        @dial.invertedAppearance = invert
    end
    
    def invertKeyBindings(invert)
        @slider.invertedControls = invert
        @scrollBar.invertedControls = invert
        @dial.invertedControls = invert
    end
end
