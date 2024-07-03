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
	
	
class Sender < Qt5::Dialog
		
	slots :startBroadcasting, :broadcastDatagram
	
	def initialize(parent = nil)
	    super(parent)
	    @statusLabel = Qt5::Label.new(tr("Ready to broadcast datagrams on port 45454"))
	    @startButton = Qt5::PushButton.new(tr("&Start"))
	    @quitButton = Qt5::PushButton.new(tr("&Quit"))
	    @timer = Qt5::Timer.new(self)
	    @udpSocket = Qt5::UdpSocket.new(self)
	    @messageNo = 1
	
	    connect(@startButton, SIGNAL(:clicked), self, SLOT(:startBroadcasting))
	    connect(@quitButton, SIGNAL(:clicked), self, SLOT(:close))
	    connect(@timer, SIGNAL(:timeout), self, SLOT(:broadcastDatagram))
	
	    buttonLayout = Qt5::HBoxLayout.new do |b|
			b.addStretch(1)
			b.addWidget(@startButton)
			b.addWidget(@quitButton)
		end
	
	    self.layout = Qt5::VBoxLayout.new do |m|
			m.addWidget(@statusLabel)
			m.addLayout(buttonLayout)
		end
	
	    self.windowTitle = tr("Broadcast Sender")
	end
	
	def startBroadcasting()
	    @startButton.enabled = false
	    @timer.start(1000)
	end
	
	def broadcastDatagram()
	    @statusLabel.text = tr("Now broadcasting datagram %d" % @messageNo)
            datagram = Qt5::NetworkDatagram.new(Qt5::ByteArray.new("Broadcast message %d" % @messageNo), Qt5::HostAddress.new(Qt5::HostAddress::Broadcast), 45454)
	    @udpSocket.writeDatagram(datagram)
	    @messageNo += 1
	end
end
