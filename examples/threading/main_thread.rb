#!/usr/bin/ruby -W
require "Qt5"

app = Qt5::Application.new(ARGV)

hello = Qt5::PushButton.new('Hello World!')
hello.resize(100, 30)
hello.show()

# This code is not thread-safe because it is trying to access the GUI
# (QT code) outside of the main thread
#Thread.new { sleep 2; hello.resize(200,50) }

# This code executes because it puts the GUI code inside
# Qt5.execute_in_main_thread
Thread.new { sleep 2; Qt5.execute_in_main_thread { hello.resize(200,50) } }

app.exec()

