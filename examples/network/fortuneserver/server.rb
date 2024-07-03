=begin
**
** Copyright (C) 2004-2006 Trolltech AS. All rights reserved.
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
    
class Server < Qt5::Dialog
    
    slots :sendFortune
    
    def initialize(parent = nil)
        super(parent)
        @statusLabel = Qt5::Label.new
        @quitButton = Qt5::PushButton.new(tr("Quit"))
        @quitButton.autoDefault = false
    
        @tcpServer = Qt5::TcpServer.new(self)
        if !@tcpServer.listen
            Qt5::MessageBox.critical(self, tr("Fortune Server"),
                tr("Unable to start the server: %s." % @tcpServer.errorString))
            close()
            return
        end
    
        @statusLabel.text = tr("The server is running on port %d.\nRun the Fortune Client example now." %
                             @tcpServer.serverPort)
    
        @fortunes = []
        @fortunes << tr("You've been leading a dog's life. Stay off the furniture.") <<
                 tr("You've got to think about tomorrow.") <<
                 tr("You will be surprised by a loud noise.") <<
                 tr("You will feel hungry again in another hour.") <<
                 tr("You might have mail.") <<
                 tr("You cannot kill time without injuring eternity.") <<
                 tr("Computers are not intelligent. They only think they are.")
    
        connect(@quitButton, SIGNAL(:clicked), self, SLOT(:close))
        connect(@tcpServer, SIGNAL(:newConnection), self, SLOT(:sendFortune))
    
        buttonLayout = Qt5::HBoxLayout.new do |b|
            b.addStretch(1)
            b.addWidget(@quitButton)
        end

        self.layout = Qt5::VBoxLayout.new do |m|
            m.addWidget(@statusLabel)
            m.addLayout(buttonLayout)
        end
    
        self.windowTitle = tr("Fortune Server")
    end
    
    def sendFortune
        block = Qt5::ByteArray.new
        outf = Qt5::DataStream.new(block, Qt5::IODevice::WriteOnly)
        outf.version = Qt5::DataStream::Qt_4_0
        outf << 0  # Write a 4 byte integer
        outf << @fortunes[rand(@fortunes.length)]
        outf.device.seek(0)
        outf << (block.length - 4)  # 4 bytes is the size of an integer
    
        clientConnection = @tcpServer.nextPendingConnection()
        connect(clientConnection, SIGNAL(:disconnected),
                clientConnection, SLOT(:deleteLater))
    
        clientConnection.write(block)
        clientConnection.disconnectFromHost
    end
end
