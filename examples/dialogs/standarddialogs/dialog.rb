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
    
    
class Dialog < Qt5::Dialog
        
    slots   'setInt()',
            'setDouble()',
            'setItem()',
            'setText()',
            'setColor()',
            'setFont()',
            'setExistingDirectory()',
            'setOpenFileName()',
            'setOpenFileNames()',
            'setSaveFileName()',
            'criticalMessage()',
            'informationMessage()',
            'questionMessage()',
            'warningMessage()',
            'errorMessage()'
    
    def initialize(parent = nil)
        super(parent)

        @message = tr("<p>Message boxes have a caption, a text, " +
                   "and up to three buttons, each with standard or custom texts." +
                   "<p>Click a button or press Esc.")

        @errorMessageDialog = Qt5::ErrorMessage.new(self)
    
        frameStyle = Qt5::Frame::Sunken | Qt5::Frame::Panel
    
        @integerLabel = Qt5::Label.new
        @integerLabel.frameStyle = frameStyle
        intButton = Qt5::PushButton.new(tr("Qt5::InputDialog.get&Int()"))
    
        @doubleLabel = Qt5::Label.new
        @doubleLabel.frameStyle = frameStyle
        doubleButton =
                Qt5::PushButton.new(tr("Qt5::InputDialog.get&Double()"))
    
        @itemLabel = Qt5::Label.new
        @itemLabel.frameStyle = frameStyle
        itemButton = Qt5::PushButton.new(tr("Qt5::InputDialog.getIte&m()"))
    
        @textLabel = Qt5::Label.new
        @textLabel.frameStyle = frameStyle
        textButton = Qt5::PushButton.new(tr("Qt5::InputDialog.get&Text()"))
    
        @colorLabel = Qt5::Label.new
        @colorLabel.frameStyle = frameStyle
        colorButton = Qt5::PushButton.new(tr("Qt5::ColorDialog.get&Color()"))
    
        @fontLabel = Qt5::Label.new
        @fontLabel.frameStyle = frameStyle
        fontButton = Qt5::PushButton.new(tr("Qt5::tFontDialog.get&Font()"))
    
        @directoryLabel = Qt5::Label.new
        @directoryLabel.frameStyle = frameStyle
        directoryButton =
                Qt5::PushButton.new(tr("Qt5::FileDialog.getE&xistingDirectory()"))
    
        @openFileNameLabel = Qt5::Label.new
        @openFileNameLabel.frameStyle = frameStyle
        openFileNameButton =
                Qt5::PushButton.new(tr("Qt5::FileDialog.get&OpenFileName()"))
    
        @openFileNamesLabel = Qt5::Label.new
        @openFileNamesLabel.frameStyle = frameStyle
        openFileNamesButton =
                Qt5::PushButton.new(tr("Qt5::File&Dialog.getOpenFileNames()"))
    
        @saveFileNameLabel = Qt5::Label.new
        @saveFileNameLabel.frameStyle = frameStyle
        saveFileNameButton =
                Qt5::PushButton.new(tr("Qt5::FileDialog.get&SaveFileName()"))
    
        @criticalLabel = Qt5::Label.new
        @criticalLabel.frameStyle = frameStyle
        criticalButton =
                Qt5::PushButton.new(tr("Qt5::MessageBox.critica&l()"))
    
        @informationLabel = Qt5::Label.new
        @informationLabel.frameStyle = frameStyle
        informationButton =
                Qt5::PushButton.new(tr("Qt5::MessageBox.i&nformation()"))
    
        @questionLabel = Qt5::Label.new
        @questionLabel.frameStyle = frameStyle
        questionButton =
                Qt5::PushButton.new(tr("Qt5::MessageBox.&question()"))
    
        @warningLabel = Qt5::Label.new
        @warningLabel.frameStyle = frameStyle
        warningButton = Qt5::PushButton.new(tr("Qt5::MessageBox.&warning()"))
    
        @errorLabel = Qt5::Label.new
        @errorLabel.frameStyle = frameStyle
        errorButton =
                Qt5::PushButton.new(tr("Qt5::ErrorMessage.show&M&essage()"))
    
        connect(intButton, SIGNAL('clicked()'), self, SLOT('setInt()'))
        connect(doubleButton, SIGNAL('clicked()'), self, SLOT('setDouble()'))
        connect(itemButton, SIGNAL('clicked()'), self, SLOT('setItem()'))
        connect(textButton, SIGNAL('clicked()'), self, SLOT('setText()'))
        connect(colorButton, SIGNAL('clicked()'), self, SLOT('setColor()'))
        connect(fontButton, SIGNAL('clicked()'), self, SLOT('setFont()'))
        connect(directoryButton, SIGNAL('clicked()'),
                self, SLOT('setExistingDirectory()'))
        connect(openFileNameButton, SIGNAL('clicked()'),
                self, SLOT('setOpenFileName()'))
        connect(openFileNamesButton, SIGNAL('clicked()'),
                self, SLOT('setOpenFileNames()'))
        connect(saveFileNameButton, SIGNAL('clicked()'),
                self, SLOT('setSaveFileName()'))
        connect(criticalButton, SIGNAL('clicked()'), self, SLOT('criticalMessage()'))
        connect(informationButton, SIGNAL('clicked()'),
                self, SLOT('informationMessage()'))
        connect(questionButton, SIGNAL('clicked()'), self, SLOT('questionMessage()'))
        connect(warningButton, SIGNAL('clicked()'), self, SLOT('warningMessage()'))
        connect(errorButton, SIGNAL('clicked()'), self, SLOT('errorMessage()'))
    
        self.layout = Qt5::GridLayout.new do |l|
            l.setColumnStretch(1, 1)
            l.setColumnMinimumWidth(1, 250)
            l.addWidget(intButton, 0, 0)
            l.addWidget(@integerLabel, 0, 1)
            l.addWidget(doubleButton, 1, 0)
            l.addWidget(@doubleLabel, 1, 1)
            l.addWidget(itemButton, 2, 0)
            l.addWidget(@itemLabel, 2, 1)
            l.addWidget(textButton, 3, 0)
            l.addWidget(@textLabel, 3, 1)
            l.addWidget(colorButton, 4, 0)
            l.addWidget(@colorLabel, 4, 1)
            l.addWidget(fontButton, 5, 0)
            l.addWidget(@fontLabel, 5, 1)
            l.addWidget(directoryButton, 6, 0)
            l.addWidget(@directoryLabel, 6, 1)
            l.addWidget(openFileNameButton, 7, 0)
            l.addWidget(@openFileNameLabel, 7, 1)
            l.addWidget(openFileNamesButton, 8, 0)
            l.addWidget(@openFileNamesLabel, 8, 1)
            l.addWidget(saveFileNameButton, 9, 0)
            l.addWidget(@saveFileNameLabel, 9, 1)
            l.addWidget(criticalButton, 10, 0)
            l.addWidget(@criticalLabel, 10, 1)
            l.addWidget(informationButton, 11, 0)
            l.addWidget(@informationLabel, 11, 1)
            l.addWidget(questionButton, 12, 0)
            l.addWidget(@questionLabel, 12, 1)
            l.addWidget(warningButton, 13, 0)
            l.addWidget(@warningLabel, 13, 1)
            l.addWidget(errorButton, 14, 0)
            l.addWidget(@errorLabel, 14, 1)
        end
    
        self.windowTitle = tr("Standard Dialogs")
    end
    
    def setInt()
        ok = Qt5::Boolean.new
        i = Qt5::InputDialog.getInt(self, tr("Qt5::InputDialog.getInt()"),
                                    tr("Percentage:"), 25, 0, 100, 1, ok)
        if ok
            @integerLabel.text = tr("%d%%" % i)
        end
    end
    
    def setDouble()
        ok = Qt5::Boolean.new
        d = Qt5::InputDialog.getDouble(self, tr("Qt5::InputDialog.getDouble()"),
                                           tr("Amount:"), 37.56, -10000, 10000, 2, ok)
        if ok
            @doubleLabel.text = "$%f" % d
        end
    end
    
    def setItem()
        items = []
        items << tr("Spring") << tr("Summer") << tr("Fall") << tr("Winter")
    
        ok = Qt5::Boolean.new
        item = Qt5::InputDialog.getItem(self, tr("Qt5::InputDialog.getItem()"),
                                             tr("Season:"), items, 0, false, ok)
        if ok && !item.nil?
            @itemLabel.text = item
        end
    end
    
    def setText()
        ok = Qt5::Boolean.new
        text = Qt5::InputDialog.getText(self, tr("Qt5::InputDialog.getText()"),
                                             tr("User name:"), Qt5::LineEdit::Normal,
                                             Qt5::Dir::home().dirName(), ok)
        if ok && !text.nil?
            @textLabel.text = text
        end
    end
    
    def setColor()
        color = Qt5::ColorDialog.getColor(Qt5::Color.new(Qt5::green), self)
        if color.isValid()
            @colorLabel.text = color.name
            @colorLabel.palette = Qt5::Palette.new(color)
        end
    end
    
    def setFont()
        ok = Qt5::Boolean.new
        font = Qt5::FontDialog.getFont(ok, Qt5::Font.new(@fontLabel.text), self)
        if ok
            @fontLabel.text = font.key()
        end
    end
    
    def setExistingDirectory()
        directory = Qt5::FileDialog.getExistingDirectory(self,
                                    tr("Qt5::FileDialog.getExistingDirectory()"),
                                    @directoryLabel.text,
                                    Qt5::FileDialog::DontResolveSymlinks |
                                    Qt5::FileDialog::ShowDirsOnly)
        if !directory.nil?
            @directoryLabel.text = directory
        end
    end
    
    def setOpenFileName()
        fileName = Qt5::FileDialog.getOpenFileName(self,
                                    tr("Qt5::FileDialog.getOpenFileName()"),
                                    @openFileNameLabel.text,
                                    tr("All Files (*);;Text Files (*.txt)"))
        if !fileName.nil?
            @openFileNameLabel.text = fileName
        end
    end
    
    def setOpenFileNames()
        files = Qt5::FileDialog.getOpenFileNames(
                                    self, tr("Qt5::FileDialog.getOpenFileNames()"),
                                    @openFilesPath,
                                    tr("All Files (*);;Text Files (*.txt)"))
        if files.length != 0
            @openFilesPath = files[0]
            @openFileNamesLabel.text = "[%s]" % files.join(", ")
        end
    end
    
    def setSaveFileName()
        fileName = Qt5::FileDialog.getSaveFileName(self,
                                    tr("Qt5::FileDialog.getSaveFileName()"),
                                    @saveFileNameLabel.text,
                                    tr("All Files (*);;Text Files (*.txt)"))
        if !fileName.nil?
            @saveFileNameLabel.text = fileName
        end
    end
    
    def criticalMessage()
        reply = Qt5::MessageBox::critical(self, tr("Qt5::MessageBox.showCritical()"),
                                          @message,
                                          Qt5::MessageBox::Abort,
                                          Qt5::MessageBox::Retry,
                                          Qt5::MessageBox::Ignore)
        if reply == Qt5::MessageBox::Abort
            @criticalLabel.text = tr("Abort")
        elsif reply == Qt5::MessageBox::Retry
            @criticalLabel.text = tr("Retry")
        else
            @criticalLabel.text = tr("Ignore")
        end
    end
    
    def informationMessage()
        Qt5::MessageBox::information(self, tr("Qt5::MessageBox.showInformation()"), @message)
        @informationLabel.text = tr("Closed with OK or Esc")
    end
    
    def questionMessage()
        reply = Qt5::MessageBox.question(self, tr("Qt5::MessageBox.showQuestion()"),
                                          @message,
                                          Qt5::MessageBox::Yes,
                                          Qt5::MessageBox::No,
                                          Qt5::MessageBox::Cancel)
        if reply == Qt5::MessageBox::Yes
            @questionLabel.text = tr("Yes")
        elsif reply == Qt5::MessageBox::No
            @questionLabel.text = tr("No")
        else
            @questionLabel.text = tr("Cancel")
        end
    end
    
    def warningMessage()
        reply = Qt5::MessageBox.warning(self, tr("Qt5::MessageBox.showWarning()"),
                                         @message,
                                         tr("Save &Again"),
                                         tr("&Continue"))
        if reply == 0
            @warningLabel.text = tr("Save Again")
        else
            @warningLabel.text = tr("Continue")
        end
    end
    
    def errorMessage()
        @errorMessageDialog.showMessage(
                tr("This dialog shows and remembers error messages. " +
                   "If the checkbox is checked (as it is by default), " +
                   "the shown message will be shown again, " +
                   "but if the user unchecks the box the message " +
                   "will not appear again if Qt5::ErrorMessage.showMessage() " +
                   "is called with the same message."))
        @errorLabel.text = tr("If the box is unchecked, the message " +
                               "won't appear again.")
    end
end
