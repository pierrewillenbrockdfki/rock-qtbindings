#!/usr/bin/env ruby

require 'Qt5'
 
class PassiveWindow < Qt5::Frame
	MARGIN = 20
	
    def initialize(message)
        super(nil, Qt5::X11BypassWindowManagerHint | Qt5::WindowStaysOnTopHint |
                   Qt5::Tool | Qt5::FramelessWindowHint)
        setFrameStyle(Qt5::Frame::Box| Qt5::Frame::Plain)
        setLineWidth(2)

        setMinimumWidth(100)
        layout = Qt5::VBoxLayout.new(self) do |l|
            l.spacing = 11
            l.margin = 6
        end
        Qt5::Label.new(message, self)

        quit=Qt5::PushButton.new(tr("Close"), self)
        connect(quit, SIGNAL("clicked()"), SLOT("close()"))
	end

    def show
        super
        move(Qt5::Application.desktop().width() - width() - MARGIN,
            Qt5::Application.desktop().height() - height() - MARGIN)
	end
end
  	
if (Process.fork != nil)
	exit
end
app = Qt5::Application.new(ARGV)
win = PassiveWindow.new(ARGV[0])
win.show
app.exec
