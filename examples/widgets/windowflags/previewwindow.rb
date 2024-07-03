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
    
    
class PreviewWindow < Qt5::Widget
    
    def initialize(parent = nil)
        super(parent)
        @textEdit = Qt5::TextEdit.new
        @textEdit.readOnly = true
        @textEdit.lineWrapMode = Qt5::TextEdit::NoWrap
    
        @closeButton = Qt5::PushButton.new(tr("&Close"))
        connect(@closeButton, SIGNAL('clicked()'), self, SLOT('close()'))
    
        layout = Qt5::VBoxLayout.new do |l|
            l.addWidget(@textEdit)
            l.addWidget(@closeButton)
        end

        setLayout(layout)
    
        setWindowTitle(tr("Preview"))
    end
    
    def setWindowFlags(flags)
        super(flags.to_i)
    
        type = (flags & Qt5::WindowType_Mask.to_i)
        if type == Qt5::WindowType
            text = "Qt5::Window"
        elsif type == Qt5::DialogType
            text = "Qt5::Dialog"
        elsif type == Qt5::SheetType
            text = "Qt5::Sheet"
        elsif type == Qt5::DrawerType
            text = "Qt5::Drawer"
        elsif type == Qt5::PopupType
            text = "Qt5::Popup"
        elsif type == Qt5::ToolType
            text = "Qt5::Tool"
        elsif type == Qt5::ToolTipType
            text = "Qt5::ToolTip"
        elsif type == Qt5::SplashScreenType
            text = "Qt5::SplashScreen"
        end
    
        if (flags & Qt5::MSWindowsFixedSizeDialogHint.to_i) != 0
            text += "\n| Qt5::MSWindowsFixedSizeDialogHint"
        end
        if (flags & Qt5::X11BypassWindowManagerHint.to_i) != 0
            text += "\n| Qt5::X11BypassWindowManagerHint"
        end
        if (flags & Qt5::FramelessWindowHint.to_i) != 0
            text += "\n| Qt5::FramelessWindowHint"
        end
        if (flags & Qt5::WindowTitleHint.to_i) != 0
            text += "\n| Qt5::WindowTitleHint"
        end
        if (flags & Qt5::WindowSystemMenuHint.to_i) != 0
            text += "\n| Qt5::WindowSystemMenuHint"
        end
        if (flags & Qt5::WindowMinimizeButtonHint.to_i) != 0
            text += "\n| Qt5::WindowMinimizeButtonHint"
        end
        if (flags & Qt5::WindowMaximizeButtonHint.to_i) != 0
            text += "\n| Qt5::WindowMaximizeButtonHint"
        end
        if (flags & Qt5::WindowContextHelpButtonHint.to_i) != 0
            text += "\n| Qt5::WindowContextHelpButtonHint"
        end
        if (flags & Qt5::WindowShadeButtonHint.to_i) != 0
            text += "\n| Qt5::WindowShadeButtonHint"
        end
        if (flags & Qt5::WindowStaysOnTopHint.to_i) != 0
            text += "\n| Qt5::WindowStaysOnTopHint"
        end
    
        @textEdit.plainText = text
    end
end
