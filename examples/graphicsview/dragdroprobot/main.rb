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
    
require 'Qt5'
require './qrc_robot.rb'
require './coloritem.rb'
require './robot.rb'

app = Qt5::Application.new(ARGV)

scene = Qt5::GraphicsScene.new(-200, -200, 400, 400)

for i in 0...10 do
    item = ColorItem.new
    item.setPos(Math.sin((i * 6.28) / 10.0) * 150,
                    Math.cos((i * 6.28) / 10.0) * 150)

    scene.addItem(item)
end

view = Qt5::GraphicsView.new(scene)

# Pass the view to the Robot and make it the parent of the
# Qt5::GraphicsItemAnimations, to prevent them being GC'd
robot = Robot.new(view)
robot.setTransform(Qt5::Transform.fromScale(1.2, 1.2), true);
robot.setPos(0, -20)
scene.addItem(robot)

view.renderHint = Qt5::Painter::Antialiasing
view.backgroundBrush = Qt5::Brush.new(Qt5::Color.new(230, 200, 167))
view.windowTitle = "Drag and Drop Robot"
view.show

app.exec
