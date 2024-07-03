#
# Copyright (c) 2001 by Jim Menard <jimm@io.com>
#
# Released under the same license as Ruby. See
# http://www.ruby-lang.org/en/LICENSE.txt.
#

require 'Qt5'
require './Canvas'
require './CameraDialog'

class WorldWindow < Qt5::MainWindow

    slots 'slotCameraDialog()'

    attr_accessor :canvas

    def initialize
        super
        setWindowTitle("Boids")
        setupMenubar()

        @canvas = Canvas.new(self, "TheDamnCanvas")
        setCentralWidget(@canvas)
        setGeometry(0, 0, $PARAMS['window_width'],
                    $PARAMS['window_height'])
    end

    def setupMenubar
        # Create and populate file menu
        exitAct = Qt5::Action.new("E&xit", self)
        exitAct.shortcut = Qt5::KeySequence.new("Ctrl+Q")
        connect(exitAct, SIGNAL('triggered()'), $qApp, SLOT('quit()'))

        # Add file menu to menu bar
        fileMenu = menuBar().addMenu("&File")
        fileMenu.addAction(exitAct)

        # Create and populate options menu
        cameraAct = Qt5::Action.new("&Camera...", self)
        connect(cameraAct, SIGNAL('triggered()'), self, SLOT('slotCameraDialog()'))

        # Add options menu to menu bar and link it to method below
        optionsMenu = menuBar().addMenu("&Options")
        optionsMenu.addAction(cameraAct)

    end

    def slotCameraDialog()
        CameraDialog.new(nil).exec()
    end
end
