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
require './qrc_mice.rb'
require './mouse.rb'

MouseCount = 7
app = Qt5::Application.new(ARGV)

scene =  Qt5::GraphicsScene.new
scene.setSceneRect(-300, -300, 600, 600)
scene.itemIndexMethod = Qt5::GraphicsScene::NoIndex

for i in 0...MouseCount do
    mouse = Mouse.new
    mouse.setPos(Math.sin(i * 6.28 / MouseCount) * 200,
                    Math.cos((i * 6.28) / MouseCount) * 200)
    scene.addItem(mouse)
end

view = Qt5::GraphicsView.new(scene)
view.renderHint = Qt5::Painter::Antialiasing
view.backgroundBrush = Qt5::Brush.new(Qt5::Pixmap.new(":/images/cheese.jpg"))
view.cacheMode = Qt5::GraphicsView::CacheBackground
view.dragMode = Qt5::GraphicsView::ScrollHandDrag
view.setWindowTitle(QT_TRANSLATE_NOOP(Qt5::GraphicsView, "Colliding Mice"))
view.resize(400, 300)
view.show

app.exec
