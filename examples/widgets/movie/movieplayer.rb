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
    
    
class MoviePlayer < Qt5::Widget
    
    slots    'open()',
            'goToFrame(int)',
            'fitToWindow()',
            'updateButtons()',
            'updateFrameSlider()'
    
    
    def initialize(parent = nil)
        super(parent)
        @movie = Qt5::Movie.new(self)
        @movie.cacheMode = Qt5::Movie::CacheAll
    
        @movieLabel = Qt5::Label.new(tr("No movie loaded")) do |m|
            m.alignment = Qt5::AlignCenter
            m.setSizePolicy(Qt5::SizePolicy::Ignored, Qt5::SizePolicy::Ignored)
            m.backgroundRole = Qt5::Palette::Dark
            m.autoFillBackground = true
        end
    
        @currentMovieDirectory = "@movies"
    
        createControls()
        createButtons()
    
        connect(@movie, SIGNAL('frameChanged(int)'), self, SLOT('updateFrameSlider()'))
        connect(@movie, SIGNAL('stateChanged(QMovie::MovieState)'),
                self, SLOT('updateButtons()'))
        connect(@fitCheckBox, SIGNAL('clicked()'), self, SLOT('fitToWindow()'))
        connect(@frameSlider, SIGNAL('valueChanged(int)'), self, SLOT('goToFrame(int)'))
        connect(@speedSpinBox, SIGNAL('valueChanged(int)'),
                @movie, SLOT('setSpeed(int)'))
    
        self.layout = Qt5::VBoxLayout.new do |m|
            m.addWidget(@movieLabel)
            m.addLayout(@controlsLayout)
            m.addLayout(@buttonsLayout)
        end
    
        updateFrameSlider()
        updateButtons()
    
        setWindowTitle(tr("Movie Player"))
        resize(400, 400)
    end
    
    def open()
        fileName = Qt5::FileDialog.getOpenFileName(self, tr("Open a Movie"),
                                   @currentMovieDirectory)
        if !fileName.empty?
            @currentMovieDirectory = Qt5::FileInfo.new(fileName).path()
    
            @movie.stop()
            @movieLabel.movie = @movie
            @movie.fileName = fileName
            @movie.start()
    
            updateFrameSlider()
            updateButtons()
        end
    end
    
    def goToFrame(frame)
        @movie.jumpToFrame(frame)
    end
    
    def fitToWindow()
        @movieLabel.scaledContents = @fitCheckBox.checked?
    end
    
    def updateFrameSlider()
        hasFrames = (@movie.currentFrameNumber() >= 0)
    
        if hasFrames
            if @movie.frameCount > 0
                @frameSlider.maximum = @movie.frameCount - 1
            else
                if @movie.currentFrameNumber > @frameSlider.maximum
                    @frameSlider.maximum = @movie.currentFrameNumber
                end
            end
            @frameSlider.value = @movie.currentFrameNumber
        else
            @frameSlider.maximum = 0
        end
        @frameLabel.enabled = hasFrames
        @frameSlider.enabled = hasFrames
    end
    
    def updateButtons()
        @playButton.enabled = @movie.valid? && @movie.frameCount != 1 &&
                               @movie.state == Qt5::Movie::NotRunning
        @pauseButton.enabled = @movie.state != Qt5::Movie::NotRunning
        @pauseButton.checked = @movie.state == Qt5::Movie::Paused
        @stopButton.enabled = @movie.state != Qt5::Movie::NotRunning
    end
    
    def createControls()
        @fitCheckBox = Qt5::CheckBox.new(tr("Fit to Window"))
    
        @frameLabel = Qt5::Label.new(tr("Current frame:"))
    
        @frameSlider = Qt5::Slider.new(Qt5::Horizontal) do |f|
            f.tickPosition = Qt5::Slider::TicksBelow
            f.tickInterval = 10
        end
    
        @speedLabel = Qt5::Label.new(tr("Speed:"))
    
        @speedSpinBox = Qt5::SpinBox.new do |s|
            s.range = 1..9999
            s.value = 100
            s.suffix = tr("%")
        end
    
        @controlsLayout = Qt5::GridLayout.new do |c|
            c.addWidget(@fitCheckBox, 0, 0, 1, 2)
            c.addWidget(@frameLabel, 1, 0)
            c.addWidget(@frameSlider, 1, 1, 1, 2)
            c.addWidget(@speedLabel, 2, 0)
            c.addWidget(@speedSpinBox, 2, 1)
        end
    end
    
    def createButtons()
        iconSize = Qt5::Size.new(36, 36)
    
        @openButton = Qt5::ToolButton.new do |o|
            o.icon = Qt5::Icon.new("images/open.png")
            o.iconSize = iconSize
            o.toolTip = tr("Open File")
        end
        connect(@openButton, SIGNAL('clicked()'), self, SLOT('open()'))
    
        @playButton = Qt5::ToolButton.new do |p|
            p.icon = Qt5::Icon.new("images/play.png")
            p.iconSize = iconSize
            p.toolTip = tr("Play")
        end
        connect(@playButton, SIGNAL('clicked()'), @movie, SLOT('start()'))
    
        @pauseButton = Qt5::ToolButton.new do |p|
            p.checkable = true
            p.icon = Qt5::Icon.new("images/pause.png")
            p.iconSize = iconSize
            p.toolTip = tr("Pause")
        end
        connect(@pauseButton, SIGNAL('clicked(bool)'), @movie, SLOT('setPaused(bool)'))
    
        @stopButton = Qt5::ToolButton.new do |s|
            s.icon = Qt5::Icon.new("images/stop.png")
            s.iconSize = iconSize
            s.toolTip = tr("Stop")
        end
        connect(@stopButton, SIGNAL('clicked()'), @movie, SLOT('stop()'))
    
        @quitButton = Qt5::ToolButton.new do |q|
            q.icon = Qt5::Icon.new("images/quit.png")
            q.iconSize = iconSize
            q.toolTip = tr("Quit")
        end
        connect(@quitButton, SIGNAL('clicked()'), self, SLOT('close()'))
    
        @buttonsLayout = Qt5::HBoxLayout.new do |b|
            b.addStretch()
            b.addWidget(@openButton)
            b.addWidget(@playButton)
            b.addWidget(@pauseButton)
            b.addWidget(@stopButton)
            b.addWidget(@quitButton)
            b.addStretch()
        end
    end
end
