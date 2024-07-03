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
    
require 'Qt5'
require './ping-common.rb'
   
class Ping < Qt5::Object
    slots 'start(const QString&, const QString&, const QString&)'

    def start(name, oldValue, newValue)
        if name != SERVICE_NAME || newValue.empty?
            return
        end

        # find our remote
        iface = Qt5::DBusInterface.new(SERVICE_NAME, "/", "com.trolltech.QtDBus.ComplexPong.Pong",
                                   Qt5::DBusConnection.sessionBus, self)
        if !iface.valid?
            $stderr.puts("%s" % Qt5::DBusConnection.sessionBus.lastError.message)
            Qt5::CoreApplication.instance.quit
        end
    
        connect(iface, SIGNAL(:aboutToQuit), Qt5::CoreApplication.instance(), SLOT(:quit))
    
        while true
            print("Ask your question: ")
    
            line = gets.strip
            if line.empty?
                iface.call("quit")
                return
            elsif line == "value"
                reply = iface.value
                if !reply.nil?
                    puts("value = %s" % reply)
                end
            elsif line =~ /^value=/
                iface.setValue Qt5::Variant.new(line[6, line.length])
            else
                reply = Qt5::DBusReply.new(iface.call("query", Qt5::Variant.new(line)))
                if reply.valid?
                    puts("Reply was: %s" % reply.value.value)
                end
            end
    
            if iface.lastError.valid?
                $stderr.puts("Call failed: %s" % iface.lastError.message)
            end
        end
    end    
end

app = Qt5::CoreApplication.new(ARGV)
    
if !Qt5::DBusConnection.sessionBus.connected?
    $stderr.puts("Cannot connect to the D-BUS session bus.\n" \
                    "To start it, run:\n" \
                    "\teval `dbus-launch --auto-syntax`\n")
    exit(1)
end
    
ping = Ping.new
ping.connect(Qt5::DBusConnection.sessionBus.interface,
             SIGNAL('serviceOwnerChanged(QString,QString,QString)'),
             SLOT('start(QString,QString,QString)'))

pong = Qt5::Process.new
pong.start("ruby ./complexpong.rb")

app.exec
